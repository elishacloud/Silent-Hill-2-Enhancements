/**
* Copyright (C) 2023 Elisha Riedlinger, mercury501
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
#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\FileSystemHooks.h"
#include "Logging\Logging.h"

HRESULT PlayWavFile(const char*);

int32_t PlaySaveSound_Hook(int32_t SoundId, float volume, DWORD param3)
{
	UNREFERENCED_PARAMETER(SoundId);
	UNREFERENCED_PARAMETER(volume);
	UNREFERENCED_PARAMETER(param3);

	Logging::Log() << __FUNCTION__;
	PlayWavFile((std::string(GetModPath("")) + "\\sound\\extra\\save_sound.wav").c_str());
	return 0x10;	// PlaySound function success
}

int32_t PlayLoadSound_Hook(int32_t SoundId, float volume, DWORD param3)
{
	UNREFERENCED_PARAMETER(SoundId);
	UNREFERENCED_PARAMETER(volume);
	UNREFERENCED_PARAMETER(param3);

	Logging::Log() << __FUNCTION__;
	PlayWavFile((std::string(GetModPath("")) + "\\sound\\extra\\g_start.wav").c_str());
	return 0x10;	// PlaySound function success
}

void PatchCustomSFXs()
{
	if (GameVersion != SH2V_10)
	{
		return;
	}

	Logging::Log() << "Patching custom SFXs...";

	uint32_t* LoadGameSoundPauseMenu = (uint32_t*)0x00454A64; //TODO addresses
	uint32_t* LoadGameSoundNewGame = (uint32_t*)0x0049870c;
	uint32_t* LoadGameSoundContinue = (uint32_t*)0x00497f84;
	uint32_t* SaveGameSoundRedSquares = (uint32_t*)0x004476DB;

	// Hook load/save game sounds
	injector::MakeCALL(LoadGameSoundPauseMenu, PlayLoadSound_Hook, true);
	injector::MakeCALL(LoadGameSoundNewGame, PlayLoadSound_Hook, true);
	injector::MakeCALL(LoadGameSoundContinue, PlayLoadSound_Hook, true);
	injector::MakeCALL(SaveGameSoundRedSquares, PlaySaveSound_Hook, true);
}

void RunPlayAdditionalSounds()
{
	static BYTE* CanUseFlashlight1 =
		GameVersion == SH2V_10 ? (BYTE*)0x01F7DC2C :
		GameVersion == SH2V_11 ? (BYTE*)0x01F8182C :
		GameVersion == SH2V_DC ? (BYTE*)0x01F8082C : nullptr;

	static BYTE* CanUseFlashlight2 =
		GameVersion == SH2V_10 ? (BYTE*)0x01F7AE80 :
		GameVersion == SH2V_11 ? (BYTE*)0x01F7EA80 :
		GameVersion == SH2V_DC ? (BYTE*)0x01F7DA80 : nullptr;

	static BYTE* WorldColorR =
		GameVersion == SH2V_10 ? (BYTE*)0x00942C50 :
		GameVersion == SH2V_11 ? (BYTE*)0x00946850 :
		GameVersion == SH2V_DC ? (BYTE*)0x00945850 : nullptr;

	static BYTE* WorldColorG =
		GameVersion == SH2V_10 ? (BYTE*)0x00942C51 :
		GameVersion == SH2V_11 ? (BYTE*)0x00946851 :
		GameVersion == SH2V_DC ? (BYTE*)0x00945851 : nullptr;

	static BYTE* WorldColorB =
		GameVersion == SH2V_10 ? (BYTE*)0x00942C52 :
		GameVersion == SH2V_11 ? (BYTE*)0x00946852 :
		GameVersion == SH2V_DC ? (BYTE*)0x00945852 : nullptr;

	static BYTE* InventoryItem =
		GameVersion == SH2V_10 ? (BYTE*)0x01F7A7E2 :
		GameVersion == SH2V_11 ? (BYTE*)0x01F7E3E2 :
		GameVersion == SH2V_DC ? (BYTE*)0x01F7D3E2 : nullptr;

	DWORD RoomID = GetRoomID();
	static DWORD LastRoomID = RoomID;
	BYTE FlashLightSwitch = GetFlashlightSwitch();
	static BYTE LastFlashLightSwitch = FlashLightSwitch;

	bool RoomsToNeverPlaySFX = (
		RoomID == 0x24 /*Angela apt room*/ ||
		RoomID == 0x89 /*Maria in prison*/ ||
		RoomID == 0x8F /*Eddie boss room 1*/ ||
		RoomID == 0x90 /*Eddie boss room 2*/);
	bool RoomsWithNoExtraCriteria = (
		RoomID == 0x65 /*Prison bug room*/ ||
		RoomID == 0xAA /*Hotel Alternate Angela Staircase*/ ||
		RoomID == 0xAB /*Hotel Alternate Employee Staircase*/ ||
		RoomID == 0xAC /*Final RPT Boss Fight Room*/ ||
		RoomID == 0xAD /*Hotel Alternate Back Hallway*/ ||
		RoomID == 0xAE /*Hotel Alternate Burned Employee Area*/ ||
		RoomID == 0xAF /*Final Nine Save Squares*/ ||
		RoomID == 0xB0 /*Hotel Alternate Reading Room*/ ||
		RoomID == 0xB1 /*Hotel Alternate 2F W Rooms Hallway*/ ||
		RoomID == 0xB2 /*Hotel Alternate W 2F Hallway*/ ||
		RoomID == 0xB3 /*Hotel Alternate E 2F Hallway*/ ||
		RoomID == 0xB4 /*Hotel Alternate 2F E Rooms Hallway*/ ||
		RoomID == 0xB5 /*Hotel Alternate 3F Hallway*/ ||
		RoomID == 0xB6 /*Hotel Alternate Bar*/ ||
		RoomID == 0xB7 /*Hotel Alternate Bar Kitchen*/ ||
		RoomID == 0xB8 /*Hotel Alternate Bar Entrance*/ ||
		RoomID == 0xB9 /*Hotel Alternate 1F Employee Hallway*/ ||
		RoomID == 0xBA /*Final Hallway Before End*/ ||
		RoomID == 0xBB /*Final Boss Room*/);

	// Check for boss level
	static bool IsBossLevel = false;
	if (RoomID == 0xBB /*Final boss room*/ && GetCutsceneID() /*any cutscene triggered*/)
	{
		IsBossLevel = true;
	}
	else if (IsBossLevel && RoomID != 0xBB)
	{
		IsBossLevel = false;
	}

	// Check for flashlight on/off
	if (FlashLightSwitch != LastFlashLightSwitch && RoomID == LastRoomID && !RoomsToNeverPlaySFX && !IsBossLevel && (
		(RoomsWithNoExtraCriteria) ||
		(*CanUseFlashlight1 == 1 && *CanUseFlashlight2 == 0 && !RoomsWithNoExtraCriteria && RoomID != 0xA2 /*Hotel room 302*/) ||
		((RoomID == 0x04 /*Town East*/ || RoomID == 0x08 /*Town West*/) && (*WorldColorR == 0 && *WorldColorG == 0 && *WorldColorB == 0)) ||
		(RoomID == 0xA2 /*Hotel room 302*/ && *InventoryItem < 0x80 /*VHSTape is NOT in player's inventory*/)))
	{
		// play flashlight_off.wav
		if (FlashLightSwitch == 0)
		{
			PlayWavFile((std::string(GetModPath("")) + "\\sound\\extra\\flashlight_off.wav").c_str());
		}
		// play flashlight_on.wav
		else if (FlashLightSwitch == 1)
		{
			PlayWavFile((std::string(GetModPath("")) + "\\sound\\extra\\flashlight_on.wav").c_str());
		}
	}
	LastRoomID = RoomID;
	LastFlashLightSwitch = FlashLightSwitch;
}
