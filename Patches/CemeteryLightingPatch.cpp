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
constexpr BYTE CemeteryByte[]{
	0x83, 0xEC, 0x10, 0x55, 0x56, 0x57, 0x50, 0x51, 0x8D, 0x54, 0x24, 0x14, 0x6A, 0x00, 0x52 };
constexpr BYTE CemeteryMOVBytes[]{ 0x89, 0x0D };
constexpr BYTE CemeteryFunctionBtyes[] = { 0x3E, 0x89, 0x0D, 0x67, 0x45, 0x23, 0x01 };
constexpr BYTE CemeteryNOP[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

// Declare fucntions
void __stdcall CemeteryLightingUpdateASM();
void __stdcall CemeteryLightingExitASM();

// Variable for ASM
void *jmpCemeteryAddr;

// ASM functions to update Cemetery Lighting dynamically
#pragma warning(disable: 4414)
__declspec(naked) void __stdcall CemeteryLightingEntryASM()
{
	__asm
	{
		cmp ecx, 0x0001000E
		je near CemeteryLightingUpdateASM
		jmp CemeteryLightingExitASM
	}
}
#pragma warning(default: 4414)
__declspec(naked) void __stdcall CemeteryLightingUpdateASM()
{
	__asm
	{
		mov ecx, 0x0001000D
		jmp CemeteryLightingExitASM
	}
}
__declspec(naked) void __stdcall CemeteryLightingExitASM()
{
	__asm
	{
		MOV DWORD PTR DS : [0x01234567], ECX
		cmp ebp, 02
		jmp jmpCemeteryAddr
	}
}

// Update SH2 code to Fix Cemetery Lighting
void UpdateCemeteryLighting()
{
	// Get Cemetery Lighting address
	DWORD CemeteryAddr = (DWORD)GetAddressOfData(CemeteryByte, sizeof(CemeteryByte), 1, 0x0047C09C, 1800);
	if (!CemeteryAddr)
	{
		Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	CemeteryAddr += 0x41;

	// Check for valid code before updating
	if (!CheckMemoryAddress((void*)CemeteryAddr, (void*)CemeteryMOVBytes, sizeof(CemeteryMOVBytes)) ||
		!CheckMemoryAddress((void*)*CemeteryLightingExitASM, (void*)CemeteryFunctionBtyes, sizeof(CemeteryFunctionBtyes)))
	{
		Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	Log() << "Setting Cemetery Lighting Fix...";

	// Update SH2 code
	jmpCemeteryAddr = (void*)(CemeteryAddr + 6);
	UpdateMemoryAddress((void*)((DWORD)*CemeteryLightingExitASM + 3), (void*)(CemeteryAddr + 2), sizeof(DWORD));
	UpdateMemoryAddress((void*)CemeteryAddr, (void*)CemeteryNOP, sizeof(CemeteryNOP));
	WriteJMPtoMemory((BYTE*)CemeteryAddr, *CemeteryLightingEntryASM);
}
