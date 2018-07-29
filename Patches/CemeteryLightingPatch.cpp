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
#include "Common\Utils.h"
#include "Common\Logging.h"

// Predefined code bytes
constexpr BYTE CemeterySearchBytes[]{ 0x83, 0xEC, 0x10, 0x55, 0x56, 0x57, 0x50, 0x51, 0x8D, 0x54, 0x24, 0x14, 0x6A, 0x00, 0x52 };
constexpr BYTE CemeteryMOVBytes[]{ 0x89, 0x0D };

// Variables for ASM
DWORD *CemeteryPointer;
void *jmpCemeteryAddr;

// ASM functions to update Cemetery Lighting dynamically
__declspec(naked) void __stdcall CemeteryLightingASM()
{
	__asm
	{
		pushf
		cmp ecx, 0x0001000E
		jne near CemeteryExit
		mov ecx, 0x0001000D
		CemeteryExit:
		push eax
		mov eax, dword ptr ds : [CemeteryPointer]
		mov dword ptr ds : [eax], ecx
		pop eax
		popf
		jmp jmpCemeteryAddr
	}
}

// Update SH2 code to Fix Cemetery Lighting
void UpdateCemeteryLighting()
{
	// Get Cemetery Lighting address
	DWORD CemeteryAddr = (DWORD)GetAddressOfData(CemeterySearchBytes, sizeof(CemeterySearchBytes), 1, 0x0047C09C, 1800);
	if (!CemeteryAddr)
	{
		Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	CemeteryAddr += 0x41;
	memcpy(&CemeteryPointer, (void*)(CemeteryAddr + 2), sizeof(DWORD));
	jmpCemeteryAddr = (void*)(CemeteryAddr + 6);

	// Check for valid code before updating
	if (!CheckMemoryAddress((void*)CemeteryAddr, (void*)CemeteryMOVBytes, sizeof(CemeteryMOVBytes)))
	{
		Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Update SH2 code
	Log() << "Setting Cemetery Lighting Fix...";
	WriteJMPtoMemory((BYTE*)CemeteryAddr, *CemeteryLightingASM, 6);
}
