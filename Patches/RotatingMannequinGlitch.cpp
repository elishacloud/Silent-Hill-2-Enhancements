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
#include "patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Update SH2 code to fix the rotating Mannequin glitch
void UpdateRotatingMannequin(DWORD *SH2_RoomID)
{
	// Get flashlight acquired Address
	static DWORD *FlashlightAcquiredAddr = nullptr;
	if (!FlashlightAcquiredAddr)
	{
		RUNONCE();

		// Get address
		constexpr BYTE SearchBytes[]{ 0x8D, 0x50, 0x1C, 0x8B, 0x0A, 0x89, 0x0D };
		FlashlightAcquiredAddr = (DWORD*)ReadSearchedAddresses(0x0045507D, 0x004552DD, 0x004552DD, SearchBytes, sizeof(SearchBytes), 0x56);

		// Checking address pointer
		if (!FlashlightAcquiredAddr)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	// Get Mannequin state Address
	static DWORD *MannequinStateAddr = nullptr;
	if (!MannequinStateAddr)
	{
		RUNONCE();

		// Get address
		constexpr BYTE SearchBytes[]{ 0x68, 0x00, 0x02, 0x00, 0x00, 0x33, 0xF6, 0x33, 0xDB, 0x50, 0x89, 0x94, 0x24, 0x50, 0x04, 0x00, 0x00 };
		MannequinStateAddr = (DWORD*)ReadSearchedAddresses(0x0048CBC5, 0x0048CE65, 0x0048D075, SearchBytes, sizeof(SearchBytes), 0x34);

		// Checking address pointer
		if (!MannequinStateAddr)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
		MannequinStateAddr = (DWORD*)((DWORD)MannequinStateAddr + 0x60);
	}

	LOG_ONCE("Fixing the rotating Mannequin glitch...");

	// Static updates
	static bool ValueSet = false;
	if (*SH2_RoomID == 0x15 && *MannequinStateAddr != 0x00 && (*FlashlightAcquiredAddr == 0x00340000 || *FlashlightAcquiredAddr == 0x003D2006))
	{
		if (!ValueSet && *MannequinStateAddr == 0x206)
		{
			DWORD Value = 0x207;
			UpdateMemoryAddress(MannequinStateAddr, &Value, sizeof(DWORD));
		}
		ValueSet = true;
	}
	else if (ValueSet)
	{
		ValueSet = false;
	}
}
