/**
* Copyright (C) 2022 Murugo
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
DWORD FadeValueAddr = 0;
float FadeInStartValue = -0.133334f;
float Zero = 0.0f;

// ASM to initialize the fade value at the start of a fade-in
__declspec(naked) void __stdcall InitFadeASM()
{
    __asm
    {
        push eax
        push ecx
        mov eax, dword ptr ds : [FadeInStartValue]
        mov ecx, dword ptr ds : [FadeValueAddr]
        mov [ecx], eax
        pop ecx
        pop eax
        ret
    }
}

// ASM to clamp a negative fade value to zero
__declspec(naked) void __stdcall ClampFadeASM()
{
    __asm
    {
        mov eax, dword ptr ds : [FadeValueAddr]
        fld dword ptr ds : [eax]
        fsubrp st(1), st(0)
        fcom dword ptr ds : [Zero]
        fnstsw ax
        test ah, 0x41
        jp ExitAsm
        fmul dword ptr ds : [Zero]

    ExitAsm:
        ret
    }
}

// Patch fade-in transition to pause for a short duration before fading in the screen.
// This is intended to hide certain animation artifacts that occur after a room transition.
void PatchDelayedFadeIn()
{
    constexpr BYTE FadeInitValueSearchBytes[]{ 0xB8, 0x13, 0x00, 0x00, 0x00, 0xA3 };
    DWORD FadeInitValueAddr = SearchAndGetAddresses(0x00478440, 0x004786E0, 0x004788F0, FadeInitValueSearchBytes, sizeof(FadeInitValueSearchBytes), 0x2C);
    if (!FadeInitValueAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }

    constexpr BYTE FadeUpdateSearchBytes[]{ 0x3B, 0xFB, 0x0F, 0x84, 0x92, 0x00, 0x00, 0x00, 0xE8 };
    DWORD FadeUpdateAddr = SearchAndGetAddresses(0x004790DA, 0x0047937A, 0x0047958A, FadeUpdateSearchBytes, sizeof(FadeUpdateSearchBytes), 0x0D);
    if (!FadeUpdateAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    memcpy(&FadeValueAddr, (void*)(FadeUpdateAddr + 0x02), sizeof(DWORD));

    Logging::Log() << "Patching Delayed Fade-in...";
    WriteCalltoMemory((BYTE*)FadeInitValueAddr, *InitFadeASM, 0x0A);
    WriteCalltoMemory((BYTE*)FadeUpdateAddr, *ClampFadeASM, 0x06);
}
