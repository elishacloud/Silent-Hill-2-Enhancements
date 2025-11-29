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

// Variables for ASM
DWORD *FlashlightAcquiredAddr = nullptr;
DWORD *SubCharStartAddr = nullptr;
DWORD *MannequinFlagsAddr = nullptr;
DWORD *jmpFixMannequinStateReturnAddr = 0;

// Scans the game object linked list for the mannequin in room 205 and correct its first state if it
// has been mistakenly marked "dead" before the player picks up the flashlight. This code will only
// be invoked when entering room 205 (0x17).
__declspec(naked) void __stdcall FixMannequinStateASM()
{
    __asm
    {
        mov eax, dword ptr ds : [FlashlightAcquiredAddr]
        test dword ptr ds : [eax], 0x40000
        jnz ExitASM
        mov eax, dword ptr ds : [SubCharStartAddr]

        LoopStart:
        mov dx, word ptr ds : [eax + 0x10]
        cmp dx, 0x0201  // Kind == Mannequin?
        jnz LoopNext

        mov edx, dword ptr ds : [eax + 0xEC]  // edx := Enemy first status
        cmp edx, 0x05  // Status == Dead?
        jnz LoopNext
        mov dword ptr [eax + 0xEC], 0x09  // Status := Dormant (on ground)

        LoopNext:
        mov eax, dword ptr [eax + 0x194]
        test eax, eax
        jnz LoopStart

        pop esi
        ret

    ExitASM:
        mov eax, dword ptr ds : [MannequinFlagsAddr]
        test dword ptr ds : [eax], 0x02000000
        jmp jmpFixMannequinStateReturnAddr
    }
}

// Fixes the state of the mannequin in Woodside Apartments room 205 before picking up the flashlight.
void PatchWoodsideMannequinState()
{
    constexpr BYTE FlashlightAcquiredSearchBytes[]{ 0x8D, 0x50, 0x1C, 0x8B, 0x0A, 0x89, 0x0D };
    FlashlightAcquiredAddr = (DWORD*)ReadSearchedAddresses(0x0045507D, 0x004552DD, 0x004552DD, FlashlightAcquiredSearchBytes, sizeof(FlashlightAcquiredSearchBytes), 0x56, __FUNCTION__);

    constexpr BYTE SubCharStartSearchBytes[]{ 0x57, 0x33, 0xC0, 0xB9, 0x64, 0x16, 0x00, 0x00 };
    SubCharStartAddr = (DWORD*)ReadSearchedAddresses(0x00538050, 0x00538380, 0x00537CA0, SubCharStartSearchBytes, sizeof(SubCharStartSearchBytes), 0x09, __FUNCTION__);

    constexpr BYTE MannequinFlagsSearchBytes[]{ 0x8D, 0x54, 0x24, 0x2C, 0x5B, 0x52 };
    MannequinFlagsAddr = (DWORD*)ReadSearchedAddresses(0x0059BC29, 0x0059C4D9, 0x000059BDF9, MannequinFlagsSearchBytes, sizeof(MannequinFlagsSearchBytes), 0x08, __FUNCTION__);

    constexpr BYTE FixMannequinStateSearchBytes[]{ 0xBE, 0x20, 0x00, 0x00, 0x00, 0xB9, 0x00, 0x80 };
    const DWORD FixMannequinStateAddr = SearchAndGetAddresses(0x0059BA35, 0x0059C2E5, 0x0059BC05, FixMannequinStateSearchBytes, sizeof(FixMannequinStateSearchBytes), -0x11, __FUNCTION__);

    if (!FlashlightAcquiredAddr || !SubCharStartAddr || !MannequinFlagsAddr || !FixMannequinStateAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    jmpFixMannequinStateReturnAddr = (DWORD*)(FixMannequinStateAddr + 0x0A);

    Logging::Log() << "Patching Woodside Room 205 Mannequin State Fix...";
    WriteJMPtoMemory((BYTE*)FixMannequinStateAddr, *FixMannequinStateASM, 0x0A);
}
