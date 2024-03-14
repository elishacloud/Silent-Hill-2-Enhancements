/**
* Copyright (C) 2024 Elisha Riedlinger
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
#include "Common\Settings.h"
#include "Logging\Logging.h"

// Variables for ASM
BYTE *RedCrossPointer;
void *jmpEnableAddr;
void *jmpDisableAddr;

// Check if is in cutscene
BOOL CheckIfInCutscene()
{
	if (DisableRedCross || *RedCrossPointer)
	{
		return TRUE;
	}
	else if (IsInFullscreenImage)
	{
		return TRUE;
	}
	else if (GetCutsceneID() && GetCutsceneID() != CS_HSP_ROOF_RECOVER)
	{
		return TRUE;
	}
	// Entering inventory screen
	else if (GetInventoryStatus() == 0x00)
	{
		return TRUE;
	}
	// Crossing apartments
	else if (GetRoomID() == R_APT_E_COURTYARD && GetJamesPosZ() == -83662.25f)
	{
		return TRUE;
	}
	// Failed push clock
	else if (GetRoomID() == R_APT_E_RM_208 && GetInGameCameraPosY() == -112.6049728f)
	{
		return TRUE;
	}
	// Apt hole fishing
	else if (GetRoomID() == R_APT_E_RM_202 && GetInGameCameraPosY() == -859.1593018f)
	{
		return TRUE;
	}
	// Apt empty hole
	else if (GetRoomID() == R_APT_E_RM_202 && GetInGameCameraPosY() == -859.1500244f)
	{
		return TRUE;
	}
	// Apt fight water draining
	else if (GetRoomID() == R_APT_W_STAIRCASE_N && GetInGameCameraPosY() == -3961.0f)
	{
		return TRUE;
	}
	// Hospital drain fishing
	else if (GetRoomID() == R_HSP_SHOWER && GetInGameCameraPosY() == -349.463501f)
	{
		return TRUE;
	}
	// James teddy bear
	else if (GetRoomID() == R_HSP_LOCKER_WOMENS && GetInGameCameraPosY() == -1166.53186f)
	{
		return TRUE;
	}
	// James / Maria teddy bear
	else if (GetRoomID() == R_HSP_LOCKER_WOMENS && GetInGameCameraPosY() == -989.6131592f)
	{
		return TRUE;
	}
	// James fridge
	else if (GetRoomID() == R_HSP_ALT_DAY_ROOM && GetInGameCameraPosY() == -1135.397583f)
	{
		return TRUE;
	}
	// James / Maria fridge
	else if (GetRoomID() == R_HSP_ALT_DAY_ROOM && GetInGameCameraPosY() == -1441.904907f)
	{
		return TRUE;
	}
	// Cube head puzzle
	else if (GetRoomID() == R_LAB_TOP_B && GetInGameCameraPosY() == -860.0f)
	{
		return TRUE;
	}
	// Rosewater Park sign
	else if (GetRoomID() == R_TOWN_WEST && GetInGameCameraPosY() == 150.0f && GetJamesPosZ() == 78547.11719f)
	{
		return TRUE;
	}
	// James monologue at end of hospital
	else if (GetRoomID() == R_TOWN_WEST && GetJamesPosZ() == -6000.0f && GetFullscreenImageEvent() == 2)
	{
		return TRUE;
	}
	// Alt Hotel flooded elevator ride
	else if (GetRoomID() == R_HLT_ALT_ELEVATOR && GetJamesPosZ() == -56599.01953f && GetFullscreenImageEvent() == 2)
	{
		return TRUE;
	}

	return FALSE;
}

// ASM functions to disable RedCross during cutscenes
__declspec(naked) void __stdcall RedCrossCutscenesASM()
{
	__asm
	{
		push eax
		push ecx
		push edx
		call CheckIfInCutscene
		cmp eax, FALSE
		pop edx
		pop ecx
		pop eax
		je near EnableHealthIndicator

	//DisableHealthIndicator:
		jmp jmpDisableAddr

	EnableHealthIndicator:
		fld dword ptr ds : [ecx + 0x00000140]
		jmp jmpEnableAddr
	}
}

// Patch SH2 code to disable RedCross during cutscenes
void PatchRedCrossInCutscene()
{
	// Get RedCross address
	constexpr BYTE RedCrossSearchBytes[]{ 0xD9, 0x81, 0x40, 0x01, 0x00, 0x00, 0xD8, 0xA1, 0x3C, 0x01, 0x00, 0x00, 0xD8, 0xB1, 0x40, 0x01, 0x00, 0x00 };
	DWORD RedCrossAddr = SearchAndGetAddresses(0x004762D0, 0x00476570, 0x00476780, RedCrossSearchBytes, sizeof(RedCrossSearchBytes), 0x00, __FUNCTION__);

	// Checking address pointer
	if (!RedCrossAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpEnableAddr = (void*)(RedCrossAddr + 0x06);
	jmpDisableAddr = (void*)(RedCrossAddr + 0xC2);

	// Get cutscene ID address
	GetCutsceneIDPointer();

	// Checking address pointer
	if (!CutsceneIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get cutscene ID address!";
		return;
	}

	// Get memory pointer for RedCross Animation
	constexpr BYTE RedCrossPtrSearchBytes[]{ 0x84, 0xC0, 0x74, 0x1B, 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x4C, 0x24, 0x08, 0xA3 };
	DWORD RedCrossMemoryPtr = SearchAndGetAddresses(0x004EEACE, 0x004EED7E, 0x004EE63E, RedCrossPtrSearchBytes, sizeof(RedCrossPtrSearchBytes), 0x1E2, __FUNCTION__);

	// Checking address pointer
	if (!RedCrossMemoryPtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
		return;
	}
	memcpy(&RedCrossPointer, (void*)(RedCrossMemoryPtr + 1), sizeof(DWORD));

	// Check for valid code before updating
	if (!CheckMemoryAddress(jmpDisableAddr, "\xC7\x05", 2, __FUNCTION__) ||
		!CheckMemoryAddress((void*)RedCrossMemoryPtr, "\xA1", 1, __FUNCTION__))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Update SH2 code
	if (DisableRedCross)
	{
		Logging::Log() << "Disabling Red Cross health indicator...";
	}
	else
	{
		Logging::Log() << "Hiding Red Cross health indicator during cutscenes...";
	}
	WriteJMPtoMemory((BYTE*)RedCrossAddr, *RedCrossCutscenesASM, 6);
}
