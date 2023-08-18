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
DWORD DrawDistanceFactorAddr = 0;  // (Size of depth sort list - 1) / (DrawDistance + 1000.0)

__declspec(naked) void __stdcall FixMothSortIndexASM()
{
    __asm
    {
        push eax
        mov eax, dword ptr ds : [DrawDistanceFactorAddr]
        // ST[0] = distance of the moth object from the camera.
        // Get the correct insertion index for depth sorting.
        fmul dword ptr ds : [eax]
        pop eax
        mov dword ptr ds : [esp + 0x64], 0x3F800000
        ret
    }
}

// Patch draw order of moths to render correctly with other transparent geometry in the scene.
void PatchMothDrawOrder()
{
    constexpr BYTE DrawDistanceFactorSearchBytes[]{ 0x85, 0xC0, 0xB9, 0x2C, 0x00, 0x00, 0x00, 0x74, 0x1B };
    DrawDistanceFactorAddr = ReadSearchedAddresses(0x00476AC5, 0x00476D65, 0x00476F75, DrawDistanceFactorSearchBytes, sizeof(DrawDistanceFactorSearchBytes), 0x0A, __FUNCTION__);
    if (!DrawDistanceFactorAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    DrawDistanceFactorAddr += 0x08;

    constexpr BYTE FixMothSortIndexSearchBytes[]{ 0xF3, 0xA5, 0xC6, 0x85, 0xA4, 0x00, 0x00, 0x00, 0x02 };
    const DWORD FixMothSortIndexAddrA = SearchAndGetAddresses(0x004A4D9F, 0x004A504F, 0x004A490F, FixMothSortIndexSearchBytes, sizeof(FixMothSortIndexSearchBytes), 0x43, __FUNCTION__);
    if (!FixMothSortIndexAddrA)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    const DWORD FixMothSortIndexAddrB = FixMothSortIndexAddrA + 0x262;

    Logging::Log() << "Patching moth object draw order...";
    WriteCalltoMemory((BYTE*)FixMothSortIndexAddrA, *FixMothSortIndexASM, 0x08);
    WriteCalltoMemory((BYTE*)FixMothSortIndexAddrB, *FixMothSortIndexASM, 0x08);
}
