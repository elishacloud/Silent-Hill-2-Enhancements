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

DWORD PlayerLastHitResult = 0;
DWORD jmpUpdateLastHitResultReturnAddr = 0;
DWORD jmpPlayChainsawHitSoundReturnAddr1 = 0;
DWORD jmpPlayChainsawHitSoundReturnAddr2 = 0;

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

void PatchChainsawSoundFix()
{
    const BYTE ChainsawHitSearchBytes[]{ 0xB8, 0x26, 0x2B, 0x00, 0x00, 0xE9, 0x4B };
    const DWORD ChainsawHitAddr = SearchAndGetAddresses(0x005358C3, 0x00535BF3, 0x00535513, ChainsawHitSearchBytes, sizeof(ChainsawHitSearchBytes), 0x00, __FUNCTION__);

    const BYTE SetSoundDamageSearchBytes[]{ 0xBA, 0x26, 0x2B, 0x00, 0x00, 0xEB, 0xD9 };
    const DWORD SetSoundDamageAddr = SearchAndGetAddresses(0x00543612, 0x00543942, 0x00543262, SetSoundDamageSearchBytes, sizeof(SetSoundDamageSearchBytes), 0x00, __FUNCTION__);

    if (!ChainsawHitAddr || !SetSoundDamageAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    jmpPlayChainsawHitSoundReturnAddr1 = ChainsawHitAddr - 0x97;
    jmpPlayChainsawHitSoundReturnAddr2 = ChainsawHitAddr + 0x05;
    jmpUpdateLastHitResultReturnAddr = ChainsawHitAddr - 0x8D;

    Logging::Log() << "Patching Chainsaw Sound Fixes...";

    WriteJMPtoMemory((BYTE*)ChainsawHitAddr, *PlayChainsawHitSoundASM, 0x05);
    WriteJMPtoMemory((BYTE*)(ChainsawHitAddr - 0x92), *UpdateLastHitResultASM, 0x05);

    // There are two functions that play the chainsaw hit sound every frame. Skip the second callsite.
    UpdateMemoryAddress((void*)SetSoundDamageAddr, "\x5F\x5B\x59\xC3\x90", 0x05);
}
