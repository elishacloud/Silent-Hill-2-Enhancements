/**
* Copyright (C) 2024 Murugo
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
DWORD PlayerRootAddr = 0;
DWORD ProgramFlagAddr = 0;
void *jmpShowerRoomCutsceneReturnAddr = nullptr;

// Determines which cutscene active flag to use to locate the flashlight position. If the current
// room is R_HSP_SHOWER, uses the player object's status flag, which remains active on the freeze
// frame when James acquires the elevator key. At this point, the program flag for an active
// cutscene has already been reset to zero.
__declspec(naked) void __stdcall CheckShowerRoomCutsceneASM()
{
    __asm
    {
        mov eax, dword ptr ds : [RoomIDAddr]
        mov eax, dword ptr ds : [eax]
        cmp eax, R_HSP_SHOWER
        jne ExitASM
        mov eax, dword ptr ds : [PlayerRootAddr]
        mov eax, dword ptr ds : [eax]
        test eax, eax
        jz ExitAsm
        test word ptr ds : [eax + 0x04], 0x2000
        jmp jmpShowerRoomCutsceneReturnAddr

    ExitASM:
        mov eax, dword ptr ds : [ProgramFlagAddr]
        test byte ptr ds : [eax], 0x40
        jmp jmpShowerRoomCutsceneReturnAddr
    }
}

// During the drain cutscene in the Brookhaven Hospital shower room, prevents the spot light from
// James's flashlight from suddently shifting on the last frame of the cutscene.
void PatchShowerRoomFlashlightFix()
{
    constexpr BYTE PlayerRootSearchBytes[]{ 0x33, 0xC0, 0x3B, 0xCE, 0x89, 0x44, 0x24, 0x1C };
    PlayerRootAddr = ReadSearchedAddresses(0x00402D2A, 0x00402D2A, 0x00402D2A, PlayerRootSearchBytes, sizeof(PlayerRootSearchBytes), -0x04, __FUNCTION__);

    constexpr BYTE CheckShowerRoomCutsceneSearchBytes[]{ 0x85, 0xC0, 0x74, 0x27, 0xF6, 0x05 };
    const DWORD CheckShowerRoomCutsceneAddr = SearchAndGetAddresses(0x0047AC55, 0x0047AEF5, 0x0047B105, CheckShowerRoomCutsceneSearchBytes, sizeof(CheckShowerRoomCutsceneSearchBytes), 0x04, __FUNCTION__);

    if (!PlayerRootAddr || !CheckShowerRoomCutsceneAddr)
    {
        Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
        return;
    }
    ProgramFlagAddr = *(DWORD*)(CheckShowerRoomCutsceneAddr + 0x02);
    jmpShowerRoomCutsceneReturnAddr = (void*)(CheckShowerRoomCutsceneAddr + 0x07);

    Logging::Log() << "Enabling Shower Room Flashlight Fix...";
    WriteJMPtoMemory((BYTE*)CheckShowerRoomCutsceneAddr, *CheckShowerRoomCutsceneASM, 7);
}
