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

// Run SH2 code to Fix Hotel Water
void RunHotelWater()
{
	// Get Address1
	static DWORD Address1 = NULL;
	if (!Address1)
	{
		RUNONCE();

		// Get Room 312 Shadow address
		constexpr BYTE SearchBytes[]{ 0x00, 0x00, 0x20, 0x40, 0xC7, 0x44, 0x24, 0x64, 0x00, 0x00, 0x20, 0x40, 0xE8, 0xCE };
		Address1 = SearchAndGetAddresses(0x004E34F1, 0x004E37A1, 0x00004E3061, SearchBytes, sizeof(SearchBytes), 0x00);

		// Checking address pointer
		if (!Address1)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	// Get Address2
	static DWORD Address2 = NULL;
	if (!Address2)
	{
		RUNONCE();

		// Get Room 312 Shadow address
		constexpr BYTE SearchBytes[]{ 0xFF, 0xFF, 0xD9, 0x44, 0x24, 0x50, 0x8B, 0x15 };
		Address2 = ReadSearchedAddresses(0x004D87AF, 0x004D8A5F, 0x004D831F, SearchBytes, sizeof(SearchBytes), 0x0E);

		// Checking address pointer
		if (!Address2)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	// Get Address3
	static DWORD Address3 = NULL;
	if (!Address3)
	{
		RUNONCE();

		// Get Room 312 Shadow address
		constexpr BYTE SearchBytes[]{ 0xFF, 0xFF, 0xFF, 0xDD, 0xD8, 0xD9, 0x05 };
		Address3 = ReadSearchedAddresses(0x004D767A, 0x004D792A, 0x004D71EA, SearchBytes, sizeof(SearchBytes), 0x07);

		// Checking address pointer
		if (!Address3)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	// Get Address4
	static DWORD Address4 = NULL;
	if (!Address4)
	{
		RUNONCE();

		constexpr BYTE SearchBytes[]{ 0xFF, 0xFF, 0xFF, 0xDD, 0xD8, 0xD9, 0x05 };

		// Get Room 312 Shadow address
		Address4 = ReadSearchedAddresses(0x004D25CC, 0x004D287C, 0x004D213C, SearchBytes, sizeof(SearchBytes), 0x07);

		// Checking address pointer
		if (!Address4)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}
	static float *Address4_a = (float*)(Address4 + 0x00);
	static float *Address4_b = (float*)(Address4 + 0x04);
	static float *Address4_c = (float*)(Address4 + 0x08);
	static float *Address4_d = (float*)(Address4 + 0x0C);

	// Static updates
	static bool FirstRun = true;
	if (FirstRun)
	{
		Logging::Log() << "Setting Hotel Water Fix...";

		// Hallway After Alternate Hotel Kitchen Water
		float Value = 1.75f;
		UpdateMemoryAddress((void*)Address1, &Value, sizeof(float));				// Water texture Z stretch/shrink

		// Alternate Hotel Kitchen Water
		Value = 100.0f;
		UpdateMemoryAddress((void*)(Address2 + 0x00), &Value, sizeof(float));	// Water color (flashlight off) - R
		UpdateMemoryAddress((void*)(Address2 + 0x04), &Value, sizeof(float));	// Water color (flashlight off) - G
		UpdateMemoryAddress((void*)(Address2 + 0x08), &Value, sizeof(float));	// Water color (flashlight off) - B
		Value = 255.0f;
		UpdateMemoryAddress((void*)(Address2 + 0x10), &Value, sizeof(float));	// Ripple specularity - R
		UpdateMemoryAddress((void*)(Address2 + 0x14), &Value, sizeof(float));	// Ripple specularity - G
		UpdateMemoryAddress((void*)(Address2 + 0x18), &Value, sizeof(float));	// Ripple specularity - B

		// Staircase After Angela Fire Cutscene Water
		Value = 125.0f;
		UpdateMemoryAddress((void*)(Address3 + 0x20), &Value, sizeof(float));	// Water color (flashlight off) - R
		UpdateMemoryAddress((void*)(Address3 + 0x24), &Value, sizeof(float));	// Water color (flashlight off) - G
		UpdateMemoryAddress((void*)(Address3 + 0x28), &Value, sizeof(float));	// Water color (flashlight off) - B
		Value = 64.0f;
		UpdateMemoryAddress((void*)(Address3 + 0x00), &Value, sizeof(float));	// Ripple specularity - R
		UpdateMemoryAddress((void*)(Address3 + 0x04), &Value, sizeof(float));	// Ripple specularity - G
		UpdateMemoryAddress((void*)(Address3 + 0x08), &Value, sizeof(float));	// Ripple specularity - B

		// Reset FirstRun
		FirstRun = false;
	}

	// Strange Area 2 and Labyrinth
	{
		const DWORD RoomIDs[] = { 0x63, 0x7A, 0x7C, 0x7E, 0x80, 0x82, 0x85 };
		bool RoomsMatch = false;
		for (auto const &RoomID : RoomIDs)
		{
			if (GetRoomID() == RoomID)
			{
				RoomsMatch = true;
			}
		}

		const float Value1 = 19.0f;
		const float Value2 = 128.0f;
		if (RoomsMatch && (*Address4_a != Value1 || *Address4_b != Value1 || *Address4_c != Value1 || *Address4_d != Value2))
		{
			UpdateMemoryAddress(Address4_a, &Value1, sizeof(float));	// Water color (flashlight off) - R
			UpdateMemoryAddress(Address4_b, &Value1, sizeof(float));	// Water color (flashlight off) - G
			UpdateMemoryAddress(Address4_c, &Value1, sizeof(float));	// Water color (flashlight off) - B
			UpdateMemoryAddress(Address4_d, &Value2, sizeof(float));	// Water intensity (flashlight off)
		}
	}
	// Alternate Hotel Bar Water and Elevator, Hallway After Alternate Hotel Kitchen Water, Final Boss Metal Staircase Water
	{
		const DWORD RoomIDs[] = { 0xB8, 0xB6, 0xB9, 0xBB };
		bool RoomsMatch = false;
		for (auto const &RoomID : RoomIDs)
		{
			if (GetRoomID() == RoomID)
			{
				RoomsMatch = true;
			}
		}

		const float Value1 = 100.0f;
		const float Value2 = 255.0f;
		if (RoomsMatch && (*Address4_a != Value1 || *Address4_b != Value1 || *Address4_c != Value1 || *Address4_d != Value2))
		{
			UpdateMemoryAddress(Address4_a, &Value1, sizeof(float));	// Water color (flashlight off) - R
			UpdateMemoryAddress(Address4_b, &Value1, sizeof(float));	// Water color (flashlight off) - G
			UpdateMemoryAddress(Address4_c, &Value1, sizeof(float));	// Water color (flashlight off) - B
			UpdateMemoryAddress(Address4_d, &Value2, sizeof(float));	// Water intensity (flashlight off)
		}
	}
}
