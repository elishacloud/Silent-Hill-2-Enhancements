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

constexpr float kObservationDeckMaxPosX = 24500.0f;
constexpr float kFogBorderWidth = 300.0f;
constexpr float kFogAlphaFactor = 1.0f / kFogBorderWidth;

constexpr int kFogParticleCountMax = 2000;
constexpr int kFogParticleByteSize = 0x60;
BYTE FogParticleArray[kFogParticleCountMax * kFogParticleByteSize];

// Variables for ASM
float *FogMaxPosPtr = nullptr;
void *RandomVec4FuncAddr = nullptr;
void *jmpSetFogParticlePosReturnAddr = nullptr;
void *jmpClampFogParticleXPosReturnAddr = nullptr;
void *jmpFadeFogParticleXPosReturnAddr = nullptr;
void *jmpClearFogWorkReturnAddr = nullptr;

// ASM function that returns the maximum X position of the fog area for the current room.
__declspec(naked) void __stdcall GetFogMaxPosXPointer()
{
    __asm
    {
        mov eax, dword ptr ds : [RoomIDAddr]
        mov eax, dword ptr ds : [eax]
        cmp eax, R_OBSV_DECK
        lea eax, kObservationDeckMaxPosX
        je ExitASM
        mov eax, dword ptr ds : [FogMaxPosPtr]

    ExitASM:
        ret
    }
}

// ASM function that spawns particles in the fog area with an optional extended [-X, X] limit.
__declspec(naked) void __stdcall SetFogParticlePosASM()
{
    __asm
    {
        push eax
        call GetFogMaxPosXPointer
        mov eax, dword ptr ds : [eax]
        push eax
        push esi
        call RandomVec4FuncAddr
        add esp, 0x08
        lea ecx, dword ptr ds : [esi + 0x04]
        push ecx
        call RandomVec4FuncAddr
        jmp jmpSetFogParticlePosReturnAddr
    }
}

// ASM function that wraps particles around the boundary of the fog area with an optional extended
// [-X, X] limit.
__declspec(naked) void __stdcall ClampFogParticleXPosASM()
{
    __asm
    {
        call GetFogMaxPosXPointer
        mov eax, dword ptr ds : [eax]
        push eax
        fld dword ptr ds : [esp]
        fchs
        fld st(3)
        fcomp
        fnstsw ax
        test ah, 0x05
        jp AboveLowerBound

        fld dword ptr ds : [esp]
        fadd st(0), st(0)
        faddp st(4), st(0)
        jmp FlipZPos

    AboveLowerBound:
        fld st(3)
        fcomp dword ptr ds : [esp]
        fnstsw ax
        test ah, 0x41
        jnz ExitASM

        fld dword ptr ds : [esp]
        fadd st(0), st(0)
        fsubp st(4), st(0)

    FlipZPos:
        fxch
        fchs
        fxch

    ExitASM:
        fstp st(0)
        pop eax
        fld dword ptr ds : [esp]
        fchs
        jmp jmpClampFogParticleXPosReturnAddr
    }
}

// ASM function that fades particles at the boundary of the fog area with an optional extended
// [-X, X] limit.
__declspec(naked) void __stdcall FadeFogParticleXPosASM()
{
    __asm
    {
        call GetFogMaxPosXPointer
        mov eax, dword ptr ds : [eax]
        push eax
        fld dword ptr ds : [kFogBorderWidth]
        fsub dword ptr ds : [esp]
        fld st(4)
        fcomp
        fld dword ptr ds : [esp]
        fnstsw ax
        test ah, 0x05
        jp AboveLowerBoundBorder

        fadd st(0), st(5)
        jmp SetParticleAlphaNow

    AboveLowerBoundBorder:
        fsub dword ptr ds : [kFogBorderWidth]
        fld st(5)
        fcompp
        fnstsw ax
        test ah, 0x41
        jnz ExitASM

        fld dword ptr ds : [esp]
        fsub st(0), st(5)

    SetParticleAlphaNow:
        fmulp st(2), st(0)
        fxch
        fmul dword ptr ds : [kFogAlphaFactor]
        fxch

    ExitASM:
        fstp st(0)
        pop eax
        fld dword ptr ds : [kFogBorderWidth]
        fsub dword ptr ds : [esp]
        jmp jmpFadeFogParticleXPosReturnAddr
    }
}

void ClearFogParticleArray()
{
    memset(FogParticleArray, 0, sizeof(FogParticleArray));
}

// ASM function to reset fog work data.
__declspec(naked) void __stdcall ClearFogWorkASM()
{
    __asm
    {
        call ClearFogParticleArray
        push 0x11390
        jmp jmpClearFogWorkReturnAddr
    }
}

// Replaces operands pointing to the static fog particle array to our own array storage.
bool RelocateFogParticleArray()
{
    DWORD DstAddr[7] = { 0 };

    constexpr BYTE SearchBytes1[]{ 0x55, 0x56, 0x57, 0x8D, 0x3C, 0x40 };
    DstAddr[0] = SearchAndGetAddresses(0x0048759F, 0x0048783F, 0x00487A4F, SearchBytes1, sizeof(SearchBytes1), 0x0D, __FUNCTION__);

    constexpr BYTE SearchBytes2[]{ 0x83, 0xC4, 0x08, 0x46, 0x83, 0xC7, 0x60, 0x3B, 0xF2 };
    DstAddr[1] = SearchAndGetAddresses(0x004881F3, 0x00488493, 0x004886A3, SearchBytes2, sizeof(SearchBytes2), -0x16, __FUNCTION__);

    constexpr BYTE SearchBytes3[]{ 0x83, 0xC6, 0x60, 0x4F, 0x75, 0xF5 };
    DstAddr[2] = SearchAndGetAddresses(0x00488267, 0x00488507, 0x00488717, SearchBytes3, sizeof(SearchBytes3), -0x0B, __FUNCTION__);

    constexpr BYTE SearchBytes4[]{ 0xB9, 0x10, 0x00, 0x00, 0x00, 0x8B, 0xF0, 0x8D, 0xBC, 0x24, 0x10, 0x01, 0x00, 0x00 };
    DstAddr[3] = SearchAndGetAddresses(0x004888C6, 0x00488B66, 0x00488D76, SearchBytes4, sizeof(SearchBytes4), 0x1B, __FUNCTION__);

    constexpr BYTE SearchBytes5[]{ 0x55, 0x56, 0x57, 0xB9, 0x00, 0x02 };
    DstAddr[4] = SearchAndGetAddresses(0x004891A3, 0x00489443, 0x00489653, SearchBytes5, sizeof(SearchBytes5), -0x5B, __FUNCTION__);

    DstAddr[5] = DstAddr[4] + 0x0C;
    DstAddr[6] = DstAddr[4] + 0xA0;
    DstAddr[7] = DstAddr[4] + 0x3B8;

    for (const DWORD addr : DstAddr)
    {
        if (!addr)
        {
            Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
            return false;
        }
    }

    DWORD NewAddr = (DWORD)FogParticleArray;
    UpdateMemoryAddress((void*)DstAddr[2], &NewAddr, sizeof(DWORD));
    UpdateMemoryAddress((void*)DstAddr[4], &NewAddr, sizeof(DWORD));
    UpdateMemoryAddress((void*)DstAddr[7], &NewAddr, sizeof(DWORD));

    NewAddr = (DWORD)FogParticleArray + 0x08;
    UpdateMemoryAddress((void*)DstAddr[5], &NewAddr, sizeof(DWORD));

    NewAddr = (DWORD)FogParticleArray + 0x10;
    UpdateMemoryAddress((void*)DstAddr[3], &NewAddr, sizeof(DWORD));

    NewAddr = (DWORD)FogParticleArray + 0x18;
    UpdateMemoryAddress((void*)DstAddr[6], &NewAddr, sizeof(DWORD));

    NewAddr = (DWORD)FogParticleArray + 0x20;
    UpdateMemoryAddress((void*)DstAddr[1], &NewAddr, sizeof(DWORD));

    NewAddr = (DWORD)FogParticleArray + 0x50;
    UpdateMemoryAddress((void*)DstAddr[0], &NewAddr, sizeof(DWORD));

    return true;
}

// Increases the maximum number of fog particles from 500 to kFogParticleCountMax.
bool SetMaxFogParticleCount()
{
    constexpr BYTE SearchBytes[]{ 0x8B, 0xD8, 0x81, 0xFB, 0xF4, 0x01, 0x00, 0x00 };
    const DWORD ParticleCountMaxAddr = SearchAndGetAddresses(0x0048756D, 0x0048780D, 0x00487A1D, SearchBytes, sizeof(SearchBytes), 0x04, __FUNCTION__);

    if (!ParticleCountMaxAddr)
    {
        Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
        return false;
    }

    UpdateMemoryAddress((void*)(ParticleCountMaxAddr), &kFogParticleCountMax, sizeof(DWORD));
    UpdateMemoryAddress((void*)(ParticleCountMaxAddr + 0x18), &kFogParticleCountMax, sizeof(DWORD));
    UpdateMemoryAddress((void*)(ParticleCountMaxAddr + 0x1F), &kFogParticleCountMax, sizeof(DWORD));
    return true;
}

// Widens the spawn range of fog particles behind the Toluca Lake observation deck.
void PatchObservationDeckFogFix()
{
    constexpr BYTE FogMaxPosSearchBytes[]{ 0x89, 0x44, 0x24, 0x00, 0xD9, 0x42, 0x04 };
    FogMaxPosPtr = (float*)ReadSearchedAddresses(0x00486D20, 0x00486FC0, 0x004871D0, FogMaxPosSearchBytes, sizeof(FogMaxPosSearchBytes), -0x0A, __FUNCTION__);

    constexpr BYTE SetFogParticlePosSearchBytes[]{ 0x8D, 0x4E, 0x10, 0x68, 0x00, 0x00, 0x80, 0x3F };
    const DWORD SetFogParticlePosAddr = SearchAndGetAddresses(0x004858AE, 0x00485B4E, 0x00485D5E, SetFogParticlePosSearchBytes, sizeof(SetFogParticlePosSearchBytes), -0x07, __FUNCTION__);

    constexpr BYTE ClampFogXPosSearchBytes[]{ 0xD9, 0x44, 0x24, 0x00, 0xD9, 0xE0, 0xD9, 0xC3 };
    const DWORD ClampFogXPosAddr = SearchAndGetAddresses(0x00486D36, 0x00486FD6, 0x004871E6, ClampFogXPosSearchBytes, sizeof(ClampFogXPosSearchBytes), 0x00, __FUNCTION__);

    constexpr BYTE ClearFogWorkSearchBytes[]{ 0x68, 0x90, 0x13, 0x01, 0x00 };
    const DWORD ClearFogWorkAddr = SearchAndGetAddresses(0x00489640, 0x004898E0, 0x00489AF0, ClearFogWorkSearchBytes, sizeof(ClearFogWorkSearchBytes), 0x00, __FUNCTION__);

    if (!FogMaxPosPtr || !SetFogParticlePosAddr || !ClampFogXPosAddr || !ClearFogWorkAddr)
    {
        Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
        return;
    }
    const DWORD FadeFogXBorderAddr = ClampFogXPosAddr + 0x151;
    const DWORD RandomVec4RelativeFuncAddr = *(int*)(SetFogParticlePosAddr + 0x03);
    RandomVec4FuncAddr = (void*)(RandomVec4RelativeFuncAddr + SetFogParticlePosAddr + 0x07);

    jmpSetFogParticlePosReturnAddr = (void*)(SetFogParticlePosAddr + 0x07);
    jmpClampFogParticleXPosReturnAddr = (void*)(ClampFogXPosAddr + 0x36);
    jmpFadeFogParticleXPosReturnAddr = (void*)(FadeFogXBorderAddr + 0x40);
    jmpClearFogWorkReturnAddr = (void*)(ClearFogWorkAddr + 0x05);

    Logging::Log() << "Enabling Observation Deck Fog Fix...";
    if (!RelocateFogParticleArray() || !SetMaxFogParticleCount())
        return;

    WriteJMPtoMemory((BYTE*)SetFogParticlePosAddr, *SetFogParticlePosASM, 7);
    WriteJMPtoMemory((BYTE*)ClampFogXPosAddr, *ClampFogParticleXPosASM, 6);
    WriteJMPtoMemory((BYTE*)FadeFogXBorderAddr, *FadeFogParticleXPosASM, 6);
    WriteJMPtoMemory((BYTE*)ClearFogWorkAddr, *ClearFogWorkASM, 5);
}
