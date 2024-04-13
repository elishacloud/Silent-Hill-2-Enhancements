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
#include "Logging\Logging.h"

// Variables for ASM
void *NewEnvFogRGB;
void *OriginalEnvFogRGB;
void *jmpFinalAreaBossAddr1;
void *jmpFinalAreaBossAddr2;

// Fog values
constexpr float BlueCreekNewFog = 4200.0f;
constexpr float BlueCreekOriginalFog = 3000.0f;
constexpr float FinalAreaCameraYOrientation = -15750.0f;

// ASM final boss area 1
__declspec(naked) void __stdcall FinalAreaBoss1ASM()
{
	__asm
	{
		mov eax, dword ptr ds : [RoomIDAddr]
		cmp dword ptr ds : [eax], R_FINAL_BOSS_RM
		jne near NotFinalBoss
		mov eax, dword ptr ds : [InGameCameraPosYAddr]
		movss xmm0, dword ptr ds : [FinalAreaCameraYOrientation]
		comiss xmm0, dword ptr ds : [eax]
		jbe near NotFinalBoss
		mov eax, dword ptr ds : [NewEnvFogRGB]
		jmp near ExitASM

	NotFinalBoss:
		mov eax, dword ptr ds : [OriginalEnvFogRGB]

	ExitASM:
		mov eax, dword ptr ds : [eax]
		jmp jmpFinalAreaBossAddr1
	}
}

// ASM final boss area 2
__declspec(naked) void __stdcall FinalAreaBoss2ASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [RoomIDAddr]
		cmp dword ptr ds : [eax], R_FINAL_BOSS_RM
		jne near NotFinalBoss
		mov eax, dword ptr ds : [InGameCameraPosYAddr]
		movss xmm0, dword ptr ds : [FinalAreaCameraYOrientation]
		comiss xmm0, dword ptr ds : [eax]
		jbe near NotFinalBoss
		pop eax
		push 0
		jmp near ExitASM

	NotFinalBoss:
		pop eax
		push 1

	ExitASM:
		push 0x1C
		push eax
		jmp jmpFinalAreaBossAddr2
	}
}

void PatchFogParameters()
{
	// New environment fog RGB
	constexpr BYTE NewEnvFogSearchBytes[]{ 0x90, 0x90, 0x90, 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x0D };
	NewEnvFogRGB = (void*)ReadSearchedAddresses(0x004798ED, 0x00479B8D, 0x00479D9D, NewEnvFogSearchBytes, sizeof(NewEnvFogSearchBytes), 0x09, __FUNCTION__);

	// Checking address pointer
	if (!NewEnvFogRGB)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Fog parameters for final boss area address 1
	constexpr BYTE FinalBossAddr1SearchBytes[]{ 0x90, 0x90, 0x90, 0x81, 0xEC, 0xB4, 0x02, 0x00, 0x00, 0xA1 };
	DWORD FinalBossAddr1 = SearchAndGetAddresses(0x0050221D, 0x0050254D, 0x00501E6D, FinalBossAddr1SearchBytes, sizeof(FinalBossAddr1SearchBytes), 0x09, __FUNCTION__);

	// Checking address pointer
	if (!FinalBossAddr1)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	OriginalEnvFogRGB = (void*)*(DWORD*)(FinalBossAddr1 + 1);
	jmpFinalAreaBossAddr1 = (void*)(FinalBossAddr1 + 5);

	// Fog parameters for final boss area address 2
	constexpr BYTE FinalBossAddr2SearchBytes[]{ 0x8B, 0x08, 0x6A, 0x01, 0x6A, 0x1C, 0x50, 0xFF, 0x91, 0xC8, 0x00, 0x00, 0x00, 0xA1 };
	DWORD FinalBossAddr2 = SearchAndGetAddresses(0x005038B5, 0x00503BE5, 0x00503505, FinalBossAddr2SearchBytes, sizeof(FinalBossAddr2SearchBytes), 0x02, __FUNCTION__);

	// Checking address pointer
	if (!FinalBossAddr2)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpFinalAreaBossAddr2 = (void*)(FinalBossAddr2 + 5);

	// Get room ID address
	GetRoomIDPointer();

	// Get cutscene ID address
	GetCutsceneIDPointer();

	// Get cutscene camera position address
	GetCutscenePosPointer();

	// Get Camera in-game position Y
	GetInGameCameraPosYPointer();

	// Checking address pointers
	if (!RoomIDAddr || !CutsceneIDAddr || !CutscenePosAddr || !InGameCameraPosYAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get cutscene ID or position address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Updating Fog Parameters...";
	WriteJMPtoMemory((BYTE*)FinalBossAddr1, FinalAreaBoss1ASM);
	WriteJMPtoMemory((BYTE*)FinalBossAddr2, FinalAreaBoss2ASM);
}

// Slow the fog movement in certain areas of the game to better match the PS2's fog movements
void RunFogSpeed()
{
	static float *FogSpeed = nullptr;
	if (!FogSpeed)
	{
		RUNONCE();

		constexpr BYTE SearchBytes[]{ 0xD9, 0x5C, 0x24, 0x5C, 0xD9, 0xC9, 0xD8, 0x44, 0x24, 0x60, 0xD9, 0x5C, 0x24, 0x60, 0xDB, 0x44, 0x24, 0x44, 0xD8, 0x3D };
		FogSpeed = (float*)ReadSearchedAddresses(0x0048683D, 0x00486ADD, 0x00486CED, SearchBytes, sizeof(SearchBytes), 0x14, __FUNCTION__);
		if (!FogSpeed)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	static float* FogFrontPointer = nullptr;
	if (!FogFrontPointer)
	{
		RUNONCE();

		// Get Fog address
		constexpr BYTE FogSearchBytes[]{ 0x8B, 0xF8, 0x81, 0xE7, 0xFF, 0x00, 0x00, 0x00, 0xC1, 0xE7, 0x10, 0x25, 0x00, 0xFF, 0x00, 0xFF, 0x0B, 0xF7, 0x0B, 0xF0, 0x56 };
		DWORD FogAddr = SearchAndGetAddresses(0x00479E71, 0x0047A111, 0x0047A321, FogSearchBytes, sizeof(FogSearchBytes), 0x00, __FUNCTION__);

		// Checking address pointer
		if (!FogAddr)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
		void* FogMemoryAddr = (void*)(FogAddr - 0x1A);

		// Check for valid code before updating
		if (!CheckMemoryAddress(FogMemoryAddr, "\x8B\x0D", 2, __FUNCTION__))
		{
			Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
			return;
		}

		// Fog front and back addresses
		memcpy(&FogFrontPointer, (void*)((DWORD)FogMemoryAddr - 4), sizeof(DWORD));
	}

	static float *JamesFogInfluence = nullptr;
	if (!JamesFogInfluence)
	{
		RUNONCE();

		constexpr BYTE SearchBytes[]{ 0xC1, 0xE0, 0x18, 0x0B, 0xC3, 0x55, 0x89, 0x84, 0x24, 0x80, 0x00, 0x00, 0x00, 0xE8 };
		JamesFogInfluence = (float*)ReadSearchedAddresses(0x00488ACB, 0x00488D6B, 0x00488F7B, SearchBytes, sizeof(SearchBytes), 0x14, __FUNCTION__);
		if (!JamesFogInfluence)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	LOG_ONCE("Setting Fog Speed Fix...");

	switch (GetRoomID())
	{
	case R_FOREST_CEMETERY:
	case R_TOWN_EAST:
	case R_TOWN_WEST:
	case R_EDI_BOSS_HALL:
	case R_HTL_ALT_READING_RM:
	case R_FINAL_BOSS_RM:
	case R_END_DOG_RM:
	case R_MAN_GRAND_ENTRANCE:
	case R_MAN_LOUNGE_2F:
	case R_MAN_LONG_HALLWAY:
	case R_MAN_SERV_RM:
	case R_MAN_OUTSIDE_ENTRANCE:
	case R_MAN_BLUE_CREEK_ENTRANCE:
	{
		constexpr float value = 0.25f;
		if (*FogSpeed != value)
		{
			*FogSpeed = value;
		}
		break;
	}
	case R_APT_E_COURTYARD:
	case R_EDI_BOSS_RM_1:
	case R_EDI_BOSS_RM_2:
	{
		constexpr float value = 0.50f;
		if (*FogSpeed != value)
		{
			*FogSpeed = value;
		}
		break;
	}
	}

	// Adjust fog values for Blue Creek Apt Room 209
	if (GetRoomID() == R_APT_W_RM_208_209 && *FogFrontPointer != BlueCreekNewFog)
	{
		*FogFrontPointer = BlueCreekNewFog;
	}

	// Prevents fog from "sticking" to James during certain parts of the Forest trail
	static bool ValueSet = false;
	if (GetRoomID() == R_FOREST_CEMETERY && GetJamesPosY() >= 1125.0f && GetJamesPosY() <= 1575.0f)
	{
		if (!ValueSet)
		{
			ValueSet = true;
			float Value = 10.0f;
			UpdateMemoryAddress(JamesFogInfluence, &Value, sizeof(float));
		}
	}
	else
	{
		if (ValueSet)
		{
			ValueSet = false;
			float Value = 200.0f;
			UpdateMemoryAddress(JamesFogInfluence, &Value, sizeof(float));
		}
	}
}

void PatchFMV()
{

	// Fix fog for cutscene 0x16	
	// 968 to 965.5
	UpdateMemoryAddress(GetMeetingMariaCutsceneFogCounterOnePointer(), "\x00\x60\x71\x44", 0x04);
	// 1463 to 1460
	UpdateMemoryAddress(GetMeetingMariaCutsceneFogCounterTwoPointer(), "\x00\x80\xb6\x44", 0x04);

	// Fix for closet cutscene
	// 360 to 359
	UpdateMemoryAddress(GetRPTClosetCutsceneMannequinDespawnPointer(), "\x00\x80\xB3\x43", 0x04);
	// 1696 to 1695 
	UpdateMemoryAddress(GetRPTClosetCutsceneBlurredBarsDespawnPointer(), "\x00\xE0\xD3\x44", 0x04);
}
