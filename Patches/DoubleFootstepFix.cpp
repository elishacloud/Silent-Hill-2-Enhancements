/**
* Copyright (C) 2023 Murugo
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
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Variables for ASM
DWORD FootstepFlagAddr = 0;
DWORD JamesFootstepReturnAddr = 0;
DWORD MariaFootstepReturnAddr = 0;

__declspec(naked) void __stdcall SkipDoubleFootstepASM()
{
    __asm
    {
        test esi, esi
        jnz NoDoubleFootstep  // Not footstep 0.
        mov al, byte ptr ds : [esp + 0x04]
        test al, al
        jl NoDoubleFootstep  // Footstep 1 frame number is not set.
        movsx dx, al
        cmp cx, dx
        jl NoDoubleFootstep  // Footstep 1 will not be triggered.
        // First two footstep flags will trigger this frame, causing a double footstep.
        mov eax, [FootstepFlagAddr]
        inc byte ptr ds : [eax]  // Set the footstep 0 flag and don't play the sound. If the double
                                 // footstep was caused by a large frameskip, the handler will play
                                 // the second footstep sound on the next frame.
        mov eax, 1
        ret

    NoDoubleFootstep:
        xor eax, eax
        ret
    }
}

__declspec(naked) void __stdcall JamesFootstepASM()
{
    __asm
    {
        mov al, byte ptr ds : [esp + 0x18]
        push eax
        call SkipDoubleFootstepASM
        add esp, 0x04
        test eax, eax
        jz ExitAsm
        pop esi
        pop ebp
        pop ebx
        add esp, 0x01B0
        ret
    ExitASM:
        movsx eax, byte ptr ds : [esp + esi * 0x08 + 0x11]
        jmp JamesFootstepReturnAddr
    }
}

__declspec(naked) void __stdcall MariaFootstepASM()
{
    __asm
    {
        mov al, byte ptr ds : [esp + 0x10]
        push eax
        call SkipDoubleFootstepASM
        add esp, 0x04
        test eax, eax
        jz ExitAsm
        pop esi
        add esp, 0x012C
        ret
    ExitASM:
        movsx eax, byte ptr ds : [esp + esi * 0x08 + 0x09]
        jmp MariaFootstepReturnAddr
    }
}

// Patch footstep handlers for player characters (James and BFaW Maria) to avoid playing a "double" footstep.
// The extra footstep sound can occur when the player enters a different walk/run animation at the same time
// the previous animation loops to the starting frame.
void PatchDoubleFootstepFix()
{
    constexpr BYTE JamesFootstepSearchBytes[]{ 0x0F, 0xBE, 0x44, 0xF4, 0x11, 0x48, 0x74, 0x5D };
    DWORD JamesFootstepInjectAddr = SearchAndGetAddresses(0x0054B330, 0x0054B660, 0x0054AF80, JamesFootstepSearchBytes, sizeof(JamesFootstepSearchBytes), 0x00, __FUNCTION__);
    if (!JamesFootstepInjectAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    constexpr BYTE MariaFootstepSearchBytes[]{ 0x0F, 0xBE, 0x44, 0xF4, 0x09, 0x48, 0x74, 0x5A };
    DWORD MariaFootstepInjectAddr = SearchAndGetAddresses(0x00549715, 0x00549A45, 0x00549365, MariaFootstepSearchBytes, sizeof(MariaFootstepSearchBytes), 0x00, __FUNCTION__);
    if (!MariaFootstepInjectAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    FootstepFlagAddr = *(DWORD*)(JamesFootstepInjectAddr - 0x0C);
    JamesFootstepReturnAddr = JamesFootstepInjectAddr + 0x05;
    MariaFootstepReturnAddr = MariaFootstepInjectAddr + 0x05;

    Logging::Log() << "Patching Double Footstep Fix...";
    WriteJMPtoMemory((BYTE*)JamesFootstepInjectAddr, *JamesFootstepASM, 0x05);
    WriteJMPtoMemory((BYTE*)MariaFootstepInjectAddr, *MariaFootstepASM, 0x05);
}
