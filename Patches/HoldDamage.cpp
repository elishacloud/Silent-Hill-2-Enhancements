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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace
{
    // Variables for ASM
    DWORD* DeltaTimeFuncAddr = 0;
    float DamageScale = 30.0f;
}

// ASM which scales enemy damage by delta time while the player is in a hold attack.
__declspec(naked) void __stdcall ScaleHoldDamageASM()
{
    __asm
    {
        push eax
        mov eax, dword ptr ds : [DeltaTimeFuncAddr]
        call eax  // Pushes delta time to st(0)
        fmul dword ptr ds : [DamageScale]
        fmul dword ptr ds : [esi + 0x11C]  // Multiply by enemy damage
        fsubp st(1), st(0)
        pop eax
        ret
    }
}

// Patch how much damage an enemy hold attack will inflict every frame by scaling damage with the current frame rate.
// Affects hold damage rate for Flesh Lips, Abstract Daddy, and the final boss (tentacle choke and moth attack).
void PatchHoldDamage()
{
    constexpr BYTE SearchBytesApplyHoldDamage[]{ 0xD9, 0x86, 0x3C, 0x01, 0x00, 0x00, 0x8B, 0x96, 0x1C, 0x01, 0x00, 0x00 };
    DWORD ApplyHoldDamageAddr = SearchAndGetAddresses(0x005359D5, 0x00535D05, 0x00535625, SearchBytesApplyHoldDamage, sizeof(SearchBytesApplyHoldDamage), 0x0C, __FUNCTION__);
    if (!ApplyHoldDamageAddr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }

    DeltaTimeFuncAddr = GetDeltaTimeFunctionPointer();
    if (!DeltaTimeFuncAddr)
    {
        return;
    }

    Logging::Log() << "Enabling Enemy Hold Damage Fix...";
    WriteCalltoMemory((BYTE*)ApplyHoldDamageAddr, *ScaleHoldDamageASM, 0x06);
}
