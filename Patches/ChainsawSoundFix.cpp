/**
* Copyright (C) 2025 Murugo
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

constexpr float kChainsawIdleFadeStartTimeSec = 1.0f;
constexpr float kChainsawIdleFadeSpeedSec = 0.1f;
constexpr int kChainsawIdleStartSoundIndex = 0x2B24;
constexpr int kChainsawIdleLoopSoundIndex = 0x1642;
constexpr int kChainsawAttackSoundIndex = 0x2B25;

constexpr float kFrameTimeThirtyFPS = 1.0f / 30.0f;
constexpr float kFrameTimeSixtyFPS = 1.0f / 60.0f;

DWORD PlayerLastHitResult = 0;
BYTE ChainsawIdleState = 0;
float ChainsawIdleTimer = 0.0f;
float ChainsawIdleVolumeFactor = 0.0f;
bool ChainsawAttackActive = false;
bool ChainsawAttackSoundPlayed = false;

int(*shSetSoundVolume)(int sound_index, float volume, float* pos, byte status) = nullptr;
int(*shPlaySoundDirectional)(int sound_index, float volume, float* pos, int status) = nullptr;
float *WeaponVolumePtr = nullptr;
DWORD *CollisionResultPtr = nullptr;
BYTE *AttackActivePtr = nullptr;

DWORD jmpUpdateLastHitResultReturnAddr = 0;
DWORD jmpPlayChainsawHitSoundReturnAddr1 = 0;
DWORD jmpPlayChainsawHitSoundReturnAddr2 = 0;
DWORD jmpSilenceChainsawLoopReturnAddr = 0;
DWORD jmpSetChainsawLoopVolumeReturnAddr1 = 0;
DWORD jmpSetChainsawLoopVolumeReturnAddr2 = 0;
DWORD jmpSetChainsawLoopBufferLengthReturnAddr = 0;
DWORD jmpSetChainsawLoopBufferStartReturnAddr = 0;
DWORD jmpCheckChainsawAttackStartReturnAddr1 = 0;
DWORD jmpCheckChainsawAttackStartReturnAddr2 = 0;

// Stores the last weapon hit result for the current frame.
__declspec(naked) void __stdcall UpdateLastHitResultASM()
{
    __asm
    {
        mov edi, dword ptr ds : [esi + 0x124]
        mov dword ptr ds : [PlayerLastHitResult], edi
        or bl, -1
        test eax, eax
        jmp jmpUpdateLastHitResultReturnAddr
    }
}

// Plays the chainsaw hit sound on the first frame of the hit only.
__declspec(naked) void __stdcall PlayChainsawHitSoundASM()
{
    __asm
    {
        cmp dword ptr ds : [PlayerLastHitResult], 0x01
        jne ExitASM
        jmp jmpPlayChainsawHitSoundReturnAddr1

    ExitASM:
        mov eax, 0x2B26
        push 0x00
        lea ecx, [esi + 0x1C]
        push ecx
        push 0x3F800000  // Volume = 1.0
        jmp jmpPlayChainsawHitSoundReturnAddr2
    }
}

// Initially plays the chainsaw idle sound at a volume of zero, and starts a timer to fade in the loop (see `RunChainsawSoundFix()`).
__declspec(naked) void __stdcall SilenceChainsawLoopASM()
{
    __asm
    {
        mov byte ptr ds : [ChainsawIdleState], 0x01
        mov edi, dword ptr ds : [kChainsawIdleFadeStartTimeSec]
        mov dword ptr ds : [ChainsawIdleTimer], edi
        push 0xFF
        push 0xFF
        push 0x1642
        jmp jmpSilenceChainsawLoopReturnAddr
    }
}

// Allows setting the volume of the chainsaw loop sound directly.
__declspec(naked) void __stdcall SetChainsawLoopVolumeASM()
{
    __asm
    {
        mov esi, dword ptr ds : [esp + 0x08]
        mov eax, 0x1642
        cmp esi, eax
        jne ExitASM
        jmp jmpSetChainsawLoopVolumeReturnAddr1

    ExitASM:
        xor eax, eax
        jmp jmpSetChainsawLoopVolumeReturnAddr2
    }
}

// Adjusts the length of the chainsaw idle loop sound buffer at 48,000 Hz.
// Also reduces the length of the initial crank sound if James starts the
// chainsaw on his left side, since this animation plays faster.
__declspec(naked) void __stdcall SetChainsawLoopBufferLengthASM()
{
    __asm
    {
        push eax
        mov eax, dword ptr ds : [ebp + 0x18]
        cmp esi, 0x14E8
        je ChainsawCrank
        cmp esi, 0x1642
        je ChainsawIdleLoop
        jmp ExitASM

    ChainsawCrank:
        cmp eax, 48000  // Sample rate == 48000 Hz?
        jne ExitASM
        mov eax, dword ptr ds : [CollisionResultPtr]
        mov eax, dword ptr ds : [eax]
        test eax, eax
        jz ExitASM
        sub ebx, 0x7CFC
        jmp SetBufferLength

    ChainsawIdleLoop:
        cmp eax, 48000  // Sample rate = 48000 Hz?
        jne ChainsawIdleLoopOriginal
        sub ebx, 0x12180
        jmp SetBufferLength
    
    ChainsawIdleLoopOriginal:
        sub ebx, 0x4796

    SetBufferLength:
        mov dword ptr ds : [esp + 0x68], ebx

    ExitAsm:
        pop eax
        jmp jmpSetChainsawLoopBufferLengthReturnAddr
    }
}

// Adjusts the start offset of the chainsaw idle loop sound buffer at 48,000 Hz.
// Also cuts silence at the beginning of the initial crank sound if James starts
// the chainsaw on his left side, since this animation plays faster.
__declspec(naked) void __stdcall SetChainsawLoopBufferStartASM()
{
    __asm
    {
        push eax
        push ecx
        mov eax, dword ptr ds : [ebp + 0x18]
        mov ecx, esi
        lea esi, dword ptr ds : [ebp + 0x2C]
        cmp ecx, 0x14E8
        je ChainsawCrank
        cmp ecx, 0x1642
        je ChainsawIdleLoop
        jmp ExitASM

    ChainsawCrank:
        cmp eax, 48000  // Sample rate == 48000 Hz?
        jne ExitASM
        mov eax, dword ptr ds : [CollisionResultPtr]
        mov eax, dword ptr ds : [eax]
        test eax, eax
        jz ExitASM
        lea esi, dword ptr ds : [ebp + 0x7D28]  // 0x7CFC + 0x2C (RIFF header size)
        jmp ExitASM

    ChainsawIdleLoop:
        cmp eax, 48000  // Sample rate = 48000 Hz?
        jne ChainsawIdleLoopOriginal
        lea esi, dword ptr ds : [ebp + 0x121AC]  // 0x12180 + 0x2C (RIFF header size)
        jmp ExitASM

    ChainsawIdleLoopOriginal:
        lea esi, dword ptr ds : [ebp + 0x47C2]

    ExitASM:
        pop ecx
        pop eax
        jmp jmpSetChainsawLoopBufferStartReturnAddr
    }
}

// Sets `ChainsawAttackActive` to true on frame 0 of the attack animation.
__declspec(naked) void __stdcall CheckChainsawAttackStartASM()
{
    __asm
    {
        push edx
        cmp eax, 0x07
        jne ExitASM
        mov edx, dword ptr ds : [esp + 0x14]  // Current frame
        test edx, edx
        jnz NotAttackStart
        mov edx, dword ptr ds : [AttackActivePtr]
        movzx edx, byte ptr ds : [edx]
        test dl, dl
        jz NotAttackStart
        mov byte ptr ds : [ChainsawAttackActive], 0x01
        jmp ExitASM

    NotAttackStart:
        mov byte ptr ds : [ChainsawAttackActive], 0x00

    ExitASM:
        pop edx
        dec eax
        cmp eax, 0x07
        ja SkipSwitch
        jmp jmpCheckChainsawAttackStartReturnAddr1

    SkipSwitch:
        jmp jmpCheckChainsawAttackStartReturnAddr2
    }
}

void PatchChainsawSoundFix()
{
    const BYTE ChainsawHitSearchBytes[]{ 0xB8, 0x26, 0x2B, 0x00, 0x00, 0xE9, 0x4B };
    const DWORD ChainsawHitAddr = SearchAndGetAddresses(0x005358C3, 0x00535BF3, 0x00535513, ChainsawHitSearchBytes, sizeof(ChainsawHitSearchBytes), 0x00, __FUNCTION__);

    const BYTE SetSoundDamageSearchBytes[]{ 0xBA, 0x26, 0x2B, 0x00, 0x00, 0xEB, 0xD9 };
    const DWORD SetSoundDamageAddr = SearchAndGetAddresses(0x00543612, 0x00543942, 0x00543262, SetSoundDamageSearchBytes, sizeof(SetSoundDamageSearchBytes), 0x00, __FUNCTION__);

    const BYTE SetSoundVolumeSearchBytes[]{ 0x6A, 0x00, 0x68, 0x54, 0x46, 0x00, 0x00 };
    const DWORD SetSoundVolumeAddr = SearchAndGetAddresses(0x00582F39, 0x005837E9, 0x00583109, SetSoundVolumeSearchBytes, sizeof(SetSoundVolumeSearchBytes), 0x34, __FUNCTION__);

    const BYTE SilenceChainsawLoopSearchBytes[]{ 0x57, 0x55, 0x68, 0x42, 0x16, 0x00, 0x00 };
    const DWORD SilenceChainsawLoopAddr = SearchAndGetAddresses(0x00514916, 0x00514C46, 0x00514566, SilenceChainsawLoopSearchBytes, sizeof(SilenceChainsawLoopSearchBytes), 0x00, __FUNCTION__);

    constexpr BYTE SetChainsawLoopVolumeSearchBytes[]{ 0x56, 0x8B, 0x74, 0x24, 0x08, 0x33, 0xC0, 0x39 };
    const DWORD SetChainsawLoopVolumeAddr = SearchAndGetAddresses(0x00514DD0, 0x00515100, 0x00514A20, SetChainsawLoopVolumeSearchBytes, sizeof(SetChainsawLoopVolumeSearchBytes), 0x01, __FUNCTION__);

    constexpr BYTE SetChainsawLoopBufferLengthSearchBytes[]{ 0xC7, 0x44, 0x24, 0x34, 0x90, 0x02, 0x04, 0x00 };
    const DWORD SetChainsawLoopBufferLengthAddr = SearchAndGetAddresses(0x0051790A, 0x00517C3A, 0x0051755A, SetChainsawLoopBufferLengthSearchBytes, sizeof(SetChainsawLoopBufferLengthSearchBytes), 0x1A, __FUNCTION__);

    constexpr BYTE CollisionResultSearchBytes[]{ 0x66, 0x3D, 0x95, 0x01, 0x74, 0x10 };
    CollisionResultPtr = (DWORD*)ReadSearchedAddresses(0x005329AD, 0x00532CDD, 0x005325FD, CollisionResultSearchBytes, sizeof(CollisionResultSearchBytes), 0x08, __FUNCTION__);

    constexpr BYTE PlayChainsawAttackSoundSearchBytes[]{ 0x8D, 0x84, 0x76, 0x8E, 0xF9, 0xFF, 0xFF, 0x83, 0xC4, 0x10 };
    const DWORD PlayChainsawAttackSoundAddr = SearchAndGetAddresses(0x005316CC, 0x005319FC, 0x0053131C, PlayChainsawAttackSoundSearchBytes, sizeof(PlayChainsawAttackSoundSearchBytes), -0x05, __FUNCTION__);

    constexpr BYTE ChainsawIdleInitVolumeSearchBytes[]{ 0x53, 0x83, 0xC1, 0x1C, 0x51, 0x68 };
    const DWORD ChainsawIdleInitVolumeAddr = SearchAndGetAddresses(0x00530DAE, 0x005310DE, 0x005309FE, ChainsawIdleInitVolumeSearchBytes, sizeof(ChainsawIdleInitVolumeSearchBytes), 0x15, __FUNCTION__);

    constexpr BYTE AttackActiveSearchBytes[]{ 0xD9, 0x9F, 0xAC, 0x00, 0x00, 0x00, 0x84, 0x15 };
    AttackActivePtr = (BYTE*)ReadSearchedAddresses(0x00558ED8, 0x00559208, 0x00558B28, AttackActiveSearchBytes, sizeof(AttackActiveSearchBytes), 0x10, __FUNCTION__);

    constexpr BYTE CheckChainsawAttackStartSearchBytes[]{ 0x48, 0x83, 0xF8, 0x07, 0x0F, 0x87, 0xBB, 0x02 };
    const DWORD CheckChainsawAttackStartAddr = SearchAndGetAddresses(0x0054A592, 0x0054A8C2, 0x0054A1E2, CheckChainsawAttackStartSearchBytes, sizeof(CheckChainsawAttackStartSearchBytes), 0x00, __FUNCTION__);

    if (!ChainsawHitAddr || !SetSoundDamageAddr || !SetSoundVolumeAddr || !SilenceChainsawLoopAddr ||
        !SetChainsawLoopVolumeAddr || !SetChainsawLoopBufferLengthAddr || !CollisionResultPtr ||
        !PlayChainsawAttackSoundAddr || !ChainsawIdleInitVolumeAddr || !AttackActivePtr ||
        !CheckChainsawAttackStartAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    jmpPlayChainsawHitSoundReturnAddr1 = ChainsawHitAddr - 0x97;
    jmpPlayChainsawHitSoundReturnAddr2 = ChainsawHitAddr - 0xA0;
    jmpUpdateLastHitResultReturnAddr = ChainsawHitAddr - 0x8D;
    jmpSilenceChainsawLoopReturnAddr = SilenceChainsawLoopAddr + 0x06;
    jmpSetChainsawLoopVolumeReturnAddr1 = SetChainsawLoopVolumeAddr + 0x49;
    jmpSetChainsawLoopVolumeReturnAddr2 = SetChainsawLoopVolumeAddr + 0x06;
    jmpSetChainsawLoopBufferLengthReturnAddr = SetChainsawLoopBufferLengthAddr + 0x12;
    jmpSetChainsawLoopBufferStartReturnAddr = SetChainsawLoopBufferLengthAddr + 0x182;
    jmpCheckChainsawAttackStartReturnAddr1 = CheckChainsawAttackStartAddr + 0x0A;
    jmpCheckChainsawAttackStartReturnAddr2 = CheckChainsawAttackStartAddr + 0x2C5;
    const DWORD SetChainsawLoopBufferStartAddr = SetChainsawLoopBufferLengthAddr + 0x171;

    Logging::Log() << "Patching Chainsaw Sound Fixes...";

    WriteJMPtoMemory((BYTE*)ChainsawHitAddr, *PlayChainsawHitSoundASM, 0x05);
    WriteJMPtoMemory((BYTE*)(ChainsawHitAddr - 0x92), *UpdateLastHitResultASM, 0x05);
    WriteJMPtoMemory((BYTE*)SilenceChainsawLoopAddr, *SilenceChainsawLoopASM, 0x07);
    WriteJMPtoMemory((BYTE*)SetChainsawLoopVolumeAddr, *SetChainsawLoopVolumeASM, 0x06);
    WriteJMPtoMemory((BYTE*)SetChainsawLoopBufferLengthAddr, *SetChainsawLoopBufferLengthASM, 0x06);
    WriteJMPtoMemory((BYTE*)SetChainsawLoopBufferStartAddr, *SetChainsawLoopBufferStartASM, 0x06);
    WriteJMPtoMemory((BYTE*)CheckChainsawAttackStartAddr, *CheckChainsawAttackStartASM, 0x0A);

    // There are two functions that play the chainsaw hit sound every frame. Skip the second callsite.
    UpdateMemoryAddress((void*)SetSoundDamageAddr, "\x5F\x5B\x59\xC3\x90", 0x05);

    // Skip the original function call that plays the chainsaw attack sound. This is now called in RunChainsawSoundFix().
    // This instruction sometimes runs before the start of the animation, causing the sound to desync.
    UpdateMemoryAddress((void*)PlayChainsawAttackSoundAddr, "\x90\x90\x90\x90\x90", 0x05);

    // Set volume of other chainsaw sound effects to 1.0.
    UpdateMemoryAddress((void*)ChainsawIdleInitVolumeAddr, "\x00\x00\x80\x3F", 0x04);
    UpdateMemoryAddress((void*)(ChainsawHitAddr - 0xF0), "\x00\x00\x80\x3F", 0x04);
}

// Controls the pan and volume of the chainsaw idle sound.
void RunChainsawSoundFix()
{
    if (!shSetSoundVolume)
    {
        RUNONCE();

        constexpr BYTE SetSoundVolumeSearchBytes[]{ 0x8B, 0x4C, 0x24, 0x20, 0x8B, 0x54, 0x24, 0x1C, 0x53 };
        const DWORD SetSoundVolumeAddr = SearchAndGetAddresses(0x005170D3, 0x00517403, 0x00516D23, SetSoundVolumeSearchBytes, sizeof(SetSoundVolumeSearchBytes), 0x0D, __FUNCTION__);

        constexpr BYTE WeaponVolumeSearchBytes[]{ 0x6A, 0x00, 0x83, 0xC0, 0x1C, 0x50, 0xD9, 0x1D };
        WeaponVolumePtr = (float*)ReadSearchedAddresses(0x0054A915, 0x0054AC45, 0x0054A565, WeaponVolumeSearchBytes, sizeof(WeaponVolumeSearchBytes), 0x08, __FUNCTION__);

        if (!SetSoundVolumeAddr || !WeaponVolumePtr)
        {
            Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
            return;
        }
        shSetSoundVolume = (int(*)(int, float, float*, byte))((BYTE*)(SetSoundVolumeAddr + 0x04) + *(DWORD*)SetSoundVolumeAddr);
        shPlaySoundDirectional = (int(*)(int, float, float*, int))(SetSoundVolumeAddr - 0x250);
    }
    if (!shSetSoundVolume || !shPlaySoundDirectional || !WeaponVolumePtr || !AttackActivePtr) return;

    if (ChainsawIdleState == 0 || *WeaponVolumePtr < 1e-5f)
    {
        ChainsawIdleState = 0;
        ChainsawIdleVolumeFactor = 0.0f;
        ChainsawIdleTimer = 0.0f;
        ChainsawAttackActive = false;
        ChainsawAttackSoundPlayed = false;
        return;
    }

    // Play the chainsaw attack sound at the start of the attack animation.
    if (ChainsawAttackActive)
    {
        if (!ChainsawAttackSoundPlayed)
        {
            shPlaySoundDirectional(kChainsawAttackSoundIndex, *WeaponVolumePtr, GetJamesPosXPointer(), /*status=*/0);
            ChainsawAttackSoundPlayed = true;
        }
    }
    else
    {
        ChainsawAttackSoundPlayed = false;
    }
    
    float IdleVolume = 0.0f;
    float IdleStartVolume = 0.0f;
    switch (ChainsawIdleState)
    {
    // Wait until the start of the custom chainsaw idle loop point.
    case 1:
        ChainsawIdleTimer -= SetSixtyFPS ? kFrameTimeSixtyFPS : kFrameTimeThirtyFPS;
        if (ChainsawIdleTimer < 0.0f) {
            ChainsawIdleTimer = 0.0f;
            ChainsawIdleVolumeFactor = 0.0f;
            ChainsawIdleState = 2;
        }
        IdleStartVolume = *WeaponVolumePtr;
        break;

    // Cross-fade between the start of the chainsaw idle sound and the looping portion.
    case 2:
        ChainsawIdleVolumeFactor += (SetSixtyFPS ? kFrameTimeSixtyFPS : kFrameTimeThirtyFPS) / kChainsawIdleFadeSpeedSec;
        if (ChainsawIdleVolumeFactor >= 1.0f) {
            ChainsawIdleVolumeFactor = 1.0f;
            ChainsawIdleState = 3;
        }
        IdleStartVolume = (1.0f - pow(ChainsawIdleVolumeFactor, 3)) * *WeaponVolumePtr;
        IdleVolume = (1.0f - pow(1.0f - ChainsawIdleVolumeFactor, 3)) * *WeaponVolumePtr;
        break;

    // Continue to update the idle sound's world position while the chainsaw is active.
    case 3:
        // Silence the idle loop while attacking with the chainsaw.
        ChainsawIdleVolumeFactor = *AttackActivePtr != 0 ? 0.0f : 1.0f;
        IdleVolume = ChainsawIdleVolumeFactor * *WeaponVolumePtr;
        break;

    default:
        break;
    }
    shSetSoundVolume(
        kChainsawIdleStartSoundIndex,
        ChainsawIdleState < 3 ? IdleStartVolume : 0.0f,
        GetJamesPosXPointer(),
        /*status=*/0
    );
    shSetSoundVolume(kChainsawIdleLoopSoundIndex, IdleVolume, GetJamesPosXPointer(), /*status=*/0);
}
