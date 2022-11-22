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

// ASM function to modify the prisoner timer after reset
__declspec(naked) void __stdcall PrisonerTimerResetASM()
{
    __asm
    {
        sal dword ptr[esi + 0xA0], 1  // Multiply prisoner timer by 2
        ret
    }
}

// Modify the value of the "Ritual" prisoner timer to fix the footstep rate
void PatchPrisonerTimer()
{
    // Get Prisoner Timer Reset address
    constexpr BYTE SearchBytesPrisonerTimerReset[]{ 0x89, 0x86, 0xA0, 0x00, 0x00, 0x00, 0xD9, 0x1C, 0x24, 0xE8 };
    DWORD PrisonerTimerResetAddr = SearchAndGetAddresses(0x004AE5F8, 0x004AE8A8, 0x004AE168, SearchBytesPrisonerTimerReset, sizeof(SearchBytesPrisonerTimerReset), 0x1D);
    if (!PrisonerTimerResetAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    Logging::Log() << "Enabling Prisoner Timer Fix...";
    WriteJMPtoMemory((BYTE*)PrisonerTimerResetAddr, *PrisonerTimerResetASM, 0x5);
}
