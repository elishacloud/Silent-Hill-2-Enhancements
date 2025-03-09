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
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace
{
    constexpr int kMusicBoxSolvedFlag = 0x1D8;

    // Variables for ASM
    BYTE* GameFlagAddr = 0;
    DWORD BgmChangeReturnAddr = 0;
    void(*shBgmCall)(int);

    bool IsHotelMusicBoxSolved()
    {
        return GameFlagAddr[kMusicBoxSolvedFlag >> 3] & (1 << (kMusicBoxSolvedFlag & 0x07));
    }

    // Returns the BGM bank index to use for Hotel 2F.
    int GetStageBgmBank()
    {
        return IsHotelMusicBoxSolved() ? 4 : 0;
    }

    // Loads "sound\adx\hotel\bgm_121.aix" in Hotel 2F only if the music box puzzle was solved.
    __declspec(naked) void __stdcall BgmChangeASM()
    {
        __asm
        {
            cmp ecx, 0x29
            jne ExitAsm
            call IsHotelMusicBoxSolved
            test al, al
            jz ExitAsm
            push 0x22
            call shBgmCall
            add esp, 0x04
            pop ebx
            ret

        ExitAsm:
            add ecx, -0x02
            cmp ecx, 0x38
            jmp BgmChangeReturnAddr
        }
    }
}

void PatchMusicBoxVolume()
{
    const DWORD Hotel2FStageAddr = 0x008CA3D0;  // 1.0 only

    constexpr BYTE GameFlagSearchBytes[]{ 0x83, 0xFE, 0x01, 0x55, 0x57, 0xBD, 0x00, 0x01, 0x00, 0x00 };
    GameFlagAddr = (BYTE*)ReadSearchedAddresses(0x0048AA9E, 0x0048AD3E, 0x0048AF4E, GameFlagSearchBytes, sizeof(GameFlagSearchBytes), 0x24, __FUNCTION__);
    if (!GameFlagAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    constexpr BYTE BgmChangeSearchBytes[]{ 0x83, 0xC1, 0xFE, 0x83, 0xF9, 0x38 };
    const DWORD BgmChangeAddr = SearchAndGetAddresses(0x0051601C, 0, 0, BgmChangeSearchBytes, sizeof(BgmChangeSearchBytes), 0x00, __FUNCTION__);
    if (!BgmChangeAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }
    BgmChangeReturnAddr = BgmChangeAddr + 0x06;
    shBgmCall = (void(*)(int))(BgmChangeAddr + 0x2B + *(DWORD*)(BgmChangeAddr + 0x27));

    // TODO: Can simply use &GetStageBgmBank ?
    int(*pfn)(void) = std::addressof(GetStageBgmBank);

    Logging::Log() << "Patching Music Box Volume...";
    // Set custom BGM control function for Hotel 2F.
    UpdateMemoryAddress((void*)(Hotel2FStageAddr + 0x30), &pfn, sizeof(&pfn));

    WriteJMPtoMemory((BYTE*)BgmChangeAddr, *BgmChangeASM, 0x06);

    // Update BGM sound regions in Hotel 1F
    const DWORD Hotel1FSoundDataAddr = 0x008B3DD0;  // 1.0 only
    const DWORD ChannelNoBanks = 0x00;
    const DWORD ChannelBank4 = 0x00100000;

    for (int i = 6; i <= 11; ++i)
    {
        // Muffled music box in 1F hallway
        UpdateMemoryAddress((void*)(Hotel1FSoundDataAddr + i * 0x40 + 0x34), &ChannelNoBanks, sizeof(DWORD));
        UpdateMemoryAddress((void*)(Hotel1FSoundDataAddr + i * 0x40 + 0x3C), &ChannelBank4, sizeof(DWORD));
    }
    for (int i = 12; i <= 18; ++i)
    {
        // No music box in rooms other than lobby and reception
        UpdateMemoryAddress((void*)(Hotel1FSoundDataAddr + i * 0x40 + 0x34), &ChannelNoBanks, sizeof(DWORD));
    }
    for (int i = 21; i <= 27; ++i)
    {
        // No music box in rooms other than lobby and reception
        UpdateMemoryAddress((void*)(Hotel1FSoundDataAddr + i * 0x40 + 0x34), &ChannelNoBanks, sizeof(DWORD));
    }

    // Update BGM sound regions in Hotel 2F
    const DWORD Hotel2FSoundDataAddr = 0x008B4510;  // 1.0 only
    const DWORD EnableAreaFlag = 0x40000010;

    for (int i = 3; i <= 9; ++i)
    {
        // Muffled music box in 2F hallways connected to the lobby
        UpdateMemoryAddress((void*)(Hotel2FSoundDataAddr + i * 0x40 + 0x20), &EnableAreaFlag, sizeof(DWORD));
        UpdateMemoryAddress((void*)(Hotel2FSoundDataAddr + i * 0x40 + 0x3C), &ChannelBank4, sizeof(DWORD));
    }
    // Muffled music box in cloak room
    UpdateMemoryAddress((void*)(Hotel2FSoundDataAddr + 12 * 0x40 + 0x20), &EnableAreaFlag, sizeof(DWORD));
    UpdateMemoryAddress((void*)(Hotel2FSoundDataAddr + 12 * 0x40 + 0x3C), &ChannelBank4, sizeof(DWORD));
}
