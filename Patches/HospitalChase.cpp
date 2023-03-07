/**
* Copyright (C) 2023 Elisha Riedlinger
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

// Run SH2 code to Fix RPT Hospital Elevator Stabbing Animation
void RunHospitalChase()
{
	static DWORD Address = NULL;
	if (!Address)
	{
		RUNONCE();

		// Get address
		constexpr BYTE SearchBytes[]{ 0x8B, 0xC2, 0x83, 0xC4, 0x18, 0xC3, 0x90, 0x90, 0x8B, 0x44, 0x24, 0x04, 0x83, 0xF8, 0x04, 0x0F, 0x87, 0x0D, 0x01, 0x00, 0x00, 0x53, 0xFF, 0x24, 0x85 };
		Address = ReadSearchedAddresses(0x004A8BD8, 0x004A8E88, 0x004A8748, SearchBytes, sizeof(SearchBytes), 0x1E, __FUNCTION__);
		if (!Address)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	RUNCODEONCE(Logging::Log() << "Setting RPT Hospital Elevator Animation Fix...");

	// Fix Animation
	static bool ValueSet = false;
	if (GetRoomID() == 0x5B)
	{

		if (!ValueSet && GetJamesPosX() > 33185.0f)
		{
			BYTE Value = 5;
			UpdateMemoryAddress((void*)Address, &Value, sizeof(BYTE));

			ValueSet = true;
		}
	}
	else if (ValueSet)
	{
		ValueSet = false;
	}
}
