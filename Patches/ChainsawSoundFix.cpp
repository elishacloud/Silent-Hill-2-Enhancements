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
constexpr float kChainsawIdleFadeSpeedSec = 0.5f;

constexpr float kFrameTimeThirtyFPS = 1.0f / 30.0f;
constexpr float kFrameTimeSixtyFPS = 1.0f / 60.0f;

DWORD PlayerLastHitResult = 0;
BYTE ChainsawIdleState = 0;
float ChainsawIdleTimer = 0.0f;
float ChainsawIdleVolume = 0.0f;

int(*shSetSoundVolume)(int sound_index, float volume, float* pos, byte status) = nullptr;
float *WeaponVolumePtr = nullptr;
BYTE *CurrentAttackIndexPtr = nullptr;

DWORD jmpUpdateLastHitResultReturnAddr = 0;
DWORD jmpPlayChainsawHitSoundReturnAddr1 = 0;
DWORD jmpPlayChainsawHitSoundReturnAddr2 = 0;
DWORD jmpSilenceChainsawLoopReturnAddr = 0;
DWORD jmpSetChainsawLoopVolumeReturnAddr1 = 0;
DWORD jmpSetChainsawLoopVolumeReturnAddr2 = 0;

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

// Adjusts the length of the chainsaw idle loop sound buffer.
__declspec(naked) void __stdcall SetChainsawLoopBufferLengthASM()
{
    __asm
    {
        mov ecx, dword ptr ds : [ebp + 0x18]
        cmp ecx, 48000  // Sample rate == 48000 Hz?
        jne ExitASM
        sub ebx, 0x12180
        ret

    ExitASM:
        sub ebx, 0x4796
        ret
    }
}

// Adjusts the start offset of the chainsaw idle loop sound buffer.
__declspec(naked) void __stdcall SetChainsawLoopBufferStartASM()
{
    __asm
    {
        push ecx
        mov ecx, dword ptr ds : [ebp + 0x18]
        cmp ecx, 48000  // Sample rate == 48000 Hz?
        pop ecx
        jne ExitASM
        cmp esi, 0x1642
        lea esi, dword ptr ds : [ebp + 0x121AC]  // 0x12180 + 0x2C (RIFF header size)
        ret

    ExitASM:
        cmp esi, 0x1642
        lea esi, dword ptr ds : [ebp + 0x47C2]
        ret
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
    const DWORD SetChainsawLoopBufferLengthAddr = SearchAndGetAddresses(0x0051790A, 0x00517C3A, 0x0051755A, SetChainsawLoopBufferLengthSearchBytes, sizeof(SetChainsawLoopBufferLengthSearchBytes), 0x22, __FUNCTION__);

    if (!ChainsawHitAddr || !SetSoundDamageAddr || !SetSoundVolumeAddr || !SilenceChainsawLoopAddr || !SetChainsawLoopVolumeAddr || !SetChainsawLoopBufferLengthAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    jmpPlayChainsawHitSoundReturnAddr1 = ChainsawHitAddr - 0x97;
    jmpPlayChainsawHitSoundReturnAddr2 = ChainsawHitAddr + 0x05;
    jmpUpdateLastHitResultReturnAddr = ChainsawHitAddr - 0x8D;
    jmpSilenceChainsawLoopReturnAddr = SilenceChainsawLoopAddr + 0x06;
    jmpSetChainsawLoopVolumeReturnAddr1 = SetChainsawLoopVolumeAddr + 0x49;
    jmpSetChainsawLoopVolumeReturnAddr2 = SetChainsawLoopVolumeAddr + 0x06;
    const DWORD SetChainsawLoopBufferStartAddr = SetChainsawLoopBufferLengthAddr + 0x169;

    Logging::Log() << "Patching Chainsaw Sound Fixes...";

    WriteJMPtoMemory((BYTE*)ChainsawHitAddr, *PlayChainsawHitSoundASM, 0x05);
    WriteJMPtoMemory((BYTE*)(ChainsawHitAddr - 0x92), *UpdateLastHitResultASM, 0x05);
    WriteJMPtoMemory((BYTE*)SilenceChainsawLoopAddr, *SilenceChainsawLoopASM, 0x07);
    WriteJMPtoMemory((BYTE*)SetChainsawLoopVolumeAddr, *SetChainsawLoopVolumeASM, 0x06);
    WriteCalltoMemory((BYTE*)SetChainsawLoopBufferLengthAddr, *SetChainsawLoopBufferLengthASM, 0x06);
    WriteCalltoMemory((BYTE*)SetChainsawLoopBufferStartAddr, *SetChainsawLoopBufferStartASM, 0x0C);

    // There are two functions that play the chainsaw hit sound every frame. Skip the second callsite.
    UpdateMemoryAddress((void*)SetSoundDamageAddr, "\x5F\x5B\x59\xC3\x90", 0x05);
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

        constexpr BYTE CurrentAttackIndexSearchBytes[]{ 0xE9, 0x15, 0x04, 0x00, 0x00 };
        CurrentAttackIndexPtr = (BYTE*)ReadSearchedAddresses(0x0053045F, 0x0053078F, 0x005300AF, CurrentAttackIndexSearchBytes, sizeof(CurrentAttackIndexSearchBytes), 0x07, __FUNCTION__);

        if (!SetSoundVolumeAddr || !WeaponVolumePtr || !CurrentAttackIndexPtr)
        {
            Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
            return;
        }
        shSetSoundVolume = (int(*)(int, float, float*, byte))((BYTE*)(SetSoundVolumeAddr + 0x04) + *(DWORD*)SetSoundVolumeAddr);
    }

    if (!shSetSoundVolume || !WeaponVolumePtr || !CurrentAttackIndexPtr) return;
    if (ChainsawIdleState == 0 || *WeaponVolumePtr < 1e-5f)
    {
        ChainsawIdleState = 0;
        ChainsawIdleVolume = 0.0f;
        ChainsawIdleTimer = 0.0f;
        return;
    }
    
    switch (ChainsawIdleState)
    {
    // Wait until the start of the chainsaw idle loop point.
    case 1:
        ChainsawIdleTimer -= SetSixtyFPS ? kFrameTimeSixtyFPS : kFrameTimeThirtyFPS;
        if (ChainsawIdleTimer < 0.0f) {
            ChainsawIdleTimer = 0.0f;
            ChainsawIdleVolume = 0.0f;
            ChainsawIdleState = 2;
        }
        break;

    // Cross-fade between the start of the chainsaw idle sound and the looping portion.
    case 2:
        ChainsawIdleVolume += (SetSixtyFPS ? kFrameTimeSixtyFPS : kFrameTimeThirtyFPS) / kChainsawIdleFadeSpeedSec;
        if (ChainsawIdleVolume >= *WeaponVolumePtr) {
            ChainsawIdleVolume = *WeaponVolumePtr;
            ChainsawIdleState = 3;
        }
        break;

    // Continue to update the idle sound's world position while the chainsaw is active.
    case 3:
        // Silence the idle loop while attacking with the chainsaw.
        ChainsawIdleVolume = *CurrentAttackIndexPtr == 0 ? *WeaponVolumePtr : 0.0f;
        break;

    default:
        break;
    }
    shSetSoundVolume(/*sound_index=*/0x2B24, /*volume=*/ChainsawIdleState < 3 ? (*WeaponVolumePtr - ChainsawIdleVolume) : 0.0f, /*pos=*/GetJamesPosXPointer(), /*status=*/0);
    shSetSoundVolume(/*sound_index=*/0x1642, /*volume=*/ChainsawIdleVolume, /*pos=*/GetJamesPosXPointer(), /*status=*/0);
}
