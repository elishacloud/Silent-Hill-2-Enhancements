/**
* Copyright (C) 2024 Elisha Riedlinger
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
DWORD *RowboatPointer;

// ASM functions to update Rowboat Animation
__declspec(naked) void __stdcall RowboatAnimationASM()
{
	__asm
	{
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
		retn
	}
}

// Patch SH2 code to Fix Rowboat Animation
void PatchRowboatAnimation()
{
	// Get Rowboat Animation address
	constexpr BYTE RowboatSearchBytes[]{ 0x8B, 0x56, 0x08, 0x89, 0x10, 0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x50, 0xC3 };
	DWORD RowboatAddr = SearchAndGetAddresses(0x004A0293, 0x004A0543, 0x0049FE03, RowboatSearchBytes, sizeof(RowboatSearchBytes), 0x22, __FUNCTION__);

	// Get memory pointer for Rowboat Animation
	constexpr BYTE RowboatSearchPtrBytes[] = { 0x56, 0x6A, 0x0A, 0x6A, 0x00, 0x50, 0xE8 };
	DWORD RowboatMemoryPtr = SearchAndGetAddresses(0x0053F9C3, 0x0053FCF3, 0x0053F613, RowboatSearchPtrBytes, sizeof(RowboatSearchPtrBytes), -0x3D, __FUNCTION__);

	// Checking address pointer
	if (!RowboatMemoryPtr || !RowboatAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
		return;
	}
	memcpy(&RowboatPointer, (void*)(RowboatMemoryPtr + 2), sizeof(DWORD));

	// Check for valid code before updating
	if (!CheckMemoryAddress((void*)RowboatAddr, "\xC3", 1, __FUNCTION__) ||
		!CheckMemoryAddress((void*)RowboatMemoryPtr, "\x3B\x05", 2, __FUNCTION__))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Setting Rowboat Animation Fix...";
	WriteJMPtoMemory((BYTE*)RowboatAddr, *RowboatAnimationASM);
}
