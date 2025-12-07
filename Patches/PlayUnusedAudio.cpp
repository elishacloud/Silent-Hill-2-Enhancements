// playunusedaudio.cpp
#include <windows.h>
#include <dsound.h>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include "Common/Settings.h"
#include "Patches.h"
#include "InputTweaks.h"
#include <shlwapi.h>

template <typename T>
struct ComReleaser
{
    T* ptr;

    ComReleaser(T* p = nullptr) : ptr(p) {}

    ~ComReleaser()
    {
        if (ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }

    // Conversion to raw pointer
    operator T* () const { return ptr; }

    T** operator&() { return &ptr; }

    T* operator->() const { return ptr; }
};

// RAII for critical section
struct AutoLock
{
    CRITICAL_SECTION* cs;
    AutoLock(CRITICAL_SECTION* c) : cs(c) { EnterCriticalSection(cs); }
    ~AutoLock() { LeaveCriticalSection(cs); }
};

void PatchUnusedAudio();
extern "C" HRESULT WINAPI DSOAL_DirectSoundCreate8(LPCGUID lpcGUID, IDirectSound8** ppDS, IUnknown* pUnkOuter);

#pragma comment(lib, "dsound.lib")

static DWORD* textscreen = nullptr;
static DWORD lastTextValue = 0;

struct AfsEntry { uint32_t offset; uint32_t size; };

static std::string g_dllDir;
static std::string g_logPath;
static std::string voicepath;
static bool g_enableLog = false;
extern "C" IMAGE_DOS_HEADER __ImageBase;

static IDirectSound8* g_pDS = nullptr;
static IDirectSoundBuffer* g_pBuffer = nullptr;
static CRITICAL_SECTION g_audioLock;
static bool g_audioLockInitialized = false;

static std::vector<AfsEntry> g_afsTable;

static volatile LONG gStarted = 0;

static const int SEQ_COUNT = 12;
static const int MAX_SEQ_IDX = 11;

static void WriteLog(const char* fmt, ...)
{
    // Only log if PlayUnusedAudio.log exists
    if (!g_enableLog) return;
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    std::ofstream f(g_logPath, std::ios::app);
    if (f.is_open()) { f << buf << "\n"; f.close(); }
}

static void StopAlertSound()
{
    // caller must own g_audioLock
    if (g_pBuffer)
    {
        g_pBuffer->Stop();
        g_pBuffer->Release();
        g_pBuffer = nullptr;
        WriteLog("StopCurrentPlayback: buffer stopped and released.");
    }
}

// Initialize global DirectSound device only once.
static bool EnsureDirectSoundInitialized()
{
    if (!g_audioLockInitialized)
    {
        InitializeCriticalSection(&g_audioLock);
        g_audioLockInitialized = true;
    }

    if (g_pDS) return true;

    IDirectSound8* pDS = nullptr;
    HRESULT hr = DSOAL_DirectSoundCreate8(nullptr, &pDS, nullptr);
    if (FAILED(hr) || !pDS)
    {
        WriteLog("EnsureDirectSoundInitialized: DSOAL_DirectSoundCreate8 failed hr=0x%08X", (unsigned)hr);
        return false;
    }

    HWND hwnd = GetForegroundWindow();
    if (!hwnd) hwnd = GetDesktopWindow();
    pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);

    // keep global
    g_pDS = pDS;
    WriteLog("EnsureDirectSoundInitialized: DirectSound device created.");
    return true;
}

static bool LoadFileToMemory(const std::string& path, std::vector<BYTE>& outBuf)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open())
        return false;

    std::streamsize size = f.tellg();
    if (size <= 0)
        return false;

    outBuf.resize(size);

    f.seekg(0, std::ios::beg);
    if (!f.read((char*)outBuf.data(), size))
    {
        outBuf.clear();
        return false;
    }

    return true;
}

// Applies master volume by multiplying every PCM sample.
static void ApplyGameMasterVolume(BYTE* buffer, DWORD sizeBytes, float volume)
{
    short* samples = (short*)buffer;
    DWORD sampleCount = sizeBytes / sizeof(short);

    for (DWORD i = 0; i < sampleCount; i++)
    {
        int v = (int)(samples[i] * volume);
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        samples[i] = (short)v;
    }
}

// Create a DirectSound buffer and play the WAV PCM data.
static bool PlayWavWithDirectSound(const WAVEFORMATEX* wf, const BYTE* audioData, DWORD audioSize)
{
    if (!wf || !audioData || audioSize == 0)
        return false;

    if (!EnsureDirectSoundInitialized())
        return false;

    AutoLock lock(&g_audioLock);

    // Stop and destroy the previous buffer
    if (g_pBuffer)
    {
        g_pBuffer->Stop();
        g_pBuffer->Release();
        g_pBuffer = nullptr;
    }

    DSBUFFERDESC desc = {};
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;
    desc.dwBufferBytes = audioSize;
    desc.lpwfxFormat = (LPWAVEFORMATEX)wf;

    // RAII for new buffer
    ComReleaser<IDirectSoundBuffer> localBuffer;

    // Create sound buffer
    HRESULT hr = g_pDS->CreateSoundBuffer(&desc, &localBuffer, nullptr);
    if (FAILED(hr))
    {
        WriteLog("PlayWavWithDirectSound: CreateSoundBuffer failed (hr=0x%08X)", (unsigned)hr);
        return false;
    }

    void* p1; DWORD b1;
    void* p2; DWORD b2;

    hr = localBuffer->Lock(0, audioSize, &p1, &b1, &p2, &b2, 0);
    if (FAILED(hr))
    {
        WriteLog("PlayWavWithDirectSound: Lock failed (hr=0x%08X)", (unsigned)hr);
        return false;
    }

    if (p1 && b1) memcpy(p1, audioData, b1);
    if (p2 && b2) memcpy(p2, audioData + b1, b2);

    localBuffer->Unlock(p1, b1, p2, b2);

    // Start playback
    hr = localBuffer->Play(0, 0, 0);
    if (FAILED(hr))
    {
        WriteLog("PlayWavWithDirectSound: Play failed (hr=0x%08X)", (unsigned)hr);
        return false;
    }

    // Transfer ownership
    g_pBuffer = localBuffer.ptr;
    localBuffer.ptr = nullptr;

    WriteLog("PlayWavWithDirectSound: playback OK (%u bytes).", audioSize);
    return true;
}

// Releases DirectSound device and audio buffer safely.
static void ReleaseDirectSoundDevice()
{
    EnterCriticalSection(&g_audioLock);
    if (g_pBuffer) { g_pBuffer->Stop(); g_pBuffer->Release(); g_pBuffer = nullptr; }
    if (g_pDS) { g_pDS->Release(); g_pDS = nullptr; }
    LeaveCriticalSection(&g_audioLock);

    if (g_audioLockInitialized)
    {
        DeleteCriticalSection(&g_audioLock);
        g_audioLockInitialized = false;
    }
}

// Plays only a slice of a WAV file (start/end seconds).
static void PlayWavFromMemoryRange(BYTE* wavBuf, size_t wavSize,
    float startSec, float endSec)
{
    if (!wavBuf || wavSize < 44) {
        WriteLog("PlayWavFromMemoryRange: invalid buffer/size");
        return;
    }

    uint32_t fmtSize = *(uint32_t*)(wavBuf + 16);
    WAVEFORMATEX* wf = (WAVEFORMATEX*)(wavBuf + 20);

    if (fmtSize < 16 || !wf->nSamplesPerSec || !wf->wBitsPerSample) {
        WriteLog("PlayWavFromMemoryRange: invalid WAV header");
        return;
    }

    DWORD dataPos = 20 + fmtSize + 8;

    if (dataPos >= wavSize) {
        WriteLog("PlayWavFromMemoryRange: dataPos >= wavSize");
        return;
    }

    BYTE* audio = wavBuf + dataPos;
    DWORD audioSize = (DWORD)(wavSize - dataPos);
    DWORD frameBytes = (wf->wBitsPerSample / 8) * wf->nChannels;

    DWORD start = (DWORD)(startSec * wf->nSamplesPerSec * frameBytes);
    DWORD end = (DWORD)(endSec * wf->nSamplesPerSec * frameBytes);

    if (start > audioSize) start = 0;
    if (end > audioSize) end = audioSize;
    if (end <= start) {
        WriteLog("PlayWavFromMemoryRange: invalid PCM range");
        return;
    }

    DWORD playLen = end - start;

    std::vector<BYTE> pcm(playLen);
    memcpy(pcm.data(), audio + start, playLen);

    float vol = EnableMasterVolume ?
        (float)ConfigData.VolumeLevel / 15.0f : 1.0f;

    if (wf->wFormatTag == WAVE_FORMAT_PCM && wf->wBitsPerSample == 16)
        ApplyGameMasterVolume(pcm.data(), playLen, vol);

    if (!PlayWavWithDirectSound(wf, pcm.data(), playLen)) {
        WriteLog("ERROR: DirectSound failed to play PCM");
        return;
    }

    WriteLog("PlayWavFromMemoryRange: playback OK.");
}

static void InitVoiceAFS()
{
    std::vector<BYTE> buf;
    if (!LoadFileToMemory(voicepath, buf)) {
        WriteLog("InitVoiceAFS: failed to open AFS file.");
        return;
    }

    if (buf.size() < 16 || memcmp(buf.data(), "AFS\0", 4) != 0) {
        WriteLog("InitVoiceAFS: invalid AFS file.");
        return;
    }

    uint32_t fileCount = *(uint32_t*)(buf.data() + 4);
    WriteLog("InitVoiceAFS: fileCount=%u", fileCount);

    size_t tableStart = 8;
    size_t tableSize = fileCount * 8;

    if (tableStart + tableSize > buf.size()) {
        WriteLog("InitVoiceAFS: offset/size table exceeds file size.");
        return;
    }

    g_afsTable.resize(fileCount);

    for (uint32_t i = 0; i < fileCount; ++i)
    {
        size_t p = tableStart + i * 8;
        g_afsTable[i].offset = *(uint32_t*)(buf.data() + p);
        g_afsTable[i].size = *(uint32_t*)(buf.data() + p + 4);
    }

    WriteLog("InitVoiceAFS: offset/size table loaded.");
}

static bool PlayVoiceAfs(uint32_t index, float startSec, float endSec)
{
    if (index >= g_afsTable.size()) {
        WriteLog("PlayVoiceAfs: invalid index %u", index);
        return false;
    }

    const AfsEntry& e = g_afsTable[index];

    WriteLog("PlayVoiceAfs: index=%u offset=0x%X size=%u",
        index, e.offset, e.size);

    std::ifstream f(voicepath, std::ios::binary);
    if (!f.is_open()) {
        WriteLog("PlayVoiceAfs: cannot open AFS.");
        return false;
    }

    f.seekg(e.offset, std::ios::beg);

    std::vector<BYTE> wavData(e.size);
    if (!f.read((char*)wavData.data(), e.size)) {
        WriteLog("PlayVoiceAfs: reading block failed.");
        return false;
    }

    PlayWavFromMemoryRange(wavData.data(), wavData.size(), startSec, endSec);
    return true;
}

static void ResetState(int& sequenceIndex, bool& active, bool& initializedAfterStart)
{
    StopAlertSound();
    sequenceIndex = 0;
    lastTextValue = 0;
    active = false;
    initializedAfterStart = false;
}

// Load sequence timestamps for this specific cutscene.
static void LoadSequenceTables(float* outStart, float* outEnd, int& outMax)
{
    static const float start[SEQ_COUNT] = { 0.0f,7.2f,13.0f,17.1f,19.2f,22.5f,35.2f,47.2f,53.4f,61.2f,64.0f, 0.0f };
    static const float end[SEQ_COUNT] = { 7.0f,12.4f,16.9f,19.0f,22.0f,34.5f,46.2f,52.2f,60.2f,63.5f,67.2f, 0.0f };
    memcpy(outStart, start, sizeof(float) * SEQ_COUNT);
    memcpy(outEnd, end, sizeof(float) * SEQ_COUNT);
    outMax = MAX_SEQ_IDX;
}

// Detects the exact moment when the audio system should activate.
static bool ShouldActivateSequence(DWORD room, DWORD cutscene, DWORD fade, float cutsceneTime)
{
    return (room == R_HTL_RESTAURANT && cutscene == CS_HTL_LAURA_PIANO && fade == 3 && cutsceneTime >= 1600.000000);
}

// Prevent false positives by checking cutscene ID twice with a delay.
static bool StabilizeCutscene() {
    bool first = (GetCutsceneID() == CS_HTL_LAURA_PIANO && GetCutsceneTimer() >= 1600.0f);
    if (first) return true;

    WriteLog("Cutscene changed — performing additional check...");
    Sleep(30);

    bool second = (GetCutsceneID() == CS_HTL_LAURA_PIANO && GetCutsceneTimer() >= 1600.0f);

    if (second)
        WriteLog("FALSE POSITIVE detected: first check failed, second succeeded.");

    return second;
}

static DWORD ReadTextscreenSafe()
{
    if (!textscreen) return 0;
    return *textscreen;
}

DWORD WINAPI AudioMonitorThread(LPVOID)
{
    char selfName[MAX_PATH];
    GetModuleFileNameA((HMODULE)&__ImageBase, selfName, MAX_PATH);
    strcpy_s(selfName, MAX_PATH, PathFindFileNameA(selfName));
    WriteLog("Monitor running inside DLL: %s", selfName);
    Sleep(3000);
    WriteLog("AudioMonitorThread started.");

    int sequenceIndex = 0;
    float seqStart[SEQ_COUNT]; float seqEnd[SEQ_COUNT]; int maxSeq;
    bool active = false;
    bool initializedAfterStart = false;

    while (true)
    {
        if (!GetModuleHandleA(selfName))
        {
            ReleaseDirectSoundDevice();
            WriteLog("DLL unload detected. Terminating monitor thread.");
            return 0;
        }

        DWORD room = GetRoomID();
        DWORD cutscene = GetCutsceneID();
        DWORD fade = GetTransitionState();
        float cutsceneTime = GetCutsceneTimer();

        // If leaving the restaurant, reset everything.
        if (room != R_HTL_RESTAURANT)
        {
            ResetState(sequenceIndex, active, initializedAfterStart);
            WriteLog("Room: %u (not restaurant). Reset state and sleeping.", room);
            continue;
        }

        LoadSequenceTables(seqStart, seqEnd, maxSeq);

        // Activation phase — waiting for the exact trigger.
        if (!active)
        {
            if (ShouldActivateSequence(room, cutscene, fade, cutsceneTime))
            {
                WriteLog("Activation: room, cutscene, fade, and CutsceneTime OK. Enabling sequence.");
                active = true; sequenceIndex = 0; initializedAfterStart = false; lastTextValue = 0;
                Sleep(20);
                continue;
            }
            else
            {
                Sleep(20);
                continue;
            }
        }

        // Cutscene might have changed — double-check.
        if (!StabilizeCutscene()) {
            WriteLog("Cutscene unstable. Resetting.");
            ResetState(sequenceIndex, active, initializedAfterStart);
            continue;
        }

        DWORD value = ReadTextscreenSafe();

        // First value after activation becomes baseline.
        if (!initializedAfterStart)
        {
            WriteLog("[INIT] First read after activation: lastTextValue=%u", value);
            lastTextValue = value;
            initializedAfterStart = true;
            Sleep(20);
            continue;
        }

        // Detect change: play the next audio slice.
        if (value != 0 && value != lastTextValue)
        {
            WriteLog("[PLAY] Change detected: last=%u : value=%u (seq=%d)", lastTextValue, value, sequenceIndex);
            if (sequenceIndex <= maxSeq)
                PlayVoiceAfs(119, seqStart[sequenceIndex], seqEnd[sequenceIndex]);
            lastTextValue = value;
            sequenceIndex++;
        }

        // All sequences finished
        if (sequenceIndex > maxSeq)
        {
            WriteLog("[END] Final sequence reached. Stopping audio and waiting for room/cutscene change...");
            StopAlertSound();

            // Wait until cutscene actually changes
            while (StabilizeCutscene())
            {
                if (!GetModuleHandleA(selfName))
                {
                    ReleaseDirectSoundDevice();
                    WriteLog("DLL unload detected. Terminating monitor thread.");
                    return 0;
                }
                Sleep(200);
            }

            ResetState(sequenceIndex, active, initializedAfterStart);
            WriteLog("[RESET] Values reset. Waiting for next activation (fade==3).");
            continue;
        }

        Sleep(20);
    }
    return 0;
}

// Entry point: locate memory address, verify AFS, start monitor thread.
void PatchUnusedAudio()
{
    BYTE pattern[] = { 0xA3, 0xEC, 0x03, 0xF6, 0x01 };
    BYTE* addr = (BYTE*)SearchAndGetAddresses(
        0x0048042F, 0x004806AF, 0x0047FE3F,
        pattern, sizeof(pattern), 0, __FUNCTION__);

    if (!addr) WriteLog("ERROR: Could not locate sh2pc.exe+1B603EC through pattern scan!");
    else
    {
        textscreen = (DWORD*)(*(DWORD*)(addr + 1));
        WriteLog("Found target address = %p", textscreen);
    }

    if (InterlockedExchange(&gStarted, 1) != 0) return;

    char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string s = path; size_t p = s.find_last_of("\\/"); if (p != std::string::npos) s = s.substr(0, p);
    g_dllDir = s; g_logPath = g_dllDir + "\\PlayUnusedAudio.log";

    { std::ifstream lf(g_logPath); g_enableLog = lf.is_open(); }
    WriteLog("Starting PlayUnusedAudio...");
    voicepath = g_dllDir + "\\sh2e\\sound\\adx\\voice\\voice.afs";
    {
        std::ifstream test(voicepath, std::ios::binary);
        if (!test.is_open()) { WriteLog("voice.afs NOT found: %s", voicepath.c_str()); WriteLog("Monitor will not start. Restart the game and try again."); return; }
    }

    WriteLog("AFS file found. Continuing...");
    InitVoiceAFS();
    CreateThread(NULL, 0, AudioMonitorThread, NULL, 0, NULL);
}