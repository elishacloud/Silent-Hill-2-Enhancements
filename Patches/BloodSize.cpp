/**
* Copyright (C) 2022 Elisha Riedlinger
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

void SetBloodSize()
{
	// Get Blood Size address
	constexpr BYTE SearchBytes[]{ 0xD9, 0x44, 0x24, 0x14, 0xD8, 0x64, 0x24, 0x08, 0xDE, 0xC9, 0xD8, 0x44, 0x24, 0x08, 0xD9, 0x81 };
	DWORD BloodSizeAddr = ReadSearchedAddresses(0x004CE1DA, 0x004CE48A, 0x004CDD4A, SearchBytes, sizeof(SearchBytes), 0x10);
	if (!BloodSizeAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	BloodSizeAddr += 0x6C;

	// Update SH2 code
	Logging::Log() << "Enabling Blood Size Fix...";
	float Value = 1.0f;
	UpdateMemoryAddress((void*)BloodSizeAddr, &Value, sizeof(float));
}

void RunBloodSize()
{
	// Update SH2 code to enable blood position fix
	RUNCODEONCE(SetBloodSize());

	// Get blood position address for Apartment Mannequin/Flashlight Room
	static float *Address1 = nullptr;
	if (!Address1)
	{
		RUNONCE();

		// Get address for blood position
		constexpr BYTE SearchBytes[]{ 0x83, 0xC4, 0x08, 0x83, 0xC6, 0x08, 0x81, 0xFE, 0xC8, 0x00, 0x00, 0x00, 0x72, 0xC9, 0x5F, 0x5E, 0x5B, 0x83, 0xC4, 0x40, 0xC3, 0x90, 0x68 };
		Address1 = (float*)ReadSearchedAddresses(0x004CAB9A, 0x004CAE4A, 0x004CA70A, SearchBytes, sizeof(SearchBytes), 0x5A);
		if (!Address1)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
		Address1 = (float*)((DWORD)Address1 + 0x6C);
	}
	static float *Address1_a = (float*)((DWORD)Address1 + 0x000);
	static float *Address1_b = (float*)((DWORD)Address1 + 0x058);
	static float *Address1_c = (float*)((DWORD)Address1 + 0x0B0);
	static float *Address1_d = (float*)((DWORD)Address1 + 0x4D0);

	// Get blood position address for Flesh Room
	static DWORD Address2 = NULL;
	if (!Address2)
	{
		RUNONCE();

		// Get address for blood position
		constexpr BYTE SearchBytes[]{ 0x5D, 0x33, 0xC0, 0x5B, 0x81, 0xC4, 0x10, 0x01, 0x00, 0x00, 0xC3, 0x90, 0x90, 0x56, 0x6A, 0x00, 0x8B, 0xF0 };
		Address2 = ReadSearchedAddresses(0x004CE5C3, 0x004CE873, 0x004CE133, SearchBytes, sizeof(SearchBytes), 0x96);
		if (!Address2)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
		Address2 = Address2 + 0x20;
	}
	//static float *Address2_a = (float*)(Address2 + 0x00);		// Not currently used
	static float *Address2_b = (float*)(Address2 + 0x08);
	static float *Address2_c = (float*)(Address2 + 0x18);
	static float *Address2_d = (float*)(Address2 + 0x20);

	// Set blood position for Apartment Mannequin/Flashlight Room
	static bool ValueSet1 = false;
	if (GetRoomID() == 0x17)
	{
		float Value = -3.0f;
		if (*Address1_a != Value || *Address1_b != Value || *Address1_c != Value || *Address1_d != Value)
		{
			UpdateMemoryAddress(Address1_a, &Value, sizeof(float));
			UpdateMemoryAddress(Address1_b, &Value, sizeof(float));
			UpdateMemoryAddress(Address1_c, &Value, sizeof(float));
			UpdateMemoryAddress(Address1_d, &Value, sizeof(float));

			ValueSet1 = true;
		}
	}
	else if (ValueSet1)
	{
		ValueSet1 = false;
	}

	// Set blood position for Flesh Room
	static bool ValueSet2 = false;
	if (GetRoomID() == 0x8A)
	{
		if (!ValueSet2)
		{
			float Value = 0.25f;
			UpdateMemoryAddress(Address2_b, &Value, sizeof(float));
			Value = -3.0f;
			UpdateMemoryAddress(Address2_c, &Value, sizeof(float));
			Value = -1.75f;
			UpdateMemoryAddress(Address2_d, &Value, sizeof(float));

			ValueSet2 = true;
		}
	}
	else if (ValueSet2)
	{
		float Value = -1.0f;
		UpdateMemoryAddress(Address2_b, &Value, sizeof(float));
		UpdateMemoryAddress(Address2_c, &Value, sizeof(float));
		UpdateMemoryAddress(Address2_d, &Value, sizeof(float));

		ValueSet2 = false;
	}
}
