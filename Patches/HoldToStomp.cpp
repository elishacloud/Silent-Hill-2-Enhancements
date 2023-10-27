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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace
{
    // Maximum duration the attack button can be held before triggering a stomp attack.
    constexpr float kStompHoldTimerMax = 0.2f;

    // Variables for ASM
    DWORD* DeltaTimeFuncAddr = 0;
    float StompHoldTimer = 0.0f;
}

// ASM function which measures the duration of the attack button press to determine whether to request
// a kick or stomp attack.
__declspec(naked) void __stdcall HoldToStompASM()
{
    __asm
    {
        cmp byte ptr ds : [eax + 0x493], 0x00
        jne ExitAsm
        cmp byte ptr ds : [eax + 0x290], 0x00  // Was the attack button pressed this frame?
        je CheckStomp
        mov dword ptr ds : [StompHoldTimer], 0x00  // Reset timer

    CheckStomp:
        cmp byte ptr ds : [eax + 0x28E], 0x00  // Is attack button being held?
        je CheckKick
        push eax
        mov eax, dword ptr ds : [DeltaTimeFuncAddr]
        call eax  // Pushes delta time to st(0)
        fadd dword ptr ds : [StompHoldTimer]
        fcom dword ptr ds : [kStompHoldTimerMax]
        fstp dword ptr ds : [StompHoldTimer]
        fnstsw ax
        test ah, 0x41
        pop eax
        jnz ExitAsm
        mov cl, 0x07  // Request stomp attack
        jmp RequestAttack

    CheckKick:
        cmp dword ptr ds : [StompHoldTimer], 0x00
        je ExitAsm
        mov cl, 0x06  // Request kick attack

    RequestAttack:
        mov byte ptr ds : [eax + 0x493], cl
        mov dword ptr ds : [StompHoldTimer], 0x00  // Reset timer

    ExitAsm:
        ret
    }
}

// Patches the ability to tap the attack button to kick a fallen enemy, or hold the attack button to
// stomp the enemy.
void PatchHoldToStomp()
{
    // Get address to request stomp attack
    constexpr BYTE SearchBytesHoldToStomp[]{ 0x8B, 0x44, 0x24, 0x04, 0x8A, 0x88, 0x8E, 0x02, 0x00, 0x00 };
    DWORD HoldToStompAddr = SearchAndGetAddresses(0x005345C0, 0x005348F0, 0x00534210, SearchBytesHoldToStomp, sizeof(SearchBytesHoldToStomp), 0x04, __FUNCTION__);
    if (!HoldToStompAddr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }
    
    DeltaTimeFuncAddr = GetDeltaTimeFunctionPointer();
    if (!DeltaTimeFuncAddr)
        return;

    Logging::Log() << "Enabling Hold to Stomp...";
    WriteJMPtoMemory((BYTE*)HoldToStompAddr, *HoldToStompASM, 0x06);
}
