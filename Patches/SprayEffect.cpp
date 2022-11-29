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

constexpr float LyingFigureSpraySpeedFactor0 = 0.1f;    // 30 FPS => 0.2f,   60 FPS => 0.1f
constexpr float LyingFigureSpraySpeedFactor1 = 0.125f;  // 30 FPS => 0.25f,  60 FPS => 0.125f 
constexpr float LyingFigureSpraySpeedFactor2 = 0.15f;   // 30 FPS => 0.3f,   60 FPS => 0.15f
constexpr float HyperSpraySpeedFactor0_1 = 0.15f;       // 30 FPS => 0.3f,   60 FPS => 0.15f
constexpr float HyperSpraySpeedFactor2 = 0.25f;         // 30 FPS => 0.5f,   60 FPS => 0.25f
constexpr float SprayYAdjust = 50.0f;                   // 30 FPS => 100.0f, 60 FPS => 50.0f

DWORD* LyingFigureSprayAllocReturnAddr = nullptr;
DWORD* LyingFigureSprayHandlerAddr = nullptr;
DWORD* HyperSprayAllocReturnAddr = nullptr;
DWORD* HyperSprayHandlerAddr = nullptr;

DWORD* ParticleTableAddr = nullptr;
DWORD* FrameCounterAddr = nullptr;

DWORD SprayFrameCounter = 0;
DWORD IsSprayActive = 0;

// ASM function to spawn a spray effect particle every other frame
__declspec(naked) void __stdcall IsSprayAllocActiveASM()
{
    __asm
    {
        mov eax, 5
        mov edx, dword ptr ds : [ParticleTableAddr]
        mov eax, dword ptr ds : [eax * 4 + edx]
        mov edx, dword ptr ds : [eax + 4]
        cmp eax, edx  // At least 1 particle allocated?
        jnz CheckFrameCount
        mov dword ptr ds : [IsSprayActive], 1
        jmp ExitASM

    CheckFrameCount:
        mov eax, dword ptr ds : [FrameCounterAddr]
        mov eax, dword ptr ds : [eax]
        mov edx, dword ptr ds : [SprayFrameCounter]
        cmp eax, edx  // Is this a new frame?
        jz LoadActive
        mov dword ptr ds : [SprayFrameCounter], eax
        mov eax, dword ptr ds : [IsSprayActive]
        xor eax, 1
        mov dword ptr ds : [IsSprayActive], eax
        jmp CheckActive

    LoadActive:
        mov eax, dword ptr ds : [IsSprayActive]
        test eax, eax
    CheckActive:
        jnz ExitASM
        mov eax, 0  // Inactive
        ret

    ExitASM:
        mov eax, 1  // Active
        ret
    }
}

__declspec(naked) void __stdcall LyingFigureSprayAllocASM()
{
    __asm
    {
        call IsSprayAllocActiveASM
        test eax, eax
        jnz ExitASM
        ret  // Skip allocating a new particle this frame

    ExitASM:
        push esi
        push 5
        mov eax, dword ptr ds : [LyingFigureSprayHandlerAddr]
        push eax
        jmp LyingFigureSprayAllocReturnAddr
    }
}

__declspec(naked) void __stdcall HyperSprayAllocASM()
{
    __asm
    {
        call IsSprayAllocActiveASM
        test eax, eax
        jnz ExitASM
        ret  // Skip allocating a new particle this frame

    ExitASM:
        push esi
        push 5
        mov eax, dword ptr ds : [HyperSprayHandlerAddr]
        push eax
        jmp HyperSprayAllocReturnAddr
    }
}

bool WritePointerToMemory(void* dataAddr, const void* dataPtr) {
    return UpdateMemoryAddress(dataAddr, &dataPtr, sizeof(DWORD*));
}

DWORD* GetParticleTableAddr()
{
    constexpr BYTE ParticleTableAddrSearchBytes[]{ 0x0F, 0xB6, 0x44, 0x24, 0x08, 0x8B, 0x0C, 0x85 };
    DWORD* Addr = (DWORD*)ReadSearchedAddresses(0x00570B80, 0x00571430, 0x00570D50, ParticleTableAddrSearchBytes, sizeof(ParticleTableAddrSearchBytes), 0x08);
    if (Addr == nullptr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for the particle table!";
        return nullptr;
    }
    return Addr;
}

DWORD* GetFrameCounterAddr()
{
    constexpr BYTE FrameCounterAddrSearchBytes[]{ 0x8B, 0xC8, 0x2B, 0xC2, 0x89, 0x0D };
    DWORD* Addr = (DWORD*)ReadSearchedAddresses(0x0044932B, 0x004494CB, 0x004494CB, FrameCounterAddrSearchBytes, sizeof(FrameCounterAddrSearchBytes), 0x06);
    if (Addr == nullptr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for the frame counter!";
        return nullptr;
    }
    return Addr;
}

// Reduce the speed of Lying Figure spray particles by half
void PatchLyingFigureSprayEffectSpeed()
{
    constexpr BYTE State0SearchBytes[]{ 0xDB, 0x46, 0x18, 0x6A, 0x06, 0xD9, 0x5C, 0x24, 0x14, 0xE8 };
    const DWORD SpraySpeedFactorState0Addr = SearchAndGetAddresses(0x004A40CD, 0x004A437D, 0x004A3C3D, State0SearchBytes, sizeof(State0SearchBytes), 0x25);
    if (!SpraySpeedFactorState0Addr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for state 0!";
        return;
    }

    constexpr BYTE State1SearchBytes[]{ 0x6A, 0x06, 0xD8, 0x7C, 0x24, 0x1C, 0xD9, 0x5C, 0x24, 0x34, 0xD9, 0x46, 0x48, 0xD8, 0x0D };
    const DWORD SpraySpeedFactorState1Addr = SearchAndGetAddresses(0x004A401A, 0x004A42CA, 0x004A3B8A, State1SearchBytes, sizeof(State1SearchBytes), 0x0F);
    if (!SpraySpeedFactorState1Addr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for state 1!";
        return;
    }
    const DWORD SprayYAdjustState1Addr = SpraySpeedFactorState1Addr + 0x39;

    constexpr BYTE State2SearchBytes[]{ 0x6A, 0x0A, 0xD8, 0x7C, 0x24, 0x1C, 0xD9, 0x5C, 0x24, 0x38, 0xD9, 0x46, 0x48, 0xD8, 0x0D };
    const DWORD SpraySpeedFactorState2Addr = SearchAndGetAddresses(0x004A3F15, 0x004A41C5, 0x004A3A85, State2SearchBytes, sizeof(State2SearchBytes), 0x0F);
    if (!SpraySpeedFactorState2Addr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for state 2!";
        return;
    }
    const DWORD SprayYAdjustState2Addr = SpraySpeedFactorState2Addr + 0x39;

    WritePointerToMemory((void*)SpraySpeedFactorState0Addr, (void*)&LyingFigureSpraySpeedFactor0);
    WritePointerToMemory((void*)SpraySpeedFactorState1Addr, (void*)&LyingFigureSpraySpeedFactor1);
    WritePointerToMemory((void*)SpraySpeedFactorState2Addr, (void*)&LyingFigureSpraySpeedFactor2);
    WritePointerToMemory((void*)SprayYAdjustState1Addr, (void*)&SprayYAdjust);
    WritePointerToMemory((void*)SprayYAdjustState2Addr, (void*)&SprayYAdjust);
}

// Patch in logic to spawn spray particles for the Lying Figure every other frame
void PatchLyingFigureSprayEffectSpawnRate()
{
    constexpr BYTE SprayAllocAddrSearchBytes[]{ 0x8B, 0xF0, 0x83, 0xC4, 0x08, 0x85, 0xF6, 0x0F };
    DWORD SprayAllocAddr = SearchAndGetAddresses(0x004A4A1D, 0x004A4CCD, 0x004A458D, SprayAllocAddrSearchBytes, sizeof(SprayAllocAddrSearchBytes), -0x0D);
    if (!SprayAllocAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for Lying Figure spray alloc handler!";
        return;
    }
    LyingFigureSprayAllocReturnAddr = (DWORD*)(SprayAllocAddr + 0x08);

    LyingFigureSprayHandlerAddr = (DWORD*)ReadSearchedAddresses(0x004A4A1D, 0x004A4CCD, 0x004A458D, SprayAllocAddrSearchBytes, sizeof(SprayAllocAddrSearchBytes), -0x09);
    if (!LyingFigureSprayHandlerAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for Lying Figure spray handler!";
        return;
    }

    WriteJMPtoMemory((BYTE*)SprayAllocAddr, *LyingFigureSprayAllocASM, 0x08);
}

// Reduce the speed of Hyper Spray particles by half
void PatchHyperSprayEffectSpeed()
{
    constexpr BYTE State0SearchBytes[]{ 0xDB, 0x46, 0x18, 0xD8, 0xC9, 0xD9, 0xC9, 0xD8, 0x4E, 0x54, 0xD8, 0x0D };
    const DWORD SpraySpeedFactorState0Addr = SearchAndGetAddresses(0x004A4579, 0x004A4829, 0x004A40E9, State0SearchBytes, sizeof(State0SearchBytes), 0x0C);
    if (!SpraySpeedFactorState0Addr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for state 0!";
        return;
    }

    constexpr BYTE State1SearchBytes[]{ 0x89, 0x44, 0x24, 0x18, 0xDB, 0x44, 0x24, 0x18, 0xD8, 0xC9, 0xD9, 0x5C, 0x24, 0x18, 0xD8, 0x4E, 0x54, 0xD8, 0x0D };
    const DWORD SpraySpeedFactorState1Addr = SearchAndGetAddresses(0x004A44EC, 0x004A479C, 0x004A405C, State1SearchBytes, sizeof(State1SearchBytes), 0x13);
    if (!SpraySpeedFactorState1Addr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for state 1!";
        return;
    }
    const DWORD SprayYAdjustState1Addr = SpraySpeedFactorState1Addr + 0x24;

    constexpr BYTE State2SearchBytes[]{ 0xDB, 0x44, 0x24, 0x18, 0xD8, 0xC9, 0xD9, 0x5C, 0x24, 0x18, 0xD8, 0x4E, 0x54, 0xD8, 0x0D };
    const DWORD SpraySpeedFactorState2Addr = SearchAndGetAddresses(0x004A442C, 0x004A46DC, 0x004A3F9C, State2SearchBytes, sizeof(State2SearchBytes), 0x0F);
    if (!SpraySpeedFactorState2Addr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for state 2!";
        return;
    }
    const DWORD SprayYAdjustState2Addr = SpraySpeedFactorState2Addr + 0x24;

    WritePointerToMemory((void*)SpraySpeedFactorState0Addr, (void*)&HyperSpraySpeedFactor0_1);
    WritePointerToMemory((void*)SpraySpeedFactorState1Addr, (void*)&HyperSpraySpeedFactor0_1);
    WritePointerToMemory((void*)SpraySpeedFactorState2Addr, (void*)&HyperSpraySpeedFactor2);
    WritePointerToMemory((void*)SprayYAdjustState1Addr, (void*)&SprayYAdjust);
    WritePointerToMemory((void*)SprayYAdjustState2Addr, (void*)&SprayYAdjust);
}

// Patch in logic to spawn Hyper Spray particles every other frame
void PatchHyperSprayEffectSpawnRate()
{
    constexpr BYTE SprayAllocAddrSearchBytes[]{ 0x8B, 0xF0, 0x33, 0xC9, 0x83, 0xC4, 0x08, 0x3B, 0xF1, 0x0F };

    DWORD SprayAllocAddr = SearchAndGetAddresses(0x004A509D, 0x004A534D, 0x004A4C0D, SprayAllocAddrSearchBytes, sizeof(SprayAllocAddrSearchBytes), -0x0D);
    if (!SprayAllocAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for Lying Figure spray alloc handler!";
        return;
    }
    HyperSprayAllocReturnAddr = (DWORD*)(SprayAllocAddr + 0x8);

    HyperSprayHandlerAddr = (DWORD*)ReadSearchedAddresses(0x004A509D, 0x004A534D, 0x004A4C0D, SprayAllocAddrSearchBytes, sizeof(SprayAllocAddrSearchBytes), -0x09);
    if (!HyperSprayHandlerAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address for Lying Figure spray handler!";
        return;
    }

    WriteJMPtoMemory((BYTE*)SprayAllocAddr, *HyperSprayAllocASM, 0x08);
}

// Patch spray effects when playing at 60 FPS
void PatchSprayEffect()
{
    Logging::Log() << "Enabling Lying Figure Spray Effect Fix...";
    ParticleTableAddr = GetParticleTableAddr();
    FrameCounterAddr = GetFrameCounterAddr();
    if (ParticleTableAddr == nullptr || FrameCounterAddr == nullptr)
    {
        return;
    }
    PatchLyingFigureSprayEffectSpeed();
    PatchLyingFigureSprayEffectSpawnRate();

    Logging::Log() << "Enabling Hyper Spray Effect Fix...";
    PatchHyperSprayEffectSpeed();
    PatchHyperSprayEffectSpawnRate();
}
