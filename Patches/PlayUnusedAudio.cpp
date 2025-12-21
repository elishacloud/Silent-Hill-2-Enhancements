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

struct AutoLock 
{
    CRITICAL_SECTION* cs;
    AutoLock(CRITICAL_SECTION* c) : cs(c) { EnterCriticalSection(cs); }
    ~AutoLock() { LeaveCriticalSection(cs); }
};

void PatchUnusedAudio();
extern "C" HRESULT WINAPI DSOAL_DirectSoundCreate8(LPCGUID lpcGUID, IDirectSound8** ppDS, IUnknown* pUnkOuter);

volatile LONG g_forceAction = 0;


struct AfsEntry { uint32_t offset; uint32_t size; };

extern KeyBindsHandler KeyBinds;
extern InputTweaks InputTweaksRef;

static std::string g_dllDir;
static std::string voicepath;
extern "C" IMAGE_DOS_HEADER __ImageBase;

static IDirectSound8* g_pDS = nullptr;
static IDirectSoundBuffer* g_pBuffer = nullptr;
static CRITICAL_SECTION g_audioLock;
static bool g_audioLockInitialized = false;

static std::vector<AfsEntry> g_afsTable;
static volatile LONG gStarted = 0;



volatile LONG g_blockPlayerInputs = 0; // set to 1 to block inputs for autoplay
volatile LONG g_stopAutoplay = 0;     // set to 1 to request immediate stop autoplay

static void StopAlertSound()
{
    // caller must own g_audioLock
    if (g_pBuffer)
    {
        g_pBuffer->Stop();
        g_pBuffer->Release();
        g_pBuffer = nullptr;
        Logging::LogDebug() << __FUNCTION__ << "buffer stopped and released.";
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
        Logging::LogDebug() << __FUNCTION__ << "DSOAL_DirectSoundCreate8 failed hr=0x%08X";
        return false;
    }

    HWND hwnd = GetForegroundWindow();
    if (!hwnd) hwnd = GetDesktopWindow();
    pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);

    // keep global
    g_pDS = pDS;
    Logging::LogDebug() << __FUNCTION__ << "DirectSound device created.";
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
    outBuf.resize((size_t)size);
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
        Logging::LogDebug() << __FUNCTION__ << "CreateSoundBuffer failed (hr=0x%08X)", (unsigned)hr;
        return false;
    }

    void* p1 = nullptr; DWORD b1 = 0;
    void* p2 = nullptr; DWORD b2 = 0;
    hr = localBuffer->Lock(0, audioSize, &p1, &b1, &p2, &b2, 0);
    if (FAILED(hr)) 
    {
        Logging::LogDebug() << __FUNCTION__ << "Lock failed (hr=0x%08X)", (unsigned)hr;
        return false;  
    }

    if (p1 && b1) memcpy(p1, audioData, b1);
    if (p2 && b2) memcpy(p2, audioData + b1, b2);
    localBuffer->Unlock(p1, b1, p2, b2);

    hr = localBuffer->Play(0, 0, 0);
    if (FAILED(hr))
    {
        Logging::LogDebug() << __FUNCTION__ << "Lock failed (hr=0x%08X)", (unsigned)hr;
        return false;
    }

    // Transfer ownership
    g_pBuffer = localBuffer.ptr;
    localBuffer.ptr = nullptr;
    Logging::LogDebug() << __FUNCTION__ << "playback OK (%u bytes).", audioSize;
    return true;
}



// Plays only a slice of a WAV file (start/end seconds).
static void PlayWavFromMemoryRange(BYTE* wavBuf, size_t wavSize, float startSec, float endSec)
{
    if (!wavBuf || wavSize < 44) 
    {
        Logging::LogDebug() << __FUNCTION__ << "invalid buffer/size";
        return; 
    }

    uint32_t fmtSize = *(uint32_t*)(wavBuf + 16);
    WAVEFORMATEX* wf = (WAVEFORMATEX*)(wavBuf + 20);

    if (fmtSize < 16 || !wf->nSamplesPerSec || !wf->wBitsPerSample) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "invalid WAV header.";
        return; 
    }

    DWORD dataPos = 20 + fmtSize + 8;
    if (dataPos >= wavSize) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "dataPos >= wavSize";
        return; 
    }

    BYTE* audio = wavBuf + dataPos;
    DWORD audioSize = (DWORD)(wavSize - dataPos);
    DWORD frameBytes = (wf->wBitsPerSample / 8) * wf->nChannels;

    DWORD start = (DWORD)(startSec * wf->nSamplesPerSec * frameBytes);
    DWORD end = (DWORD)(endSec * wf->nSamplesPerSec * frameBytes);

    if (start > audioSize) start = 0;
    if (end > audioSize) end = audioSize;
    if (end <= start) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "invalid PCM range.";
    return;
    }

    DWORD playLen = end - start;
    std::vector<BYTE> pcm(playLen);
    memcpy(pcm.data(), audio + start, playLen);

    float vol = EnableMasterVolume ? (float)ConfigData.VolumeLevel / 15.0f : 1.0f;
    if (wf->wFormatTag == WAVE_FORMAT_PCM && wf->wBitsPerSample == 16)
        ApplyGameMasterVolume(pcm.data(), playLen, vol);

    if (!PlayWavWithDirectSound(wf, pcm.data(), playLen)) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "ERROR: DirectSound failed to play PCM.";
        return; 
    }
    Logging::LogDebug() << __FUNCTION__ << "playback OK.";
}

static void InitVoiceAFS()
{
    std::vector<BYTE> buf;
    if (!LoadFileToMemory(voicepath, buf)) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "failed to open AFS file.";
        return; 
    }
    if (buf.size() < 16 || memcmp(buf.data(), "AFS\0", 4) != 0) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "invalid AFS file.";
        return; 
    }

    uint32_t fileCount = *(uint32_t*)(buf.data() + 4);
    Logging::LogDebug() << __FUNCTION__ << "fileCount=%u", fileCount;


    size_t tableStart = 8;
    size_t tableSize = (size_t)fileCount * 8;
    if (tableStart + tableSize > buf.size()) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "offset/size table exceeds file size.";
        return; 
    }

    g_afsTable.resize(fileCount);
    for (uint32_t i = 0; i < fileCount; ++i)
    {
        size_t p = tableStart + i * 8;
        g_afsTable[i].offset = *(uint32_t*)(buf.data() + p);
        g_afsTable[i].size = *(uint32_t*)(buf.data() + p + 4);
    }
    Logging::LogDebug() << __FUNCTION__ << "offset/size table loaded.";
}

static bool PlayVoiceAfs(uint32_t index, float startSec, float endSec)
{
    if (index >= g_afsTable.size()) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "invalid index %u", index;
        return false; 
    }
    const AfsEntry& e = g_afsTable[index];
    Logging::LogDebug() << __FUNCTION__ << "index=%u offset=0x%X size=%u", index, e.offset, e.size;
    std::ifstream f(voicepath, std::ios::binary);
    if (!f.is_open()) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "cannot open AFS.";

        return false; 
    }
    f.seekg(e.offset, std::ios::beg);

    std::vector<BYTE> wavData(e.size);
    if (!f.read((char*)wavData.data(), e.size)) 
    { 
        Logging::LogDebug() << __FUNCTION__ << "reading block failed.";
        return false; 
    }

    PlayWavFromMemoryRange(wavData.data(), wavData.size(), startSec, endSec);
    return true;
}



static bool ShouldActivateSequence(DWORD room, DWORD cutscene, DWORD fade, float cutsceneTime)
{
    return (room == R_HTL_RESTAURANT && cutscene == CS_HTL_LAURA_PIANO && fade == 3 && cutsceneTime >= 1600.0f);
}

static bool StabilizeCutscene()
{
    bool first = (GetCutsceneID() == CS_HTL_LAURA_PIANO && GetCutsceneTimer() >= 1600.0f);
    if (first) return true;
    Logging::LogDebug() << __FUNCTION__ << " Cutscene changed — performing additional check...";
    Sleep(30);
    bool second = (GetCutsceneID() == CS_HTL_LAURA_PIANO && GetCutsceneTimer() >= 1600.0f);
    Logging::LogDebug() << __FUNCTION__ << " FALSE POSITIVE detected: first check failed, second succeeded.";
    return second;
}


static volatile WORD* g_lastTextValuePtr = nullptr;

void ResolveLastTextPtr()
{
    BYTE pattern[] = {
        0x66, 0x89, 0x15,
        0x5C, 0xF5, 0xF5, 0x01
    };

    BYTE* addr = (BYTE*)SearchAndGetAddresses(
        0x00480460, 0x004806D0, 0x0047FE60,
        pattern, sizeof(pattern),
        0, __FUNCTION__);

    if (!addr)
    {
        Logging::LogDebug() << __FUNCTION__ << "Could not locate last-text address pattern!";
        return;
    }

    DWORD absolute = *(DWORD*)(addr + 3);

    g_lastTextValuePtr = (volatile WORD*)absolute;

    Logging::LogDebug() << __FUNCTION__ << "Resolved last-text ptr = %p", g_lastTextValuePtr;
}

void SetLastTextValueTo9()
{
    if (!g_lastTextValuePtr)
    {
        Logging::LogDebug() << __FUNCTION__ << "pointer unresolved";
        return;
    }

    DWORD oldProtect;

    if (VirtualProtect((LPVOID)g_lastTextValuePtr, sizeof(WORD), PAGE_READWRITE, &oldProtect))
    {
        *g_lastTextValuePtr = 9;
        MemoryBarrier();
        VirtualProtect((LPVOID)g_lastTextValuePtr, sizeof(WORD), oldProtect, &oldProtect);
        Logging::LogDebug() << __FUNCTION__ << "wrote 9 to %p", g_lastTextValuePtr;
    }
    else
    {
        Logging::LogDebug() << __FUNCTION__ << "VirtualProtect failed", g_lastTextValuePtr;
    }
}


//It will only press the buttons automatically if the SH2:EE window is in focus.
static bool WaitForGameFocusOrStop(DWORD sleepMs = 50)
{
    while (GetForegroundWindow() != GameWindowHandle)
    {
        Sleep(sleepMs);
        if (InterlockedCompareExchange(&g_stopAutoplay, 0, 0) == 1)
            return false;
    }

    return true;
}

static void InjectAction() 
{
    InterlockedExchange(&g_forceAction, 1);
    Sleep(80);
    InterlockedExchange(&g_forceAction, 0);
}

// Called from InputTweaks when player presses Skip during sequence
extern "C" void StopAutoplayImmediate()
{
    Logging::LogDebug() << __FUNCTION__ << "stopping autoplay.";

    InterlockedExchange(&g_stopAutoplay, 1);

    StopAlertSound();
    SetLastTextValueTo9();

    InterlockedExchange(&g_forceAction, 1);
}

inline LONG AtomicRead(volatile LONG* target) {
    return InterlockedCompareExchange(target, 0, 0);
}


DWORD WINAPI AudioMonitorThread(LPVOID) {
    static const float seqStart[] = { 0.0f, 7.2f, 13.0f, 17.1f, 19.2f, 22.5f, 35.2f, 47.2f, 53.4f, 61.2f, 64.0f };
    static const float seqEnd[] = { 7.0f, 12.4f, 16.9f, 19.0f, 22.0f, 34.5f, 46.2f, 52.2f, 60.2f, 63.5f, 67.2f };

    while (true) {
        Sleep(100);

        if (GetRoomID() != R_HTL_RESTAURANT) {
            if (InterlockedExchange(&g_blockPlayerInputs, 0) == 1) StopAlertSound();
            continue;
        }

        if (ShouldActivateSequence(GetRoomID(), GetCutsceneID(), GetTransitionState(), GetCutsceneTimer())) 
        {
            InterlockedExchange(&g_blockPlayerInputs, 1);
            InterlockedExchange(&g_stopAutoplay, 0);

            Sleep(500);
            if (WaitForGameFocusOrStop()) 
                InjectAction();

            for (int i = 0; i < 11; ++i) 
            {
                // Safety check: Exit loop if player skipped the cutscene
                if (AtomicRead(&g_stopAutoplay)) break;
                if (!StabilizeCutscene()) break;

                PlayVoiceAfs(119, seqStart[i], seqEnd[i]);

                // Controlled wait loop allowing responsive interruption
                DWORD waitMs = (DWORD)((seqEnd[i] - seqStart[i]) * 1000.0f) + 200;
                for (DWORD t = 0; t < waitMs; t += 50) {
                    if (AtomicRead(&g_stopAutoplay)) goto end_sequence;
                    Sleep(50);
                }

                if (WaitForGameFocusOrStop()) {
                    InjectAction();
                    Sleep(100);
                }
            }

        end_sequence:
            StopAlertSound();
            InterlockedExchange(&g_blockPlayerInputs, 0);

            while (StabilizeCutscene() && !AtomicRead(&g_stopAutoplay)) 
                Sleep(200);
        }
    }
    return 0;
}

// Entry point: locate memory address, verify AFS, start monitor thread.
void PatchUnusedAudio()
{
    if (GameVersion != SH2V_10 && GameVersion != SH2V_11)
    {
        Logging::Log() << "[PlayUnusedAudio] Disabled: incompatible game version";
        return;
    }

    if (InterlockedExchange(&gStarted, 1) != 0) 
        return;

    char path[MAX_PATH]; 
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string s = path; size_t p = s.find_last_of("\\/"); if (p != std::string::npos) s = s.substr(0, p);
    g_dllDir = s;

    Logging::LogDebug() << __FUNCTION__ << "Starting PlayUnusedAudio...";
    voicepath = g_dllDir + "\\sh2e\\sound\\adx\\voice\\voice.afs";
    {
        std::ifstream test(voicepath, std::ios::binary);
        if (!test.is_open()) 
        { 
            Logging::LogDebug() << __FUNCTION__ << "voice.afs NOT found: %s", voicepath.c_str();
        return; 
        }
    }

    ResolveLastTextPtr();
    Logging::LogDebug() << __FUNCTION__ << "AFS file found. Continuing...";
    InitVoiceAFS();
    CreateThread(NULL, 0, AudioMonitorThread, NULL, 0, NULL);
}