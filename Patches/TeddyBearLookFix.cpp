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

constexpr int kBentNeedleGameFlag = 0xAA;

// Variables for ASM
DWORD LookAtReturnAddr1 = 0;
DWORD LookAtReturnAddr2 = 0;
BYTE* GameFlagAddr = 0;

bool IsBentNeedleGameFlagSet()
{
    return GameFlagAddr[kBentNeedleGameFlag >> 3] & (1 << (kBentNeedleGameFlag & 0x07));
}

// ASM function that ignores the teddy bear as a game object that James can look at after
// watching the cutscene.
__declspec(naked) void __stdcall CheckTeddyBearASM()
{
    __asm
    {
        mov cx, word ptr ds : [esi + 0x10]
        cmp cx, 0x0727  // kind == i_bear?
        jne ExitAsm
        push eax
        call IsBentNeedleGameFlagSet
        test al, al
        pop eax
        jz ExitAsm
        jmp LookAtReturnAddr2

    ExitAsm:
        movsx eax, cx
        jmp LookAtReturnAddr1
    }
}

// Prevents James from looking at the teddy bear in the women's locker room on the second floor of
// Brookhaven Hospital after James acquires the bent needle.
void PatchTeddyBearLookFix()
{
    constexpr BYTE LookAtSearchBytes[]{ 0x66, 0x8B, 0x4E, 0x10, 0x0F, 0xBF, 0xC1 };
    DWORD LookAtInjectAddr = SearchAndGetAddresses(0x00542769, 0x00542A99, 0x005423B9, LookAtSearchBytes, sizeof(LookAtSearchBytes), 0x00, __FUNCTION__);
    if (!LookAtInjectAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }
    LookAtReturnAddr1 = LookAtInjectAddr + 0x07;
    LookAtReturnAddr2 = LookAtInjectAddr + 0xA9;

    constexpr BYTE GameFlagSearchBytes[]{ 0x83, 0xFE, 0x01, 0x55, 0x57, 0xBD, 0x00, 0x01, 0x00, 0x00 };
    GameFlagAddr = (BYTE*)ReadSearchedAddresses(0x0048AA9E, 0x0048AD3E, 0x0048AF4E, GameFlagSearchBytes, sizeof(LookAtSearchBytes), 0x24, __FUNCTION__);
    if (!GameFlagAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    Logging::Log() << "Patching Teddy Bear Look Fix...";
    WriteJMPtoMemory((BYTE*)LookAtInjectAddr, *CheckTeddyBearASM, 0x07);
}
