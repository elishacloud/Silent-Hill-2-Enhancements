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
constexpr float FmvBaseFramerate = 29.97f;  // NTSC

DWORD InitSubtitlesFuncAddr = 0;
DWORD DrawSubtitlesFuncAddr = 0;
DWORD SaveEaxAddr = 0;
DWORD BinkObjectAddr = 0;
DWORD LastFrameIndexAddr = 0;
DWORD SyncSubtitlesReturnAddr = 0;
DWORD SyncSubtitlesInWaterReturnAddr = 0;

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


// Recalculates the index of the current subtitle frame using the framerate of the active Bink video
__declspec(naked) void __stdcall SyncSubtitlesASM()
{
    __asm
    {
        mov ecx, dword ptr ds : [BinkObjectAddr]
        test ecx, ecx
        jz ExitAsm

        mov ecx, dword ptr ds : [ecx]
        fild dword ptr ds : [ecx + 0x14]
        fidiv dword ptr ds : [ecx + 0x18]  // ST(0) = Bink video FPS
        fdivr dword ptr ds : [FmvBaseFramerate]
        push esi
        fild dword ptr ds : [esp]
        fmulp st(1), st(0)
        fistp dword ptr ds : [esp]
        mov esi, dword ptr ds : [esp]  // Subtitle frame *= (29.97 FPS / Bink video FPS)
        add esp, 0x04

    ExitAsm:
        test eax, eax
        jnz ExitAsmInWater
        jmp SyncSubtitlesReturnAddr

    ExitAsmInWater:
        mov ecx, dword ptr ds : [LastFrameIndexAddr]
        cmp dword ptr ds : [ecx], esi
        jmp SyncSubtitlesInWaterReturnAddr
    }
}

// Patch subtitles during FMVs to draw on top of noise grain
void PatchFmvSubtitlesNoiseFix()
{
    constexpr BYTE InjectSubtitlesSearchBytes[]{ 0x55, 0x55, 0x6A, 0x73, 0xE8 };
    const DWORD InjectSubtitlesAddr = SearchAndGetAddresses(0x0043E496, 0x0043E656, 0x0043E656, InjectSubtitlesSearchBytes, sizeof(InjectSubtitlesSearchBytes), 0x0C, __FUNCTION__);
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

// Patch FMV subtitles to sync correctly with the video framerate
void PatchFmvSubtitlesSyncFix()
{
    constexpr BYTE InjectSubtitlesSyncSearchBytes[]{ 0x85, 0xC0, 0x74, 0x2D, 0x39, 0x35 };
    const DWORD InjectSubtitlesSyncAddr = SearchAndGetAddresses(0x0043DC50, 0x0043DE10, 0x0043DE10, InjectSubtitlesSyncSearchBytes, sizeof(InjectSubtitlesSyncSearchBytes), 0x00, __FUNCTION__);
    if (!InjectSubtitlesSyncAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }
    SyncSubtitlesReturnAddr = InjectSubtitlesSyncAddr + 0x31;
    SyncSubtitlesInWaterReturnAddr = InjectSubtitlesSyncAddr + 0x0A;
    memcpy(&LastFrameIndexAddr, (void*)(InjectSubtitlesSyncAddr + 0x06), sizeof(DWORD));

    constexpr BYTE BinkObjectSearchBytes[]{ 0x84, 0xC0, 0x0F, 0x85, 0xB6, 0xF8, 0xFF, 0xFF };
    BinkObjectAddr = ReadSearchedAddresses(0x0043E4D2, 0x0043E692, 0x0043E692, BinkObjectSearchBytes, sizeof(BinkObjectSearchBytes), 0x09, __FUNCTION__);
    if (!BinkObjectAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address for Bink object!";
        return;
    }

    Logging::Log() << "Patching FMV subtitles to sync with video framerate...";
    WriteJMPtoMemory((BYTE*)InjectSubtitlesSyncAddr, (BYTE*)SyncSubtitlesASM, 0x0A);
}
