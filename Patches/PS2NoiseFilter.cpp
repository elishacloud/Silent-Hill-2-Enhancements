/**
* Copyright (C) 2018 Elisha Riedlinger
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
#include "..\Common\Utils.h"
#include "..\Common\Logging.h"

// Predefined addresses
constexpr BYTE FilterByteEDX[2][5] = {
	{ 0xBA, 0xFF, 0x00, 0x00, 0x00 },
	{ 0xBA, 0xD7, 0x01, 0x00, 0x00 } };
constexpr BYTE FilterByteMOV[2][1] = { { 0xFF },{ 0x22 } };
constexpr BYTE FilterByteJMP[] = { 0xA2, 0xC5 };
constexpr BYTE FilterFunctionBtyes[] = { 0x3E, 0xA2, 0x67, 0x45, 0x23, 0x01 };
constexpr BYTE NOP[] = { 0x90 };

// Variables for ASM
BYTE tmpAddr;
DWORD tmpVar;
void *jmpAddr;
constexpr float BrightnessControl = 7.4f;

// ASM function to update PS2NoiseFilter dynamically
#pragma warning(suppress: 4725)
__declspec(naked) void __stdcall PS2NoiseFilterASM()
{
	__asm
	{
		mov tmpAddr, al
		fild dword ptr tmpAddr
		fdiv dword ptr BrightnessControl
		fistp dword ptr tmpVar
		mov eax, tmpVar
		MOV BYTE PTR DS : [0x01234567], AL
		jmp jmpAddr
	}
}

// Update SH2 code for PS2 Style Noise Filter
void UpdatePS2NoiseFilter()
{
	// Get PS2 filter memory address
	DWORD SH2AddrEDX = (DWORD)GetAddressOfData(FilterByteEDX[0], sizeof(FilterByteEDX[0]), 1, 0x0477C1D, 1800);
	if (!SH2AddrEDX)
	{
		Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get relative addresses
	DWORD SH2AddrMOV = SH2AddrEDX + 0x4862;
	DWORD SH2AddrJMP = SH2AddrEDX + 0x483D;
	jmpAddr = (void*)(SH2AddrJMP + 5);

	// Check for valid code before updating
	if (!CheckMemoryAddress((void*)SH2AddrEDX, (void*)FilterByteEDX[0], 5) ||
		!CheckMemoryAddress((void*)SH2AddrMOV, (void*)FilterByteMOV[0], 1) ||
		!CheckMemoryAddress((void*)SH2AddrJMP, (void*)FilterByteJMP, 2))
	{
		Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Find code in fucntion to update
	DWORD fltFunctAddrJMP = (DWORD)GetAddressOfData(FilterFunctionBtyes, 6, 1, (DWORD)*PS2NoiseFilterASM, 50);
	if (!fltFunctAddrJMP)
	{
		Log() << __FUNCTION__ << " Error: failed to find function address!";
		return;
	}

	// Update SH2 code
	Log() << "Setting PS2 Style Noise Filter...";
	UpdateMemoryAddress((void*)SH2AddrEDX, (void*)FilterByteEDX[1], 5);
	UpdateMemoryAddress((void*)SH2AddrMOV, (void*)FilterByteMOV[1], 1);
	UpdateMemoryAddress((void*)(fltFunctAddrJMP + 2), (void*)(SH2AddrJMP + 1), 4);
	WriteJMPtoMemory((BYTE*)SH2AddrJMP, *PS2NoiseFilterASM);
}
