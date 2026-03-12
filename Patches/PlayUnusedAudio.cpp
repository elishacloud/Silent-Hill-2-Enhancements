// playunusedaudio.cpp
#include <windows.h>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include "Common\FileSystemHooks.h"
#include "Common\Settings.h"
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Wrappers/dsound/dsoundwrapper.h"
#include <shlwapi.h>

void PatchUnusedAudio();

volatile LONG g_forceAction = 0;

struct AfsEntry { uint32_t offset; uint32_t size; };

static std::string g_dllDir;
static std::string voicepath;
extern "C" IMAGE_DOS_HEADER __ImageBase;

static std::vector<AfsEntry> g_afsTable;
static volatile LONG gStarted = 0;

constexpr DWORD UnusedLauraLetterBufferID = 3;

volatile LONG g_blockPlayerInputs = 0;
volatile LONG g_stopAutoplay = 0;

static void StopAlertSound()
{
    m_IDirectSound8::StopWavSoundBuffer(UnusedLauraLetterBufferID);
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

// Plays only a slice of a WAV buffer (start/end seconds) directly from memory
static void PlayWavFromMemoryRange(BYTE* wavBuf, size_t wavSize, float startSec, float endSec)
{
    if (!wavBuf || wavSize < 44)
        return;

    uint32_t fmtSize = *(uint32_t*)(wavBuf + 16);
    if (fmtSize < 16)
        return;

    WAVEFORMATEX* wf = (WAVEFORMATEX*)(wavBuf + 20);

    DWORD dataPos = 20 + fmtSize + 8;
    if (dataPos >= wavSize)
        return;

    BYTE* audio = wavBuf + dataPos;
    DWORD audioSize = (DWORD)(wavSize - dataPos);
    DWORD frameBytes = (wf->wBitsPerSample / 8) * wf->nChannels;
    if (frameBytes == 0 || wf->nSamplesPerSec == 0)
        return;

    DWORD start = (DWORD)(startSec * wf->nSamplesPerSec * frameBytes);
    DWORD end = (DWORD)(endSec * wf->nSamplesPerSec * frameBytes);

    if (start > audioSize) start = 0;
    if (end > audioSize) end = audioSize;
    if (end <= start) return;

    DWORD playLen = end - start;
    std::vector<BYTE> pcm(playLen);
    memcpy(pcm.data(), audio + start, playLen);

    m_IDirectSound8::StopWavSoundBuffer(UnusedLauraLetterBufferID);
    m_IDirectSound8::PlayWavMemory(wf, pcm.data(), playLen, UnusedLauraLetterBufferID, false);
}

static void InitVoiceAFS()
{
    std::vector<BYTE> buf;
    if (!LoadFileToMemory(voicepath, buf))
        return;
    if (buf.size() < 16 || memcmp(buf.data(), "AFS\0", 4) != 0)
        return;

    uint32_t fileCount = *(uint32_t*)(buf.data() + 4);

    size_t tableStart = 8;
    size_t tableSize = (size_t)fileCount * 8;
    if (tableStart + tableSize > buf.size())
        return;

    g_afsTable.resize(fileCount);
    for (uint32_t i = 0; i < fileCount; ++i)
    {
        size_t p = tableStart + i * 8;
        g_afsTable[i].offset = *(uint32_t*)(buf.data() + p);
        g_afsTable[i].size = *(uint32_t*)(buf.data() + p + 4);
    }
}

static bool PlayVoiceAfs(uint32_t index, float startSec, float endSec)
{
    if (index >= g_afsTable.size())
        return false;
    const AfsEntry& e = g_afsTable[index];
    std::ifstream f(voicepath, std::ios::binary);
    if (!f.is_open())
        return false;

    f.seekg(e.offset, std::ios::beg);

    std::vector<BYTE> wavData(e.size);
    if (!f.read((char*)wavData.data(), e.size))
        return false;

    PlayWavFromMemoryRange(wavData.data(), wavData.size(), startSec, endSec);
    return true;
}

static bool ShouldActivateSequence(DWORD room, DWORD cutscene, DWORD fade, float cutsceneTime)
{
    return (room == R_HTL_RESTAURANT && cutscene == CS_HTL_LAURA_PIANO && fade == 3 && cutsceneTime >= 1600.0f);
}

static bool StabilizeCutscene()
{
    bool first = (GetCutsceneID() == CS_HTL_LAURA_PIANO && GetCutsceneTimer() >= 1600.0f && GetTransitionState() != 3);
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
        Logging::LogDebug() << __FUNCTION__ << " Could not locate last-text address pattern!";
        return;
    }

    DWORD absolute = *(DWORD*)(addr + 3);
    g_lastTextValuePtr = (volatile WORD*)absolute;

    Logging::LogDebug() << __FUNCTION__ << " Resolved last-text ptr = %p", g_lastTextValuePtr;
}

void SetLastTextValueTo9()
{
    if (!g_lastTextValuePtr)
    {
        Logging::LogDebug() << __FUNCTION__ << " pointer unresolved";
        return;
    }

    DWORD oldProtect;

    if (VirtualProtect((LPVOID)g_lastTextValuePtr, sizeof(WORD), PAGE_READWRITE, &oldProtect))
    {
        *g_lastTextValuePtr = 9;
        MemoryBarrier();
        VirtualProtect((LPVOID)g_lastTextValuePtr, sizeof(WORD), oldProtect, &oldProtect);
        Logging::LogDebug() << __FUNCTION__ << " wrote 9 to %p", g_lastTextValuePtr;
    }
    else
    {
        Logging::LogDebug() << __FUNCTION__ << " VirtualProtect failed", g_lastTextValuePtr;
    }
}

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

void StopAutoplayImmediate()
{
    Logging::LogDebug() << __FUNCTION__ << " stopping autoplay.";

    InterlockedExchange(&g_stopAutoplay, 1);

    StopAlertSound();
    SetLastTextValueTo9();

    InterlockedExchange(&g_forceAction, 1);
}

inline LONG AtomicRead(volatile LONG* target)
{
    return InterlockedCompareExchange(target, 0, 0);
}

DWORD WINAPI AudioMonitorThread(LPVOID)
{
    static const float seqStart[] = { 0.0f, 7.2f, 13.0f, 17.1f, 19.2f, 22.5f, 35.2f, 47.2f, 53.4f, 61.2f, 64.0f };
    static const float seqEnd[] = { 7.0f, 12.4f, 16.9f, 19.0f, 22.0f, 34.5f, 46.2f, 52.2f, 60.2f, 63.5f, 67.2f };

    while (true)
    {
        Sleep(100);

        if (GetRoomID() != R_HTL_RESTAURANT)
        {
            if (InterlockedExchange(&g_blockPlayerInputs, 0) == 1)
                StopAlertSound();
            continue;
        }

        if (ShouldActivateSequence(GetRoomID(), GetCutsceneID(), GetTransitionState(), GetCutsceneTimer()))
        {
            InterlockedExchange(&g_blockPlayerInputs, 1);
            InterlockedExchange(&g_stopAutoplay, 0);

            Sleep(1500);
            if (WaitForGameFocusOrStop())
                InjectAction();

            for (int i = 0; i < 11; ++i)
            {
                if (AtomicRead(&g_stopAutoplay)) break;
                if (!StabilizeCutscene()) break;

                PlayVoiceAfs(119, seqStart[i], seqEnd[i]);

                DWORD waitMs = (DWORD)((seqEnd[i] - seqStart[i]) * 1000.0f) + 200;
                for (DWORD t = 0; t < waitMs; t += 50)
                {
                    if (AtomicRead(&g_stopAutoplay) || !StabilizeCutscene())
                        goto end_sequence;

                    Sleep(50);
                }

                if (WaitForGameFocusOrStop())
                {
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
    std::string s = path;
    size_t p = s.find_last_of("\\/");
    if (p != std::string::npos) s = s.substr(0, p);
    g_dllDir = s;

    Logging::LogDebug() << __FUNCTION__ << " Starting PlayUnusedAudio...";
    voicepath = g_dllDir + "\\" + GetModPath("") + "\\sound\\adx\\voice\\voice.afs";
    {
        std::ifstream test(voicepath, std::ios::binary);
        if (!test.is_open())
        {
            Logging::LogDebug() << __FUNCTION__ << " voice.afs NOT found: " << voicepath.c_str();
            return;
        }
    }

    ResolveLastTextPtr();
    Logging::LogDebug() << __FUNCTION__ << " AFS file found. Continuing...";
    InitVoiceAFS();
    CreateThread(NULL, 0, AudioMonitorThread, NULL, 0, NULL);
}
