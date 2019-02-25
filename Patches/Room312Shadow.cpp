/**
* Copyright (C) 2019 Elisha Riedlinger
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

// Update SH2 code to Fix Hotel Room 312 Shadow Flicker
void UpdateRoom312ShadowFix(DWORD *SH2_RoomID)
{
	// Get Address
	static DWORD Address = NULL;
	if (!Address)
	{
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		constexpr BYTE SearchBytes[]{ 0xC1, 0xEA, 0x11, 0xC1, 0xE9, 0x1D, 0xC1, 0xE8, 0x0A, 0xC1, 0xEF, 0x03, 0x23, 0xD3, 0x23, 0xCB, 0x23, 0xC3, 0x23, 0xFB };

		// Get Room 312 Shadow address
		DWORD SearchAddress = (DWORD)CheckMultiMemoryAddress((void*)0x004F727D, (void*)0x004F75AD, (void*)0x004F6ECC, (void*)SearchBytes, sizeof(SearchBytes));

		// Search for address
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " searching for memory address!";
			SearchAddress = (DWORD)GetAddressOfData(SearchBytes, sizeof(SearchBytes), 1, 0x004F6D7D, 2600);
		}

		// Checking address pointer
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}

		// Get address pointer
		SearchAddress = SearchAddress + 0x21;
		memcpy(&Address, (void*)(SearchAddress), sizeof(DWORD));
		Address = Address + 0x05;
	}

	// Log update
	static bool FirstRun = true;
	if (FirstRun)
	{
		Logging::Log() << "Setting Hotel Room 312 Shadow Flicker Fix...";

		// Reset FirstRun
		FirstRun = false;
	}

	// Set value for Room 312 Shadow fix
	static bool ValueSet = false;
	if (*SH2_RoomID == 0xA2 && !ValueSet)
	{
		BYTE Value = 3;
		if (Address == 0x00A333C5)
		{
			Value = 7;
		}

		UpdateMemoryAddress((void*)Address, &Value, sizeof(BYTE));
		ValueSet = true;
	}
	else if (*SH2_RoomID != 0xA2 && ValueSet)
	{
		BYTE Value = 0;
		UpdateMemoryAddress((void*)Address, &Value, sizeof(BYTE));
		ValueSet = false;
	}
}
