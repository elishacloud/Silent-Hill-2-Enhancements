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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Variables for ASM
void *Brightness1;
void *Brightness2;
void *FlashlightExcludeAddr;
void *jmpFlashlightBrightnessReturnAddr;
void *MoveableObject;
void *jmpMovableObject1Addr;
void *jmpMovableObject2Addr;
void *FlashlightValue;
void *jmpFlashlightReachAddr;
void *FlashlightHallwayAddr;
void *jmpFlashlightHallwayReturnAddr;
float ObjectBrightnessValue = 0.7f;

// ASM function to update Flashlight Brightness
__declspec(naked) void __stdcall FlashlightBrightnessASM()
{
	__asm
	{
		push eax
		lea eax, [esi]
		cmp eax, FlashlightExcludeAddr
		jne near Brightness2Label

	// Brightness1Label:
		pop eax
		mov dword ptr ds : [Brightness1], 0x3F99999A		// flashlight brightness; 1.2 float
		fmul dword ptr ds : [Brightness1]
		fstp dword ptr ds : [esp + 0x04]
		fld dword ptr ds : [esi + 0x04]
		fmul dword ptr ds : [Brightness1]
		fstp dword ptr ds : [esp + 0x08]
		fld dword ptr ds : [esi + 0x08]
		fmul dword ptr ds : [Brightness1]
		fstp dword ptr ds : [esp + 0x0C]
		fld dword ptr ds : [esi + 0x0C]
		fmul dword ptr ds : [Brightness1]
		jmp jmpFlashlightBrightnessReturnAddr

	Brightness2Label:
		pop eax
		mov  dword ptr ds : [Brightness2], 0x40800000		// environment brightness; 4.0 float
		fmul dword ptr ds : [Brightness2]
		fstp dword ptr ds : [esp + 0x04]
		fld dword ptr ds : [esi + 0x04]
		fmul dword ptr ds : [Brightness2]
		fstp dword ptr ds : [esp + 0x08]
		fld dword ptr ds : [esi + 0x08]
		fmul dword ptr ds : [Brightness2]
		fstp dword ptr ds : [esp + 0x0C]
		fld dword ptr ds : [esi + 0x0C]
		fmul dword ptr ds : [Brightness2]
		jmp jmpFlashlightBrightnessReturnAddr
	}
}

// ASM function to update Movable Object Preset Part 1
__declspec(naked) void __stdcall MovableObject1ASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [CutsceneIDAddr] 	// moves cutscene ID pointer to eax
		cmp dword ptr ds : [eax], CS_LAURA_PIANO
		je near LauraDrawing						// jumps if Laura drawing in hotel cutscene
		cmp dword ptr ds : [eax], CS_EDDIE_LAURA_BOWLING
		je near HotelBowling						// jumps if bowling Eddie and Laura cutscene
		mov eax, dword ptr ds : [RoomIDAddr]	 	// moves room ID pointer to eax
		cmp dword ptr ds : [eax], 0xB7
		je near HotelBowling						// jumps if Otherworld Hotel Kitchen
		cmp dword ptr ds : [eax], 0xAB
		je near HotelBowling						// jumps if Otherworld Hotel Stairwell

		// for everything else
		mov eax, dword ptr ds : [MoveableObject]
		mov dword ptr ds : [eax], 0x03				// movable object preset
		pop eax
		jmp jmpMovableObject1Addr

		// for Laura drawing in hotel cutscene
	LauraDrawing:
		mov eax, dword ptr ds : [MoveableObject]
		mov dword ptr ds : [eax], 0x02				// movable object preset
		pop eax
		jmp jmpMovableObject1Addr

		// for Otherworld Hotel & bowling Eddie and Laura cutscene
	HotelBowling:
		mov eax, dword ptr ds : [MoveableObject]
		mov dword ptr ds : [eax], 0x04				// movable object preset
		pop eax
		jmp jmpMovableObject1Addr
	}
}

// ASM function to update Movable Object Preset Part 2
__declspec(naked) void __stdcall MovableObject2ASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [CutsceneIDAddr] 	// moves cutscene ID pointer to eax
		cmp dword ptr ds : [eax], CS_LAURA_PIANO
		je near LauraDrawing						// jumps if Laura drawing in hotel cutscene
		cmp dword ptr ds : [eax], CS_EDDIE_LAURA_BOWLING
		je near HotelBowling						// jumps if bowling Eddie and Laura cutscene
		mov eax, dword ptr ds : [RoomIDAddr]	 	// moves room ID pointer to eax
		cmp dword ptr ds : [eax], 0xB7
		je near HotelBowling						// jumps if Otherworld Hotel Kitchen
		cmp dword ptr ds : [eax], 0xAB
		je near HotelBowling						// jumps if Otherworld Hotel Stairwell

		// for everything else
		mov eax, dword ptr ds : [MoveableObject]
		mov dword ptr ds : [eax], 0x03				// movable object preset
		pop eax
		jmp jmpMovableObject2Addr

		// for Laura drawing in hotel cutscene
	LauraDrawing:
		mov eax, dword ptr ds : [MoveableObject]
		mov dword ptr ds : [eax], 0x02				// movable object preset
		pop eax
		jmp jmpMovableObject2Addr

		// for Otherworld Hotel & bowling Eddie and Laura cutscene
	HotelBowling:
		mov eax, dword ptr ds : [MoveableObject]
		mov dword ptr ds : [eax], 0x04				// movable object preset
		pop eax
		jmp jmpMovableObject2Addr
	}
}

// Flashlight Reach
__declspec(naked) void __stdcall FlashlightReachASM()
{
	__asm
	{
		cmp ecx, 0x45BB8000
		jne near FlashlightReachLabel				// jumps if value is not 6000 flt
		mov ecx, 0x4604D000							// flashlight reach; 8500 flt

	FlashlightReachLabel:
		push eax
		mov eax, dword ptr ds : [FlashlightValue]
		mov dword ptr ds : [eax], ecx
		pop eax
		jmp jmpFlashlightReachAddr
	}
}

// Flashlight Hallway
__declspec(naked) void __stdcall FlashlightHallwayASM()
{
	__asm
	{
		push ecx
		mov ecx, dword ptr ds : [RoomIDAddr]	 	// moves room ID pointer to ecx
		cmp dword ptr ds : [ecx], 0x0C
		pop ecx
		je near HeavensNightHallway

	// if not Heaven's Night hallway
		cmp eax, 0x02
		jmp near ExitASM1

	// if Heaven's Night hallway
	HeavensNightHallway:
		cmp eax, 0x03

	ExitASM1:
		jg near ExitASM2
		jmp jmpFlashlightHallwayReturnAddr

	ExitASM2:
		jmp FlashlightHallwayAddr
	}
}

// Scale the inner glow of the flashlight
void RunInnerFlashlightGlow(DWORD Height)
{
	static DWORD LastHeight = 0;
	if (LastHeight == Height)
	{
		return;
	}
	LastHeight = Height;

	static void *Addr1 = nullptr;
	if (!Addr1)
	{
		RUNONCE();

		constexpr BYTE SearchBytes[]{ 0x8D, 0x44, 0x24, 0x28, 0x6A, 0x20, 0x50, 0xE8 };
		Addr1 = (void*)ReadSearchedAddresses(0x00510693, 0x005109C3, 0x005102E3, SearchBytes, sizeof(SearchBytes), 0x11, __FUNCTION__);
		if (!Addr1)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	float InnerGlowSize = (Height / 480.0f) * 11.0f;

	UpdateMemoryAddress(Addr1, &InnerGlowSize, sizeof(float));
}

// Patch SH2 code to enable PS2 flashlight
void PatchPS2Flashlight()
{
	// Get address for Flashlight Brightness ASM
	constexpr BYTE SearchBytesFlashlight[]{ 0x8B, 0xC1, 0x89, 0x4C, 0x24, 0x2C, 0x8B, 0x4C, 0x24, 0x24, 0x89, 0x54, 0x24, 0x30 };
	DWORD FlashlightAddr = SearchAndGetAddresses(0x0047A50B, 0x0047A7AB, 0x0047A9BB, SearchBytesFlashlight, sizeof(SearchBytesFlashlight), 0x33, __FUNCTION__);
	if (!FlashlightAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpFlashlightBrightnessReturnAddr = (void*)(FlashlightAddr + 0x2D);

	// Get address being excluded from Flashlight Brightness ASM
	constexpr BYTE SearchBytesExcluded[]{ 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x4C, 0x24, 0x08, 0x8B, 0x54, 0x24, 0x0C, 0xA3 };
	FlashlightExcludeAddr = (void*)ReadSearchedAddresses(0x00479A70, 0x00479D10, 0x00479F20, SearchBytesExcluded, sizeof(SearchBytesExcluded), 0x0D, __FUNCTION__);
	if (!FlashlightExcludeAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get address for Movable Object Preset Part 1 ASM
	constexpr BYTE SearchBytesMovableObject1[]{ 0xD8, 0x9F, 0x8C, 0x00, 0x00, 0x00, 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x0F, 0x85 };
	DWORD MovableObject1Addr = SearchAndGetAddresses(0x004FF1C6, 0x004FF4F6, 0x004FEE16, SearchBytesMovableObject1, sizeof(SearchBytesMovableObject1), 0x1A, __FUNCTION__);
	if (!MovableObject1Addr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	memcpy(&MoveableObject, (void*)(MovableObject1Addr + 0x02), sizeof(DWORD));
	jmpMovableObject1Addr = (void*)(MovableObject1Addr + 0x0A);
	DWORD ObjectBrightness1Addr = MovableObject1Addr + 0x10;

	// Get address for Movable Object Preset Part 2 ASM
	constexpr BYTE SearchBytesMovableObject2[]{ 0xD8, 0x9F, 0x8C, 0x00, 0x00, 0x00, 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x0F, 0x85 };
	DWORD MovableObject2Addr = SearchAndGetAddresses(0x00502B96, 0x00502EC6, 0x005027E6, SearchBytesMovableObject2, sizeof(SearchBytesMovableObject2), 0x1A, __FUNCTION__);
	if (!MovableObject2Addr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpMovableObject2Addr = (void*)(MovableObject2Addr + 0x0A);
	DWORD ObjectBrightness2Addr = MovableObject2Addr + 0x10;

	// Get address for Flashlight Reach ASM
	constexpr BYTE SearchBytesFlashlightReach[]{ 0x8D, 0x86, 0x80, 0x00, 0x00, 0x00, 0x8B, 0x08, 0x89, 0x0D };
	DWORD FlashlightReachAddr = SearchAndGetAddresses(0x0047B99A, 0x0047BC3A, 0x0047BE4A, SearchBytesFlashlightReach, sizeof(SearchBytesFlashlightReach), 0x1A, __FUNCTION__);
	if (!FlashlightReachAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	memcpy(&FlashlightValue, (void*)(FlashlightReachAddr + 0x02), sizeof(DWORD));
	jmpFlashlightReachAddr = (void*)(FlashlightReachAddr + 0x06);

	// Get address for the brightness of movable objects for specific rooms
	constexpr BYTE SearchBytesSpecificRooms[]{ 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x44, 0x24, 0x20, 0x8B, 0x54, 0x24, 0x24, 0x8B, 0xE9, 0xC1, 0xFD, 0x10, 0x81, 0xE5, 0xFF, 0x0F, 0x00, 0x00, 0x8B, 0x34, 0xAD };
	DWORD SpecificRoomsAddr = ReadSearchedAddresses(0x0047C2EF, 0x0047C58F, 0x0047C79F, SearchBytesSpecificRooms, sizeof(SearchBytesSpecificRooms), 0x1A, __FUNCTION__);
	if (!SpecificRoomsAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get address for the brightness of movable objects for specific rooms 1
	DWORD Address1;
	memcpy(&Address1, (void*)(SpecificRoomsAddr + 0x24), sizeof(DWORD));
	Address1 = Address1 + 0x680;

	// Get address for the brightness of movable objects for specific rooms 2
	DWORD Address2;
	memcpy(&Address2, (void*)(SpecificRoomsAddr + 0x34), sizeof(DWORD));
	Address2 = Address2 + 0x2A0;

	// Get room ID address
	GetRoomIDPointer();

	// Get cutscene ID address
	GetCutsceneIDPointer();

	// Checking address pointer
	if (!RoomIDAddr || !CutsceneIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get room ID or cutscene ID address!";
		return;
	}

	// Fix Heaven's Night hallway
	constexpr BYTE SearchBytesHallway[]{ 0x75, 0x0F, 0x3B, 0xCD, 0x75, 0x14, 0x8B, 0x0D };
	DWORD HeavensNightHallwayAddr = SearchAndGetAddresses(0x004FFCBB, 0x004FFFEB, 0x004FF90B, SearchBytesHallway, sizeof(SearchBytesHallway), 0x11, __FUNCTION__);
	if (!HeavensNightHallwayAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	FlashlightHallwayAddr = (void*)(HeavensNightHallwayAddr + 0xB0);
	jmpFlashlightHallwayReturnAddr = (void*)(HeavensNightHallwayAddr + 0x09);

	// Maximum Movable Object Brightness
	void *ValueAddr = &ObjectBrightnessValue;
	UpdateMemoryAddress((void*)(ObjectBrightness1Addr + 0x02), &ValueAddr, sizeof(void*));
	UpdateMemoryAddress((void*)(ObjectBrightness2Addr + 0x02), &ValueAddr, sizeof(void*));

	// Woodside Apartment Room 205
	float Value = 0.0f;
	UpdateMemoryAddress((void*)(Address1 + 0x00), &Value, sizeof(float));		// Movable Object Brightness (Red)
	UpdateMemoryAddress((void*)(Address1 + 0x04), &Value, sizeof(float));		// Movable Object Brightness (Green)
	UpdateMemoryAddress((void*)(Address1 + 0x08), &Value, sizeof(float));		// Movable Object Brightness (Blue)

	// Otherworld Hotel Kitchen
	Value = 0.0f;
	UpdateMemoryAddress((void*)(Address2 + 0x00), &Value, sizeof(float));		// Movable Object Brightness (Red)
	UpdateMemoryAddress((void*)(Address2 + 0x04), &Value, sizeof(float));		// Movable Object Brightness (Green)
	UpdateMemoryAddress((void*)(Address2 + 0x08), &Value, sizeof(float));		// Movable Object Brightness (Blue)

	// Update SH2 code
	Logging::Log() << "Enabling PS2 Flashlight Fix...";
	WriteJMPtoMemory((BYTE*)FlashlightAddr, *FlashlightBrightnessASM, 6);
	WriteJMPtoMemory((BYTE*)MovableObject1Addr, *MovableObject1ASM, 10);
	WriteJMPtoMemory((BYTE*)MovableObject2Addr, *MovableObject2ASM, 10);
	WriteJMPtoMemory((BYTE*)FlashlightReachAddr, *FlashlightReachASM, 6);
	WriteJMPtoMemory((BYTE*)HeavensNightHallwayAddr, *FlashlightHallwayASM, 9);
}
