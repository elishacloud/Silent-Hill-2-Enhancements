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
#include "Logging\Logging.h"

// Predefined code bytes
constexpr BYTE RowboatSearchBytes[]{ 0x8B, 0x56, 0x08, 0x89, 0x10, 0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x50, 0xC3 };
constexpr BYTE RowboatRETNBytes[]{ 0xC3 };
constexpr BYTE RowboatSearchPtrBytes[] = { 0x56, 0x6A, 0x0A, 0x6A, 0x00, 0x50, 0xE8 };
constexpr BYTE RowboatCMPBytes[] = { 0x3B, 0x05 };

// Variables for ASM
DWORD *RowboatPointer;

// ASM functions to update Rowboat Animation
__declspec(naked) void __stdcall RowboatAnimationASM()
{
	__asm
	{
		pushf
		cmp eax, 0x4C
		jne near RowboatExit
		push ecx
		push eax
		xor ecx, ecx
		dec ecx
		mov eax, dword ptr ds : [RowboatPointer]
		mov dword ptr ds : [eax], ecx
		pop eax
		pop ecx

	RowboatExit:
		popf
		retn
	}
}

// Update SH2 code to Fix Rowboat Animation
void UpdateRowboatAnimation()
{
	// Get Rowboat Animation address
	DWORD RowboatAddr = (DWORD)CheckMultiMemoryAddress((void*)0x004A0293, (void*)0x004A0543, (void*)0x0049FE03, (void*)RowboatSearchBytes, sizeof(RowboatSearchBytes));

	// Search for address
	if (!RowboatAddr)
	{
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		RowboatAddr = (DWORD)GetAddressOfData(RowboatSearchBytes, sizeof(RowboatSearchBytes), 1, 0x000049FBA5, 2600);
	}

	// Checking address pointer
	if (!RowboatAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	RowboatAddr += 0x22;

	// Get memory pointer for Rowboat Animation
	DWORD RowboatMemoryPtr = (DWORD)CheckMultiMemoryAddress((void*)0x0053F9C3, (void*)0x0053FCF3, (void*)0x0053F613, (void*)RowboatSearchPtrBytes, sizeof(RowboatSearchPtrBytes));

	// Search for address
	if (!RowboatMemoryPtr)
	{
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		RowboatMemoryPtr = (DWORD)GetAddressOfData(RowboatSearchPtrBytes, sizeof(RowboatSearchPtrBytes), 1, 0x000053F356, 2600);
	}

	// Checking address pointer
	if (!RowboatMemoryPtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
		return;
	}
	RowboatMemoryPtr -= 0x3D;
	memcpy(&RowboatPointer, (void*)(RowboatMemoryPtr + 2), sizeof(DWORD));

	// Check for valid code before updating
	if (!CheckMemoryAddress((void*)RowboatAddr, (void*)RowboatRETNBytes, sizeof(RowboatRETNBytes)) ||
		!CheckMemoryAddress((void*)RowboatMemoryPtr, (void*)RowboatCMPBytes, sizeof(RowboatCMPBytes)))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Setting Rowboat Animation Fix...";
	WriteJMPtoMemory((BYTE*)RowboatAddr, *RowboatAnimationASM);
}
