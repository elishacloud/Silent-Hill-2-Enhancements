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

BYTE AllowQuickSaveFlag = TRUE;
void *QuickSaveCmpAddr;
void *jmpQuickSaveAddr;

// ASM functions to disable quick saves
__declspec(naked) void __stdcall QuickSaveASM()
{
	__asm
	{
		pushf
		cmp byte ptr ds : [AllowQuickSaveFlag], TRUE
		je near AllowQuickSave
	//DisallowQuickSave:
		popf
		jmp near Exit
	AllowQuickSave:
		popf
		mov eax, dword ptr ds : [QuickSaveCmpAddr]
		cmp dword ptr ds : [eax], esi
	Exit:
		jmp jmpQuickSaveAddr
	}
}

void SetGameLoad()
{
	// Get elevator room save address
	constexpr BYTE GameLoadSearchBytes[]{ 0x83, 0xC4, 0x10, 0xF7, 0xC1, 0x00, 0x00, 0x00, 0x04, 0x5E, 0x74, 0x0F, 0xC7, 0x05 };
	DWORD GameLoadAddr = SearchAndGetAddresses(0x0058312C, 0x005839DC, 0x005832FC, GameLoadSearchBytes, sizeof(GameLoadSearchBytes), 0x94);
	if (!GameLoadAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	constexpr BYTE QuickSaveSearchBytes[]{ 0x83, 0xC4, 0x04, 0x85, 0xC0, 0x74, 0x4C, 0x39, 0x35 };
	DWORD QuickSaveFunction = SearchAndGetAddresses(0x00402495, 0x00402495, 0x00402495, QuickSaveSearchBytes, sizeof(QuickSaveSearchBytes), 0x07);
	if (!QuickSaveFunction)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpQuickSaveAddr = (void*)(QuickSaveFunction + 0x06);
	QuickSaveCmpAddr = (void*)*(DWORD*)(QuickSaveFunction + 0x02);

	// Update SH2 code
	Logging::Log() << "Enabling Load Game Fix...";
	DWORD Value = 0x00;
	UpdateMemoryAddress((void*)GameLoadAddr, &Value, sizeof(DWORD));
	WriteJMPtoMemory((BYTE*)QuickSaveFunction, *QuickSaveASM, 6);
}

void UpdateGameLoad(DWORD *SH2_RoomID, float *SH2_JamesPosX, float *SH2_JamesPosZ)
{
	// Update save code elevator room
	RUNCODEONCE(SetGameLoad());

	// Get game save address
	static BYTE *SaveGameAddress = nullptr;
	if (!SaveGameAddress)
	{
		RUNONCE();

		// Get address for game save
		constexpr BYTE SearchBytes[]{ 0x3C, 0x1B, 0x74, 0x27, 0x3C, 0x25, 0x74, 0x23, 0x3C, 0x30, 0x74, 0x1F, 0x3C, 0x31, 0x74, 0x1B, 0x3C, 0x32, 0x74, 0x17, 0x3C, 0x33, 0x74, 0x13, 0x3C, 0x34, 0x74, 0x0F };
		SaveGameAddress = (BYTE*)ReadSearchedAddresses(0x0044C648, 0x0044C7E8, 0x0044C7E8, SearchBytes, sizeof(SearchBytes), -0x0D);
		if (!SaveGameAddress)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	// Get elevator running address
	static BYTE *ElevatorRunning = nullptr;
	if (!ElevatorRunning)
	{
		RUNONCE();

		// Get address for game save
		constexpr BYTE SearchBytes[]{ 0xF7, 0xC6, 0x00, 0x0C, 0x00, 0x00, 0x0F, 0x95, 0xC0, 0x49, 0x74, 0x0A, 0x49, 0x75, 0x25, 0x84, 0xC0 };
		ElevatorRunning = (BYTE*)ReadSearchedAddresses(0x0052EA81, 0x0052EDB1, 0x0052E6D1, SearchBytes, sizeof(SearchBytes), -0x0E);
		if (!ElevatorRunning)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	// Get in-game voice event address
	static BYTE *InGameVoiceEvent = nullptr;
	if (!InGameVoiceEvent)
	{
		RUNONCE();

		// Get address for game save
		constexpr BYTE SearchBytes[]{ 0xB9, 0xA0, 0x02, 0x00, 0x00, 0x33, 0xC0, 0xBF };
		InGameVoiceEvent = (BYTE*)ReadSearchedAddresses(0x00563CB4, 0x00562804, 0x00562124, SearchBytes, sizeof(SearchBytes), 0x08);
		if (!InGameVoiceEvent)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}

		InGameVoiceEvent = (BYTE*)((DWORD)InGameVoiceEvent + 0x90);
	}

	// Get full screen image event address
	static BYTE *FullscreenImageEvent = nullptr;
	if (!FullscreenImageEvent)
	{
		RUNONCE();

		// Get address for game save
		constexpr BYTE SearchBytes[]{ 0x90, 0x90, 0x8B, 0x44, 0x24, 0x04, 0x83, 0xC0, 0xFE, 0x83, 0xF8, 0x06, 0x77, 0x61, 0xFF, 0x24, 0x85 };
		FullscreenImageEvent = (BYTE*)ReadSearchedAddresses(0x0052E25E, 0x0052E58E, 0x0052DEAE, SearchBytes, sizeof(SearchBytes), 0x1F);
		if (!FullscreenImageEvent)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}

		FullscreenImageEvent = (BYTE*)((DWORD)FullscreenImageEvent + 0x14);
	}

	// Set static variables
	static bool ValueSet = false;
	static bool ValueUnSet = false;

	bool DisableQuickSave = false;

	// Enable game saves for specific rooms
	if (*SH2_RoomID == 0x29)
	{
		*SaveGameAddress = 1;
		ValueSet = true;
	}
	// Disable game saves for specific rooms
	else if (*SH2_RoomID == 0x13 || *SH2_RoomID == 0x17 || *SH2_RoomID == 0xAA || *SH2_RoomID == 0xC7 ||
		(*SH2_RoomID == 0x78 && *SH2_JamesPosX < -18600.0f) ||
		(*SH2_RoomID == 0x04 && *SH2_JamesPosZ > 49000.0f))
	{
		*SaveGameAddress = 0;
		ValueUnSet = true;
	}
	// Disable game saves for specific rooms and disable quick save if the Elevator is not running or there is in-game voice event happening
	else if (*SH2_RoomID == 0x2A || *SH2_RoomID == 0x46 ||
		(*SH2_RoomID == 0x9D && *SH2_JamesPosX < 60650.0f) ||
		(*SH2_RoomID == 0xB8 && *SH2_JamesPosX > -15800.0f))
	{
		*SaveGameAddress = 0;
		ValueUnSet = true;

		// Disable game saves for specific rooms and disable quick save if the Elevator is running or there is in-game voice event happening
		if (*ElevatorRunning == 0 || *InGameVoiceEvent == 1)
		{
			DisableQuickSave = true;
			AllowQuickSaveFlag = FALSE;
		}
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
			*SaveGameAddress = 1;
			ValueUnSet = false;
		}
	}

	// Disable quick save during certian in-game voice events and during fullscreen image events
	if ((((*SH2_RoomID == 0x0A) || (*SH2_RoomID == 0xBA)) && *InGameVoiceEvent == 1) ||
		*FullscreenImageEvent == 0)
	{
		DisableQuickSave = true;
		AllowQuickSaveFlag = FALSE;
	}

	// Reset quick save when needed
	if (!DisableQuickSave && !AllowQuickSaveFlag)
	{
		AllowQuickSaveFlag = TRUE;
	}
}
