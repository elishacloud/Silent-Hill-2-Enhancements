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

// Update SH2 code to Fix Hotel Water
void UpdateHotelWater(DWORD *SH2_RoomID)
{
	static bool FirstRun = true;
	if (FirstRun)
	{
		// Hallway After Alternate Hotel Kitchen Water
		DWORD Address = 0x004E34F1;
		float Value = 1.75f;
		UpdateMemoryAddress((void*)Address, &Value, sizeof(float));				// Water texture Z stretch/shrink

		// Alternate Hotel Kitchen Water
		Address = 0x00894084;
		Value = 100.0f;
		UpdateMemoryAddress((void*)(Address + 0x00), &Value, sizeof(float));	// Water color (flashlight off) - R
		UpdateMemoryAddress((void*)(Address + 0x04), &Value, sizeof(float));	// Water color (flashlight off) - G
		UpdateMemoryAddress((void*)(Address + 0x08), &Value, sizeof(float));	// Water color (flashlight off) - B
		Value = 255.0f;
		UpdateMemoryAddress((void*)(Address + 0x10), &Value, sizeof(float));	// Ripple specularity - R
		UpdateMemoryAddress((void*)(Address + 0x14), &Value, sizeof(float));	// Ripple specularity - G
		UpdateMemoryAddress((void*)(Address + 0x18), &Value, sizeof(float));	// Ripple specularity - B

		// Staircase After Angela Fire Cutscene Water
		Address = 0x00893F5C;
		Value = 125.0f;
		UpdateMemoryAddress((void*)(Address + 0x20), &Value, sizeof(float));	// Water color (flashlight off) - R
		UpdateMemoryAddress((void*)(Address + 0x24), &Value, sizeof(float));	// Water color (flashlight off) - G
		UpdateMemoryAddress((void*)(Address + 0x28), &Value, sizeof(float));	// Water color (flashlight off) - B
		Value = 64.0f;
		UpdateMemoryAddress((void*)(Address + 0x00), &Value, sizeof(float));	// Ripple specularity - R
		UpdateMemoryAddress((void*)(Address + 0x04), &Value, sizeof(float));	// Ripple specularity - G
		UpdateMemoryAddress((void*)(Address + 0x08), &Value, sizeof(float));	// Ripple specularity - B

		// Reset FirstRun
		FirstRun = false;
	}

	// Strange Area 2 and Labyrinth
	{
		const DWORD RoomIDs[] = { 0x63, 0x82, 0x80, 0x7E, 0x7C, 0x7A };
		bool RoomsMatch = false;
		for (auto const &RoomID : RoomIDs)
		{
			if (*SH2_RoomID == RoomID)
			{
				RoomsMatch = true;
			}
		}

		if (RoomsMatch)
		{
			DWORD Address = 0x00893AB4;
			float Value = 19.0f;
			UpdateMemoryAddress((void*)(Address + 0x00), &Value, sizeof(float));	// Water color (flashlight off) - R
			UpdateMemoryAddress((void*)(Address + 0x04), &Value, sizeof(float));	// Water color (flashlight off) - G
			UpdateMemoryAddress((void*)(Address + 0x08), &Value, sizeof(float));	// Water color (flashlight off) - B
			Value = 128.0f;
			UpdateMemoryAddress((void*)(Address + 0x0C), &Value, sizeof(float));	// Water intensity (flashlight off)
		}
	}
	// Alternate Hotel Bar Water and Elevator, Hallway After Alternate Hotel Kitchen Water, Final Boss Metal Staircase Water
	{
		const DWORD RoomIDs[] = { 0xB8, 0xB6, 0xB9, 0xBB };
		bool RoomsMatch = false;
		for (auto const &RoomID : RoomIDs)
		{
			if (*SH2_RoomID == RoomID)
			{
				RoomsMatch = true;
			}
		}

		if (RoomsMatch)
		{
			DWORD Address = 0x00893AB4;
			float Value = 100.0f;
			UpdateMemoryAddress((void*)(Address + 0x00), &Value, sizeof(float));	// Water color (flashlight off) - R
			UpdateMemoryAddress((void*)(Address + 0x04), &Value, sizeof(float));	// Water color (flashlight off) - G
			UpdateMemoryAddress((void*)(Address + 0x08), &Value, sizeof(float));	// Water color (flashlight off) - B
			Value = 255.0f;
			UpdateMemoryAddress((void*)(Address + 0x0C), &Value, sizeof(float));	// Water intensity (flashlight off)
		}
	}
}
