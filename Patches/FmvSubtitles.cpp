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
DWORD InitSubtitlesFuncAddr = 0;
DWORD DrawSubtitlesFuncAddr = 0;
DWORD SaveEaxAddr = 0;

__declspec(naked) void __stdcall DrawSubtitlesASM()
{
    __asm
    {
        mov eax, dword ptr ds : [esp + 0x1C]
        mov esi, dword ptr ds : [esp + 0x18]
        push edx
        mov edx, dword ptr ds : [InitSubtitlesFuncAddr]
        call edx
        mov edx, dword ptr ds : [DrawSubtitlesFuncAddr]
        call edx
        pop edx
        mov eax, dword ptr ds : [SaveEaxAddr]
        mov eax, dword ptr ds : [eax]
        ret
    }
}

// Patch subtitles during FMVs to draw on top of noise grain
void PatchFmvSubtitles()
{
    constexpr BYTE InjectSubtitlesSearchBytes[]{ 0x55, 0x55, 0x6A, 0x73, 0xE8 };
    const DWORD InjectSubtitlesAddr = SearchAndGetAddresses(0x0043E496, 0x0043E656, 0x0043E656, InjectSubtitlesSearchBytes, sizeof(InjectSubtitlesSearchBytes), 0x0C);
    if (!InjectSubtitlesAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    const DWORD SkipSubtitlesAddr = InjectSubtitlesAddr - 0x7F;
    const DWORD SkipSubtitlesJumpAddr = SkipSubtitlesAddr + 0x12;

    DWORD InitSubtitlesRelativeAddr = 0;
    memcpy(&InitSubtitlesRelativeAddr, (void*)(SkipSubtitlesAddr + 0x09), sizeof(DWORD));
    InitSubtitlesFuncAddr = InitSubtitlesRelativeAddr + SkipSubtitlesAddr + 0x0D;

    DWORD DrawSubtitlesRelativeAddr = 0;
    memcpy(&DrawSubtitlesRelativeAddr, (void*)(SkipSubtitlesAddr + 0x0E), sizeof(DWORD));
    DrawSubtitlesFuncAddr = DrawSubtitlesRelativeAddr + SkipSubtitlesAddr + 0x12;

    memcpy(&SaveEaxAddr, (void*)(InjectSubtitlesAddr + 0x01), sizeof(DWORD));

    Logging::Log() << "Patching FMV subtitles to draw over noise grain...";

    // Skip original function calls that draw subtitles under noise grain.
    WriteJMPtoMemory((BYTE*)SkipSubtitlesAddr, (BYTE*)SkipSubtitlesJumpAddr, 0x08);

    // Add new function calls to draw subtitles over noise grain.
    WriteCalltoMemory((BYTE*)InjectSubtitlesAddr, *DrawSubtitlesASM, 0x05);
}
