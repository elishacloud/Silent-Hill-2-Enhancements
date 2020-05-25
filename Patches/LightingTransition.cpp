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

// Variables for ASM
void *jmpLightingTransitionReturnAddr;
DWORD LightingTransitionObject;

// ASM function to update Lighting Transition
__declspec(naked) void __stdcall LightingTransitionASM()
{
	__asm
	{
		push eax
		push edx
		mov eax, dword ptr ds : [RoomIDAddr]		// moves room ID pointer to eax
		cmp dword ptr ds : [eax], 0x27				// Blue Creek Apt Room 203
		je near BlueCreekAptHotel
		cmp dword ptr ds : [eax], 0xA0				// Hotel 2F West Hallway
		je near BlueCreekAptHotel
		cmp dword ptr ds : [eax], 0xB2				// Otherworld Hotel 2F West Hallway
		je near BlueCreekAptHotel
		cmp dword ptr ds : [eax], 0xB1				// Otherworld Hotel 2F West Room Hallway
		je near BlueCreekAptHotel
		cmp dword ptr ds : [eax], 0xB4				// Otherworld Hotel 2F East Room Hallway
		je near BlueCreekAptHotel
		cmp dword ptr ds : [eax], 0xAB				// Otherworld Hotel Stairwell
		je near BlueCreekAptHotel
		cmp dword ptr ds : [eax], 0xD0				// Baldwin Mansion 2F Hallway
		je near BaldwinMansion2FHallway

		// if none of these locations
		mov [esi], 0x03
		jmp near ExitASM

		// if Baldwin Mansion 2F Hallway
	BaldwinMansion2FHallway:
		mov [esi], 0x02
		jmp near ExitASM

		// if Blue Creek Apt or Hotel
	BlueCreekAptHotel:
		lea edx, [esi]
		cmp edx, LightingTransitionObject			// determines if James or other object
		je near OtherObjects

		// if James
		mov [esi], 0x02
		jmp near ExitASM

		// if other object
	OtherObjects:
		mov [esi], 0x03

	ExitASM:
		pop edx
		pop eax
		jmp jmpLightingTransitionReturnAddr
	}
}

// Update SH2 code to enable Lighting Transition fix
void SetLightingTransition()
{
	// Get Lighting Transition address
	constexpr BYTE SearchBytesLightingTransition[]{ 0x8B, 0x11, 0x89, 0x10, 0x8B, 0x51, 0x04, 0x89, 0x50, 0x04, 0x8B, 0x51, 0x08, 0x89, 0x50, 0x08, 0x8B, 0x54, 0x24, 0x18, 0x8B, 0x49, 0x0C, 0x52, 0x53, 0x89, 0x48, 0x0C, 0xE8, 0xFF, 0xEF, 0xFF, 0xFF };
	DWORD LightingTransitionAddr = SearchAndGetAddresses(0x0050D6F0, 0x0050DA20, 0x0050D340, SearchBytesLightingTransition, sizeof(SearchBytesLightingTransition), -0x0C);
	if (!LightingTransitionAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpLightingTransitionReturnAddr = (void*)(LightingTransitionAddr + 0x06);

	// Get Object address
	LightingTransitionObject = ReadSearchedAddresses(0x0050D6F0, 0x0050DA20, 0x0050D340, SearchBytesLightingTransition, sizeof(SearchBytesLightingTransition), -0x20);
	if (!LightingTransitionObject)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get room ID address
	RoomIDAddr = GetRoomIDPointer();

	// Checking address pointer
	if (!RoomIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get room ID address!";
		return;
	}

	// Get address for the brightness of movable objects for specific rooms
	constexpr BYTE SearchBytesSpecificRooms[]{ 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x44, 0x24, 0x20, 0x8B, 0x54, 0x24, 0x24, 0x8B, 0xE9, 0xC1, 0xFD, 0x10, 0x81, 0xE5, 0xFF, 0x0F, 0x00, 0x00, 0x8B, 0x34, 0xAD };
	DWORD SpecificRoomsAddr = ReadSearchedAddresses(0x0047C2EF, 0x0047C58F, 0x0047C79F, SearchBytesSpecificRooms, sizeof(SearchBytesSpecificRooms), 0x1A);
	if (!SpecificRoomsAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get address for the brightness of movable objects for specific rooms 1
	DWORD Address1;
	memcpy(&Address1, (void*)(SpecificRoomsAddr + 0x24), sizeof(DWORD));
	Address1 = Address1 + 0x1030;

	// Get address for the brightness of movable objects for specific rooms 2
	DWORD Address2;
	memcpy(&Address2, (void*)(SpecificRoomsAddr + 0x40), sizeof(DWORD));
	Address2 = Address2 + 0x490;

	// Get address for the brightness of movable objects for specific rooms 3
	DWORD Address3;
	memcpy(&Address3, (void*)(SpecificRoomsAddr + 0x30), sizeof(DWORD));
	DWORD Address4 = Address3 + 0x2390;
	Address3 = Address3 + 0x2A0;

	// Blue Creek Apt Room 203
	float Value = 1.0f;
	UpdateMemoryAddress((void*)(Address1 + 0x00), &Value, sizeof(float));		// Movable Object Brightness (Red)
	UpdateMemoryAddress((void*)(Address1 + 0x04), &Value, sizeof(float));		// Movable Object Brightness (Green)
	UpdateMemoryAddress((void*)(Address1 + 0x08), &Value, sizeof(float));		// Movable Object Brightness (Blue)

	// Baldwin Mansion 2F Hallway
	Value = 1.0f;
	UpdateMemoryAddress((void*)(Address2 + 0x00), &Value, sizeof(float));		// Movable Object Brightness (Red)
	UpdateMemoryAddress((void*)(Address2 + 0x04), &Value, sizeof(float));		// Movable Object Brightness (Green)
	UpdateMemoryAddress((void*)(Address2 + 0x08), &Value, sizeof(float));		// Movable Object Brightness (Blue)

	// Normal Hotel Kitchen
	Value = 0.8f;
	UpdateMemoryAddress((void*)(Address3 + 0x00), &Value, sizeof(float));		// Movable Object Brightness (Red)
	UpdateMemoryAddress((void*)(Address3 + 0x04), &Value, sizeof(float));		// Movable Object Brightness (Green)
	UpdateMemoryAddress((void*)(Address3 + 0x08), &Value, sizeof(float));		// Movable Object Brightness (Blue)

	// Hotel 202/204
	Value = 1.0f;
	UpdateMemoryAddress((void*)(Address4 + 0x00), &Value, sizeof(float));
	UpdateMemoryAddress((void*)(Address4 + 0x04), &Value, sizeof(float));
	UpdateMemoryAddress((void*)(Address4 + 0x08), &Value, sizeof(float));
	Value = 500.0f;
	UpdateMemoryAddress((void*)(Address4 + 0x18), &Value, sizeof(float));
	UpdateMemoryAddress((void*)(Address4 + 0x1C), &Value, sizeof(float));

	// Update SH2 code
	Logging::Log() << "Enabling Lighting Transition Fix...";
	WriteJMPtoMemory((BYTE*)LightingTransitionAddr, *LightingTransitionASM, 6);
}

void RunLightingTransition()
{
	// Update SH2 code to enable Lighting Transition fix
	RUNCODEONCE(SetLightingTransition());

	// Get flashlight render address
	FlashLightRenderAddr = GetFlashLightRenderPointer();

	// Checking address pointer
	if (!FlashLightRenderAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get flashlight render address!";
		return;
	}

	// Set flashlight render
	static bool ValueSet = false;
	static DWORD Counter = 0;
	if (GetCutsceneID() == 0x5C)
	{
		if (*FlashLightRenderAddr != 0 && ++Counter < 3)
		{
			*FlashLightRenderAddr = 0;
			ValueSet = true;
		}
	}
	else if (GetCutsceneID() == 0x19)
	{
		if (!ValueSet)
		{
			*FlashLightRenderAddr = 0;
			ValueSet = true;
		}
	}
	else if (ValueSet)
	{
		ValueSet = false;
		Counter = 0;
	}
}
