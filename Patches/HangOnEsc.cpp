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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Update SH2 code to Fix an issue where the game will hang when Esc is pressed while transition is active
void UpdateHangOnEsc(DWORD *SH2_RoomID)
{
	static DWORD *ScreenEvent = nullptr;
	if (!ScreenEvent)
	{
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		constexpr BYTE SearchBytesScreenEvent[]{ 0x83, 0xF8, 0x19, 0x7E, 0x72, 0x83, 0xF8, 0x1A, 0x75, 0x05, 0xBF, 0x01, 0x00, 0x00, 0x00, 0x39, 0x1D };
		ScreenEvent = (DWORD*)ReadSearchedAddresses(0x0048E87B, 0x0048EB1B, 0x0048ED2B, SearchBytesScreenEvent, sizeof(SearchBytesScreenEvent), 0x2A);
		if (!ScreenEvent)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	static DWORD Address = NULL;
	if (!Address)
	{
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		Address = 0x0079B834;
		constexpr BYTE SearchBytes[]{ 0x8B, 0x10, 0x6A, 0x00, 0x6A, 0x1B, 0x50, 0xFF, 0x92, 0xC8, 0x00, 0x00, 0x00, 0x81, 0xC4, 0x68, 0x01, 0x00, 0x00, 0xC3 };
		Address = ReadSearchedAddresses(0x0044C615, 0x0044C7B5, 0x0044C7B5, SearchBytes, sizeof(SearchBytes), 0x26);
		if (!Address)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
		Address += 0x04;
	}

	// Log update
	static bool FirstRun = true;
	if (FirstRun)
	{
		Logging::Log() << "Fixing Esc while transition is active...";

		// Reset FirstRun
		FirstRun = false;
	}

	// Prevent player from pressing Esc while transition is active
	if (*ScreenEvent == 0x03 || (IsInBloomEffect && *SH2_RoomID == 0x90))
	{
		DWORD Value = 3;
		UpdateMemoryAddress((void*)Address, &Value, sizeof(DWORD));
	}
}
