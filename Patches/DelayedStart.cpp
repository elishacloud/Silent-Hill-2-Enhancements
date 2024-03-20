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

void DelayedStart();

// Variables for ASM
void* EntryPointExceptionFunction = nullptr;

// ASM function to start functions delayed
__declspec(naked) void __stdcall DelayedStartASM()
{
	__asm
	{
		call DelayedStart
		jmp EntryPointExceptionFunction
	}
}

// Set hook at beginning of Silent Hill 2 code to do startup functions delayed
bool SetDelayedStart()
{
	// Get memory pointer
	constexpr BYTE SearchBytes[]{ 0xFF, 0xD7, 0x66, 0x81, 0x38, 0x4D, 0x5A, 0x75, 0x1F, 0x8B, 0x48, 0x3C, 0x03, 0xC8, 0x81, 0x39 };
	DWORD Address = SearchAndGetAddresses(0x0056FDEB, 0x0056EBBB, 0x0056E4DB, SearchBytes, sizeof(SearchBytes), -0xE, __FUNCTION__);

	// Checking address pointer
	if (!Address || *(BYTE*)Address != 0xE8)
	{
		Logging::Log() << "Error: failed to set delayed startup...";
		return false;
	}
	EntryPointExceptionFunction = (void*)(Address + 5 + *(DWORD*)(Address + 1));

	// Update SH2 code
	Logging::Log() << "Setting delayed startup...";
	WriteCalltoMemory((BYTE*)Address, *DelayedStartASM, 5);

	// Return
	return true;
}
