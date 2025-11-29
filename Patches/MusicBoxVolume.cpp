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
    // Game flag set after unlocking the music box in the Lakeview Hotel 1F lobby.
    constexpr int kMusicBoxSolvedFlag = 0x1D8;

    // Variables for ASM
    BYTE* GameFlagAddr = 0;
    DWORD BgmChangeReturnAddr = 0;
    void(*shBgmCall)(int);

    bool IsHotelMusicBoxSolved()
    {
        return GameFlagAddr[kMusicBoxSolvedFlag >> 3] & (1 << (kMusicBoxSolvedFlag & 0x07));
    }

    // Returns the BGM bank index to use for Lakeview Hotel 2F.
    int GetStageBgmBank()
    {
        return IsHotelMusicBoxSolved() ? 4 : 0;
    }

    // Loads "sound\adx\hotel\bgm_112.aix" for Lakeview Hotel 2F if the music box puzzle is solved.
    // Otherwise, loads "sound\adx\hotel\bgm_121.aix" as the default.
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

// Plays a quieter version of the music box in certain rooms of the Lakeview Hotel.
// This assumes that the quieter BGM is track 7 of "sound\adx\hotel\bgm_112.aix" (this track is silent in vanilla).
void PatchMusicBoxVolume()
{
    const BYTE Hotel1FSoundDataSearchBytes[]{ 0x00, 0x40, 0x83, 0xC6, 0x00, 0x40, 0xB5, 0x46, 0x00, 0x00, 0x48, 0x42 };
    const DWORD Hotel1FSoundDataAddr = SearchAndGetAddresses(0x008B3E20, 0x008B7A10, 0x008B6A10, Hotel1FSoundDataSearchBytes, sizeof(Hotel1FSoundDataSearchBytes), -0x50, __FUNCTION__);

    const BYTE Hotel2FSoundDataSearchBytes[]{ 0x00, 0xC4, 0x6A, 0xC7, 0x00, 0x68, 0xBF, 0x47, 0x00, 0x00, 0x48, 0x42 };
    const DWORD Hotel2FSoundDataAddr = SearchAndGetAddresses(0x008B44A0, 0x008B8090, 0x008B7090, Hotel2FSoundDataSearchBytes, sizeof(Hotel2FSoundDataSearchBytes), 0x70, __FUNCTION__);

    const BYTE Hotel2FStageSearchBytes[]{ 0x64, 0x68, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x74 };
    const DWORD Hotel2FStageAddr = SearchAndGetAddresses(0x008CA414, 0x008CE0E4, 0x008CD0E4, Hotel2FStageSearchBytes, sizeof(Hotel2FStageSearchBytes), -0x44, __FUNCTION__);

    const BYTE BgmChangeSearchBytes[]{ 0x83, 0xC1, 0xFE, 0x83, 0xF9, 0x38 };
    const DWORD BgmChangeAddr = SearchAndGetAddresses(0x0051601C, 0x0051634C, 0x00515C6C, BgmChangeSearchBytes, sizeof(BgmChangeSearchBytes), 0x00, __FUNCTION__);

    const BYTE GameFlagSearchBytes[]{ 0x83, 0xFE, 0x01, 0x55, 0x57, 0xBD, 0x00, 0x01, 0x00, 0x00 };
    GameFlagAddr = (BYTE*)ReadSearchedAddresses(0x0048AA9E, 0x0048AD3E, 0x0048AF4E, GameFlagSearchBytes, sizeof(GameFlagSearchBytes), 0x24, __FUNCTION__);

    if (!Hotel1FSoundDataAddr || !Hotel2FSoundDataAddr || !Hotel2FStageAddr || !BgmChangeAddr || !GameFlagAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    BgmChangeReturnAddr = BgmChangeAddr + 0x06;
    shBgmCall = (void(*)(int))(BgmChangeAddr + 0x2B + *(DWORD*)(BgmChangeAddr + 0x27));

    Logging::Log() << "Patching Music Box Volume...";

    WriteJMPtoMemory((BYTE*)BgmChangeAddr, *BgmChangeASM, 0x06);

    // Set custom BGM control function for Hotel 2F.
    int(*GetStageBgmBankAddr)(void) = GetStageBgmBank;
    UpdateMemoryAddress((void*)(Hotel2FStageAddr + 0x30), &GetStageBgmBankAddr, sizeof(&GetStageBgmBank));

    // Update BGM sound regions in Hotel 1F.
    const DWORD ChannelNoBanks = 0x00;
    const DWORD ChannelBank4 = 0x00100000;
    for (int i = 6; i <= 11; ++i)
    {
        // Quieter music box in 1F hallway.
        UpdateMemoryAddress((void*)(Hotel1FSoundDataAddr + i * 0x40 + 0x34), &ChannelNoBanks, sizeof(DWORD));
        UpdateMemoryAddress((void*)(Hotel1FSoundDataAddr + i * 0x40 + 0x3C), &ChannelBank4, sizeof(DWORD));
    }
    for (int i = 12; i <= 27; ++i)
    {
        // No music box in rooms other than lobby and reception.
        if (i == 19 || i == 20) continue;
        UpdateMemoryAddress((void*)(Hotel1FSoundDataAddr + i * 0x40 + 0x34), &ChannelNoBanks, sizeof(DWORD));
    }
    // No music box outside of known regions. This covers the 1F employee hallway since the sound regions
    // for this hallway are actually misplaced on top of a different room.
    UpdateMemoryAddress((void*)(Hotel1FSoundDataAddr + 0x34), &ChannelNoBanks, sizeof(DWORD));

    // Update BGM sound regions in Hotel 2F.
    const DWORD EnableAreaFlag = 0x40000010;
    for (int i = 3; i <= 9; ++i)
    {
        // Quieter music box in 2F hallways connected to the lobby.
        UpdateMemoryAddress((void*)(Hotel2FSoundDataAddr + i * 0x40 + 0x20), &EnableAreaFlag, sizeof(DWORD));
        UpdateMemoryAddress((void*)(Hotel2FSoundDataAddr + i * 0x40 + 0x3C), &ChannelBank4, sizeof(DWORD));
    }
    // Quieter music box in cloak room.
    UpdateMemoryAddress((void*)(Hotel2FSoundDataAddr + 12 * 0x40 + 0x20), &EnableAreaFlag, sizeof(DWORD));
    UpdateMemoryAddress((void*)(Hotel2FSoundDataAddr + 12 * 0x40 + 0x3C), &ChannelBank4, sizeof(DWORD));
}
