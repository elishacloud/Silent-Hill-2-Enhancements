/**
* Copyright (C) 2024 Elisha Riedlinger, mercury501
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
#include <shlwapi.h>
#include "Common\Utils.h"
#include "Common\FileSystemHooks.h"
#include "Logging\Logging.h"

void* GameSoundReturnAddress = nullptr;

HRESULT PlayWavFile(const char* filePath, DWORD BifferID);

// Function instance created
typedef int32_t(WINAPIV* PlaySoundProc)(int32_t SoundId, float volume, DWORD param3);
PlaySoundProc m_pPlaySound = nullptr;

const char* SaveGameWav = "\\sound\\extra\\save_sound.wav";
const char* LoadGameWav = "\\sound\\extra\\g_start.wav";
const char* FlashLightOnWav = "\\sound\\extra\\flashlight_on.wav";
const char* FlashLightOffWav = "\\sound\\extra\\flashlight_off.wav";

int32_t PlaySaveSound(int32_t SoundId, float volume, DWORD param3)
{
	UNREFERENCED_PARAMETER(SoundId);
	UNREFERENCED_PARAMETER(volume);
	UNREFERENCED_PARAMETER(param3);

	PlayWavFile((std::string(GetModPath("")) + SaveGameWav).c_str(), 0);

	return 0x10;	// PlaySound function success
}

int32_t PlayLoadSound(int32_t SoundId, float volume, DWORD param3)
{
	UNREFERENCED_PARAMETER(SoundId);
	UNREFERENCED_PARAMETER(volume);
	UNREFERENCED_PARAMETER(param3);

	PlayWavFile((std::string(GetModPath("")) + LoadGameWav).c_str(), 1);

	return 0x10;	// PlaySound function success
}

__declspec(naked) void __stdcall SaveGameSoundASM()
{
	__asm
	{
		push eax
		mov eax, PauseMenuButtonIndexAddr
		cmp [eax], 0x1
		jne real_code
		push esi
		push 0x3F800000
		push 0x0002743
		call PlaySaveSound
		pop eax
		jmp GameSoundReturnAddress
	real_code:
		push esi
		push 0x3F800000
		push 0x0002712
		call m_pPlaySound
		pop eax
		jmp GameSoundReturnAddress
	}
}

void PatchCustomSFXs()
{
	Logging::Log() << "Patching custom save and load SFXs...";

	// Calls this function for the getting PauseMenuButtonIndexAddr
	GetPauseMenuButtonIndex();

	// Get Pause Menu sound function address
	constexpr BYTE SearchPauseMenuBytes[]{ 0x6A, 0x00, 0x68, 0x00, 0x00, 0x80, 0x3F, 0x68, 0x9A, 0x3A, 0x00, 0x00, 0xE8 };
	BYTE* LoadGameSoundPauseMenu = (BYTE*)SearchAndGetAddresses(0x00454A58, 0x00454CB8, 0x00454CB8, SearchPauseMenuBytes, sizeof(SearchPauseMenuBytes), 0x0C, __FUNCTION__);

	// Get Load Game sound function address
	constexpr BYTE SearchLoadGameBytes[]{ 0xEB, 0x3F, 0x3B, 0xF3, 0x75, 0x21, 0x3B, 0xFB, 0x75, 0x1D, 0x3B, 0xEB, 0x75, 0x19, 0x3B, 0xC3, 0x75, 0x15, 0x53, 0x68, 0x00, 0x00, 0x80, 0x3F, 0x68, 0x9A, 0x3A, 0x00, 0x00, 0xE8 };
	BYTE* LoadGameSoundContinue = (BYTE*)SearchAndGetAddresses(0x00497F67, 0x00498217, 0x004978A7, SearchLoadGameBytes, sizeof(SearchLoadGameBytes), 0x1D, __FUNCTION__);

	// Get Save Game sound function address
	constexpr BYTE SearchSaveGameBytes[]{ 0x83, 0xC4, 0x04, 0x85, 0xC0, 0x74, 0x79, 0x56, 0x68, 0x00, 0x00, 0x80, 0x3F, 0x68, 0x43, 0x27, 0x00, 0x00, 0xE8 };
	BYTE* SaveGameSoundRedSquares = (BYTE*)SearchAndGetAddresses(0x004476C9, 0x00447869, 0x00447869, SearchSaveGameBytes, sizeof(SearchSaveGameBytes), 0x12, __FUNCTION__);

	// Play Sound Effect function address
	constexpr BYTE SearchPlayMusicBytes[]{ 0x6A, 0x01, 0xE8, 0x19, 0x90, 0xFE, 0xFF, 0x83, 0xC4, 0x04 };
	m_pPlaySound = (PlaySoundProc)(CheckMultiMemoryAddress((void*)0x00515580, (void*)0x005158B0, (void*)0x005151D0, (void*)SearchPlayMusicBytes, sizeof(SearchPlayMusicBytes), __FUNCTION__));

	BYTE* LoadGameSoundNewGame = (BYTE*)(
		GameVersion == SH2V_10 ? 0x0049870C :
		GameVersion == SH2V_11 ? 0x004989BC :
		GameVersion == SH2V_DC ? 0x00498201 : NULL);

	// Checking address pointer
	if (!PauseMenuButtonIndexAddr || !LoadGameSoundPauseMenu || !LoadGameSoundContinue || !SaveGameSoundRedSquares || !m_pPlaySound ||
		!LoadGameSoundNewGame || *LoadGameSoundNewGame != 0xE8)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Sound effect function address, same address for all known versions of the game
	BYTE* SoundEffectCallAddr = (BYTE*)0x00402823;
	GameSoundReturnAddress = reinterpret_cast<void*>(SoundEffectCallAddr + 16);

	// Update SH2 code
	if (PathFileExistsA((std::string(GetModPath("")) + LoadGameWav).c_str()))
	{
		WriteCalltoMemory(LoadGameSoundPauseMenu, PlayLoadSound);
		WriteCalltoMemory(LoadGameSoundNewGame, PlayLoadSound);
		WriteCalltoMemory(LoadGameSoundContinue, PlayLoadSound);
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: could not find Load Game WAV file: " << LoadGameWav;
	}
	if (PathFileExistsA((std::string(GetModPath("")) + SaveGameWav).c_str()))
	{
		WriteJMPtoMemory(SoundEffectCallAddr, *SaveGameSoundASM, 16);
		WriteCalltoMemory(SaveGameSoundRedSquares, PlaySaveSound);
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: could not find Save Game WAV file: " << SaveGameWav;
	}
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

	// Checking address pointer
	if (!CanUseFlashlight1 || !CanUseFlashlight2)
	{
		LOG_ONCE(__FUNCTION__ " Error: failed to find memory address!");
		return;
	}

	static DWORD LastRoomID = 0;
	DWORD RoomID = GetRoomID();
	static BYTE LastFlashLightSwitch = 0;
	BYTE FlashLightSwitch = GetFlashlightSwitch();

	bool RoomsToNeverPlaySFX = (
		RoomID == R_APT_W_RM_109_2 ||
		RoomID == R_LAB_BOTTOM_D ||
		RoomID == R_EDI_BOSS_RM_1 ||
		RoomID == R_EDI_BOSS_RM_2);
	bool RoomsWithNoExtraCriteria = (
		RoomID == R_BUG_RM ||
		RoomID == R_HTL_FIRE_STAIRCASE ||
		RoomID == R_HTL_ALT_EMPLOYEE_STAIRS ||
		RoomID == R_HTL_ALT_RPT_BOSS_RM ||
		RoomID == R_HTL_ALT_MAIN_HALL_1F ||
		RoomID == R_HTL_ALT_EMPLOYEE_HALL_1F ||
		RoomID == R_HTL_ALT_NINE_SAVE_RM ||
		RoomID == R_HTL_ALT_READING_RM ||
		RoomID == R_HTL_ALT_W_ROOM_HALL_2F ||
		RoomID == R_HTL_ALT_W_HALL_2F ||
		RoomID == R_HTL_ALT_E_HALL_2F ||
		RoomID == R_HTL_ALT_E_ROOM_HALL_2F ||
		RoomID == R_HTL_ALT_MAIN_HALL_3F ||
		RoomID == R_HTL_ALT_BAR ||
		RoomID == R_HTL_ALT_BAR_KITCHEN ||
		RoomID == R_HTL_ALT_ELEVATOR ||
		RoomID == R_HTL_ALT_EMPLOYEE_HALL_BF ||
		RoomID == R_HTL_ALT_FINAL_HALL ||
		RoomID == R_FINAL_BOSS_RM);

	// Check for boss level
	static bool IsBossLevel = false;
	if (RoomID == R_FINAL_BOSS_RM && GetCutsceneID() != CS_NONE)
	{
		IsBossLevel = true;
	}
	else if (IsBossLevel && RoomID != R_FINAL_BOSS_RM)
	{
		IsBossLevel = false;
	}

	// Check for flashlight on/off
	if (FlashLightSwitch != LastFlashLightSwitch && RoomID == LastRoomID && !RoomsToNeverPlaySFX && !IsBossLevel && (
		(RoomsWithNoExtraCriteria) ||
		(*CanUseFlashlight1 == 1 && *CanUseFlashlight2 == 0 && !RoomsWithNoExtraCriteria && RoomID != R_HTL_RM_312) ||
		((RoomID == R_TOWN_EAST || RoomID == R_TOWN_WEST) && (GetWorldColorR() == 0 && GetWorldColorG() == 0 && GetWorldColorB() == 0)) ||
		(RoomID == R_HTL_RM_312 && GetInventoryItem() < 0x80 /*VHSTape is NOT in player's inventory*/)))
	{
		// play flashlight_off.wav
		if (FlashLightSwitch == 0)
		{
			PlayWavFile((std::string(GetModPath("")) + FlashLightOffWav).c_str(), 2);
		}
		// play flashlight_on.wav
		else if (FlashLightSwitch == 1)
		{
			PlayWavFile((std::string(GetModPath("")) + FlashLightOnWav).c_str(), 2);
		}
	}
	LastRoomID = RoomID;
	LastFlashLightSwitch = FlashLightSwitch;
}
