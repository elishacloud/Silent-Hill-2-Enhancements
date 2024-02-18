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
}
