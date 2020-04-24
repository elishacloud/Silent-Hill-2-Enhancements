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

void SetGameLoad()
{
	// Get elevator room save address
	constexpr BYTE SearchBytes[]{ 0x83, 0xC4, 0x10, 0xF7, 0xC1, 0x00, 0x00, 0x00, 0x04, 0x5E, 0x74, 0x0F, 0xC7, 0x05 };
	DWORD GameLoadAddr = SearchAndGetAddresses(0x0058312C, 0x005839DC, 0x005832FC, SearchBytes, sizeof(SearchBytes), 0x94);
	if (!GameLoadAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Enabling Load Game Fix...";
	DWORD Value = 0x00;
	UpdateMemoryAddress((void*)GameLoadAddr, &Value, sizeof(DWORD));
}

void UpdateGameLoad(DWORD *SH2_RoomID, float *SH2_JamesPosX)
{
	// Update save code elevator room
	RUNCODEONCE(SetGameLoad());

	// Get game save address
	static BYTE *Address = nullptr;
	if (!Address)
	{
		RUNONCE();

		// Get address for game save
		constexpr BYTE SearchBytes[]{ 0x3C, 0x1B, 0x74, 0x27, 0x3C, 0x25, 0x74, 0x23, 0x3C, 0x30, 0x74, 0x1F, 0x3C, 0x31, 0x74, 0x1B, 0x3C, 0x32, 0x74, 0x17, 0x3C, 0x33, 0x74, 0x13, 0x3C, 0x34, 0x74, 0x0F };
		Address = (BYTE*)ReadSearchedAddresses(0x0044C648, 0x0044C7E8, 0x0044C7E8, SearchBytes, sizeof(SearchBytes), -0x0D);
		if (!Address)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	// Set static variables
	static bool ValueSet = false;
	static bool ValueUnSet = false;

	// Enable game saves for specific rooms
	if (*SH2_RoomID == 0x29)
	{
		*Address = 1;
		ValueSet = true;
	}
	// Disable game saves for specific rooms
	else if (*SH2_RoomID == 0x13 || *SH2_RoomID == 0x17 || *SH2_RoomID == 0xAA ||
		(*SH2_RoomID == 0x78 && *SH2_JamesPosX < -18600.0f))
	{
		*Address = 0;
		ValueUnSet = true;
	}
	// Reset static variables
	else
	{
		if (ValueSet)
		{
			ValueSet = false;
		}
		if (ValueUnSet)
		{
			*Address = 1;
			ValueUnSet = false;
		}
	}
}
