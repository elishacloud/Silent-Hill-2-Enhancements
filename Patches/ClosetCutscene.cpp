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

// Update SH2 code to Fix RPT Apartment Closet Cutscene
void UpdateClosetCutscene(DWORD *SH2_CutsceneID, float *SH2_CutsceneCameraPos)
{
	// Get Address1
	static DWORD Address1 = NULL;
	if (!Address1)
	{
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		constexpr BYTE SearchBytes[]{ 0x7C, 0xDC, 0xB0, 0x0C, 0x5F, 0xC6, 0x05 };

		// Get Room 312 Shadow address
		DWORD SearchAddress = (DWORD)CheckMultiMemoryAddress((void*)0x004799D4, (void*)0x00479C74, (void*)0x00479E84, (void*)SearchBytes, sizeof(SearchBytes));

		// Search for address
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " searching for memory address!";
			SearchAddress = (DWORD)GetAddressOfData(SearchBytes, sizeof(SearchBytes), 1, 0x004794D4, 2600);
		}

		// Checking address pointer
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}

		// Get address pointer
		SearchAddress = SearchAddress + 0x07;
		memcpy(&Address1, (void*)(SearchAddress), sizeof(DWORD));
	}

	// Get Address2
	static DWORD Address2 = NULL;
	if (!Address2)
	{
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		constexpr BYTE SearchBytes[]{ 0x03, 0xC6, 0x46, 0x10, 0x03, 0xC6, 0x46, 0x14, 0xF8, 0xC6, 0x46, 0x12, 0x40, 0xC6, 0x46, 0x11, 0x82, 0xE8 };

		// Get Room 312 Shadow address
		Address2 = (DWORD)CheckMultiMemoryAddress((void*)0x005999CA, (void*)0x0059A27A, (void*)0x00599B9A, (void*)SearchBytes, sizeof(SearchBytes));

		// Search for address
		if (!Address2)
		{
			Logging::Log() << __FUNCTION__ << " searching for memory address!";
			Address2 = (DWORD)GetAddressOfData(SearchBytes, sizeof(SearchBytes), 1, 0x005994CA, 2600);
		}

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
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		constexpr BYTE SearchBytes[]{ 0x5E, 0x83, 0xC4, 0x10, 0xC3, 0xC6, 0x46, 0x07, 0x00, 0x8B, 0x15 };

		// Get Room 312 Shadow address
		DWORD SearchAddress = (DWORD)CheckMultiMemoryAddress((void*)0x0043F9BF, (void*)0x0043FB7F, (void*)0x0043FB7F, (void*)SearchBytes, sizeof(SearchBytes));

		// Search for address
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " searching for memory address!";
			SearchAddress = (DWORD)GetAddressOfData(SearchBytes, sizeof(SearchBytes), 1, 0x0043F4BF, 2600);
		}

		// Checking address pointer
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}

		// Get address pointer
		SearchAddress = SearchAddress - 0x1B;
		memcpy(&Address3, (void*)(SearchAddress), sizeof(DWORD));
	}

	// Log update
	static bool FirstRun = true;
	if (FirstRun)
	{
		Logging::Log() << "Setting RPT Apartment Closet Cutscene Fix...";

		// Reset FirstRun
		FirstRun = false;
	}

	// Darken & blur more closet bars
	static bool ValueSet1 = false;
	float CutsceneCameraPos = -21376.56445f;
	if (*SH2_CutsceneID == 0x0E && *SH2_CutsceneCameraPos == CutsceneCameraPos)
	{
		if (!ValueSet1)
		{
			BYTE ByteValue = 15;
			UpdateMemoryAddress((void*)Address1, &ByteValue, sizeof(BYTE));		// "Time of Day"
			ByteValue = 7;
			UpdateMemoryAddress((void*)Address2, &ByteValue, sizeof(BYTE));		// Closet bar blur intensity
		}
		ValueSet1 = true;
	}
	else if (ValueSet1)
	{
		BYTE ByteValue = 11;
		UpdateMemoryAddress((void*)Address1, &ByteValue, sizeof(BYTE));		// "Time of Day"
		ByteValue = 3;
		UpdateMemoryAddress((void*)Address2, &ByteValue, sizeof(BYTE));		// Closet bar blur intensity
		ValueSet1 = false;
	}

	// Adjust room/dynamic objects for one shot
	static bool ValueSet2 = false;
	CutsceneCameraPos = -20133.99805f;
	if (*SH2_CutsceneID == 0x0E  && *SH2_CutsceneCameraPos == CutsceneCameraPos)
	{
		if (!ValueSet2)
		{
			float Value = -0.2f;
			UpdateMemoryAddress((void*)(Address3 + 0x00), &Value, sizeof(float));		// PC location textures + models color R (set 1)
			UpdateMemoryAddress((void*)(Address3 + 0x04), &Value, sizeof(float));		// PC location textures + models color G (set 1)
			UpdateMemoryAddress((void*)(Address3 + 0x08), &Value, sizeof(float));		// PC location textures + models color B (set 1)
			Value = 1.0f;
			UpdateMemoryAddress((void*)(Address3 + 0x20), &Value, sizeof(float));		// PC location textures + models color R (set 2)
			UpdateMemoryAddress((void*)(Address3 + 0x24), &Value, sizeof(float));		// PC location textures + models color G (set 2)
			UpdateMemoryAddress((void*)(Address3 + 0x28), &Value, sizeof(float));		// PC location textures + models color B (set 2)
		}
		ValueSet2 = true;
	}
	else if (ValueSet2)
	{
		float Value = 0.150000006f;
		UpdateMemoryAddress((void*)(Address3 + 0x00), &Value, sizeof(float));		// PC location textures + models color R (set 1)
		UpdateMemoryAddress((void*)(Address3 + 0x04), &Value, sizeof(float));		// PC location textures + models color G (set 1)
		UpdateMemoryAddress((void*)(Address3 + 0x08), &Value, sizeof(float));		// PC location textures + models color B (set 1)
		Value = 0.04500000179f;
		UpdateMemoryAddress((void*)(Address3 + 0x20), &Value, sizeof(float));		// PC location textures + models color R (set 2)
		UpdateMemoryAddress((void*)(Address3 + 0x24), &Value, sizeof(float));		// PC location textures + models color G (set 2)
		UpdateMemoryAddress((void*)(Address3 + 0x28), &Value, sizeof(float));		// PC location textures + models color B (set 2)
		ValueSet2 = false;
	}
}
