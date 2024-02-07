/**
* Copyright (C) 2023 Elisha Riedlinger
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
*
* Based on Nemesis2000 Fog Fix: http://ps2wide.net/pc.html#sh2
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

// Variables for ASM
DWORD FogDensityMemoryAddr;
void *jmpFogDensityReturnAddr;
void *jmpClearFogWorkReturnAddr;

constexpr int kFogParticleCount = 2000;
constexpr int kSizeOfFogParticle = 0x60;
BYTE FogParticleArray[kFogParticleCount * kSizeOfFogParticle];

// ASM function to update fog density
__declspec(naked) void __stdcall FogDensityASM()
{
	__asm
	{
		push eax
		fmul dword ptr ds : [fog_layer2_density_mult]
		fadd dword ptr ds : [fog_layer2_density_add]
		mov eax, dword ptr ds : [FogDensityMemoryAddr]
		fstp dword ptr ds : [eax]
		pop eax
		jmp jmpFogDensityReturnAddr
	}
}

void ClearFogParticleArray()
{
    memset(FogParticleArray, 0, sizeof(FogParticleArray));
}

// ASM function to reset fog work data
__declspec(naked) void __stdcall ClearFogWorkASM()
{
    __asm
    {
        call ClearFogParticleArray
        push 0x11390
        jmp jmpClearFogWorkReturnAddr
    }
}

void PatchExperimentalFogParticles()
{
    constexpr BYTE ClearFogWorkSearchBytes[]{ 0x68, 0x90, 0x13, 0x01, 0x00 };
    DWORD ClearFogWorkAddr = SearchAndGetAddresses(0x00489640, 0x004898E0, 0x00489AF0, ClearFogWorkSearchBytes, sizeof(ClearFogWorkSearchBytes), 0x00, __FUNCTION__);
    if (!ClearFogWorkAddr)
    {
        Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
        return;
    }
    jmpClearFogWorkReturnAddr = (void*)(ClearFogWorkAddr + 0x05);

    // Relocate every reference to the fog particle array to our custom storage.
    // TODO: Remove hardcoded addresses for V1.0.
    std::pair<void*, DWORD> fog_addresses[] = {
        {(void*)(0x004875AC), (DWORD)(FogParticleArray + 0x50)},
        {(void*)(0x004881DD), (DWORD)(FogParticleArray + 0x20)},
        {(void*)(0x0048825C), (DWORD)(FogParticleArray)},
        {(void*)(0x004888E1), (DWORD)(FogParticleArray + 0x10)},
        {(void*)(0x00489148), (DWORD)(FogParticleArray)},
        {(void*)(0x00489154), (DWORD)(FogParticleArray + 0x08)},
        {(void*)(0x004891E8), (DWORD)(FogParticleArray + 0x18)},
        {(void*)(0x00489500), (DWORD)(FogParticleArray)},
        // {(void*)(0x00489646), (DWORD)(FogParticleArray)}
    };
    
    // Increase the maximum number of allowed particles.
    // TODO: Remove hardcoded addresses for V1.0.
    UpdateMemoryAddress((void*)(0x00487571), &kFogParticleCount, sizeof(DWORD));
    UpdateMemoryAddress((void*)(0x00487589), &kFogParticleCount, sizeof(DWORD));
    UpdateMemoryAddress((void*)(0x00487590), &kFogParticleCount, sizeof(DWORD));

    for (const auto& [address, value] : fog_addresses) {
        UpdateMemoryAddress(address, &value, sizeof(DWORD));
    }
    WriteJMPtoMemory((BYTE*)ClearFogWorkAddr, *ClearFogWorkASM, 5);
}

// Patch the custom fog
void PatchCustomFog()
{
	// Get fog addresses
	constexpr BYTE TL1SearchBytes[]{ 0x8B, 0x4C, 0x24, 0x00, 0x89, 0x44, 0x24, 0x08, 0xDB, 0x44, 0x24, 0x08, 0x89, 0x0D };
	DWORD TransparencyLayer1Addr = SearchAndGetAddresses(0x0048734C, 0x004875EC, 0x004877FC, TL1SearchBytes, sizeof(TL1SearchBytes), 0x1D, __FUNCTION__);

	constexpr BYTE TL2SearchBytes[]{ 0xD9, 0x94, 0x24, 0x0C, 0x01, 0x00, 0x00, 0xD8, 0x5C, 0x24, 0x24, 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x0F, 0x8B };
	DWORD TransparencyLayer2Addr = SearchAndGetAddresses(0x0048893D, 0x00488BDD, 0x00488DED, TL2SearchBytes, sizeof(TL2SearchBytes), 0x31, __FUNCTION__);

	if (!TransparencyLayer1Addr || !TransparencyLayer2Addr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Layer1 X/Y addresses
	DWORD Layer1X1Addr = TransparencyLayer1Addr + 0x17;
	DWORD Layer1X2Addr = TransparencyLayer1Addr + 0x32;
	DWORD Layer1Y1Addr = Layer1X1Addr + 0x0A;
	DWORD Layer1Y2Addr = Layer1X2Addr + 0x0A;

	// Complexity address
	DWORD Layer2ComplexityAddr = *(DWORD*)(TransparencyLayer2Addr + 0x402);

	// ASM addresses
	DWORD DensityAddr = TransparencyLayer2Addr + 0x667;
	memcpy(&FogDensityMemoryAddr, (void*)(DensityAddr + 2), sizeof(float));
	jmpFogDensityReturnAddr = (void*)(DensityAddr + 6);

	// Update SH2 code
	Logging::Log() << "Enabling Fog Fix...";
	DWORD Address = (DWORD)&fog_transparency_layer1;
	UpdateMemoryAddress((void*)TransparencyLayer1Addr, &Address, sizeof(DWORD));
	Address = (DWORD)&fog_transparency_layer2;
	UpdateMemoryAddress((void*)TransparencyLayer2Addr, &Address, sizeof(DWORD));
	Address = (DWORD)&fog_layer1_x1;
	UpdateMemoryAddress((void*)Layer1X1Addr, &Address, sizeof(DWORD));
	Address = (DWORD)&fog_layer1_x2;
	UpdateMemoryAddress((void*)Layer1X2Addr, &Address, sizeof(DWORD));
	Address = (DWORD)&fog_layer1_y1;
	UpdateMemoryAddress((void*)Layer1Y1Addr, &Address, sizeof(DWORD));
	Address = (DWORD)&fog_layer1_y2;
	UpdateMemoryAddress((void*)Layer1Y2Addr, &Address, sizeof(DWORD));
	UpdateMemoryAddress((void*)Layer2ComplexityAddr, &fog_layer2_complexity, sizeof(float));
	WriteJMPtoMemory((BYTE*)DensityAddr, *FogDensityASM, 6);

    PatchExperimentalFogParticles();
}
