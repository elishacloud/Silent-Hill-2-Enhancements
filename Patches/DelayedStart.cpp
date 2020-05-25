/**
* Copyright (C) 2020 Elisha Riedlinger
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

// Variables for ASM
DWORD GameAddressPointer;
void *jmpDelayedStart;

// ASM function to start functions delayed
__declspec(naked) void __stdcall DelayedStartASM()
{
	__asm
	{
		push eax
		push ebx
		push ecx
		call DelayedStart
		pop ecx
		pop ebx
		pop eax
		push dword ptr ds : [GameAddressPointer]
		jmp jmpDelayedStart
	}
}

// Set hook at beginning of Silent Hill 2 code to do startup functions delayed
void SetDelayedStart()
{
	// Get memory pointer
	constexpr BYTE SearchBytes[]{ 0xFF, 0xD7, 0x66, 0x81, 0x38, 0x4D, 0x5A, 0x75, 0x1F, 0x8B, 0x48, 0x3C, 0x03, 0xC8, 0x81, 0x39 };
	DWORD Address = SearchAndGetAddresses(0x0056FDEB, 0x0056EBBB, 0x0056E4DB, SearchBytes, sizeof(SearchBytes), -0x13);

	// Checking address pointer
	if (!Address)
	{
		return;
	}
	GameAddressPointer = *(DWORD*)(Address + 2);
	jmpDelayedStart = (void*)(Address + 5);

	// Update SH2 code
	WriteJMPtoMemory((BYTE*)Address, *DelayedStartASM, 5);
}
