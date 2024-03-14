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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

void PatchFireEscapeKey()
{
	// Get Fire Escape Key pointer
	constexpr BYTE SearchBytes[]{ 0xC3, 0xB8 };
	DWORD* FireEscapeKeyPtr = (DWORD*)ReadSearchedAddresses(0x00494F00, 0x004951A0, 0x004953B0, SearchBytes, sizeof(SearchBytes), 0x2, __FUNCTION__);
	if (!FireEscapeKeyPtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
		return;
	}

	// Get Fire Escape Key address
	DWORD FireEscapeKeyAddr = 0;
	if (!ReadMemoryAddress(FireEscapeKeyPtr, &FireEscapeKeyAddr, sizeof(DWORD)) || !FireEscapeKeyAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get Pickup Fire Escape Key address
	DWORD PickupKeyAddr = FireEscapeKeyAddr + 0x64;

	// Get Examine Fire Escape Key (Behind Bars) address
	DWORD ExamineKeyAddr = PickupKeyAddr + 0x7D8;

	// Update SH2 code
	Logging::Log() << "Enabling Fire Escape Key Fix...";
	UpdateMemoryAddress((void*)PickupKeyAddr, "\x00", sizeof(BYTE));
	UpdateMemoryAddress((void*)ExamineKeyAddr, "\x00", sizeof(BYTE));
}
