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

// Variables for ASM
DWORD CameraConditionFlag = 0x00;
void *FogFrontPointer;
void *FogBackPointer;
void *jmpFogReturnAddr;
void *jmpBlueCreekFogReturnAddr;
void *NewEnvFogRGB;
void *OriginalEnvFogRGB;
void *jmpFinalAreaBossAddr1;
void *jmpFinalAreaBossAddr2;


// Fog values
constexpr float NewFrontFog = 2200.0f;
constexpr float NewBackFog = 2800.0f;
constexpr float OriginalFrontFog = 8000.0f;
constexpr float OriginalBackFog = 12000.0f;
constexpr float BlueCreekNewFog = 4200.0f;
constexpr float BlueCreekOriginalFog = 3000.0f;
constexpr float FinalAreaCameraYOrientation = -15750.0f;

// ASM functions to adjust fog values for Angela cutscene
__declspec(naked) void __stdcall FogAdjustmentASM()
{
	__asm
	{
		push ecx
		cmp CameraConditionFlag, 0x01
		je near CheckCutsceneID								// jumps to check cutscene ID if camera conditions were already met
		mov eax, dword ptr ds : [CutscenePosAddr]
		cmp dword ptr ds : [eax], 0x4767DA5D
		jne near ConditionsNotMet							// jumps if camera is not facing the door

	CheckCutsceneID:
		mov eax, dword ptr ds : [CutsceneIDAddr]
		cmp dword ptr ds : [eax], 0x12
		jne near ConditionsNotMet							// jumps if not angela mirror cutscene

	// Conditions Met
		mov CameraConditionFlag, 0x01						// writes 01 to indicate that camera conditions have been met
		mov ecx, NewFrontFog
		mov eax, dword ptr ds : [FogFrontPointer]
		mov dword ptr ds : [eax], ecx						// set new fog front value
		mov ecx, NewBackFog
		mov eax, dword ptr ds : [FogBackPointer]
		mov dword ptr ds : [eax], ecx						// set new fog back value
		jmp near ExitFunction

	ConditionsNotMet:
		mov CameraConditionFlag, 0x00						// writes 00 to indicate that camera conditions have not been met
		mov eax, dword ptr ds : [FogFrontPointer]
		mov eax, dword ptr ds : [eax]
		cmp eax, NewFrontFog
		jne near ExitFunction								// jumps if fog front value is not the new fog front value
		mov eax, dword ptr ds : [FogBackPointer]
		mov eax, dword ptr ds : [eax]
		cmp eax, NewBackFog
		jne near ExitFunction								// jumps if fog back value is not the new fog back value
		mov ecx, OriginalFrontFog
		mov eax, dword ptr ds : [FogFrontPointer]
		mov dword ptr ds : [eax], ecx						// restores original fog front value; 8000 flt
		mov ecx, OriginalBackFog
		mov eax, dword ptr ds : [FogBackPointer]
		mov dword ptr ds : [eax], ecx						// restores original fog back value; 12000 flt

	ExitFunction:
		mov eax, dword ptr ds : [FogFrontPointer]
		mov eax, dword ptr ds : [eax]
		pop ecx
		jmp jmpFogReturnAddr
	}
}

// ASM functions to adjust fog values for Blue Creek Apt Room 209
__declspec(naked) void __stdcall BlueCreekFogAdjustmentASM()
{
	__asm
	{
		push eax
		push ecx
		mov eax, dword ptr ds : [RoomIDAddr]
		cmp dword ptr ds : [eax], 0x28
		jne near ConditionsNotMet						// jumps if not Blue Creek Apt Room 209
		mov ecx, BlueCreekNewFog
		mov eax, dword ptr ds : [FogFrontPointer]
		mov dword ptr ds : [eax], ecx					// new fog value
		jmp near ExitFunction

	ConditionsNotMet:
		mov ecx, BlueCreekOriginalFog
		mov eax, dword ptr ds : [FogFrontPointer]
		mov dword ptr ds : [eax], ecx					// original fog value; 3000 flt

	ExitFunction:
		pop ecx
		pop eax
		jmp jmpBlueCreekFogReturnAddr
	}
}

// ASM final boss area 1
__declspec(naked) void __stdcall FinalAreaBoss1ASM()
{
	__asm
	{
		mov eax, dword ptr ds : [RoomIDAddr]
		cmp dword ptr ds : [eax], 0xBB
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
		cmp dword ptr ds : [eax], 0xBB
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
	// Get Fog address
	constexpr BYTE FogSearchBytes[]{ 0x8B, 0xF8, 0x81, 0xE7, 0xFF, 0x00, 0x00, 0x00, 0xC1, 0xE7, 0x10, 0x25, 0x00, 0xFF, 0x00, 0xFF, 0x0B, 0xF7, 0x0B, 0xF0, 0x56 };
	DWORD FogAddr = SearchAndGetAddresses(0x00479E71, 0x0047A111, 0x0047A321, FogSearchBytes, sizeof(FogSearchBytes), 0x00);

	// Checking address pointer
	if (!FogAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpFogReturnAddr = (void*)(FogAddr - 0x1A);

	// Get Blue Creek return address
	constexpr BYTE BlueCreekFogSearchBytes[]{ 0x85, 0xC0, 0xDF, 0xE0, 0x0F, 0x84, 0x90, 0x01, 0x00, 0x00, 0xF6, 0xC4, 0x44, 0x7A, 0x2A };
	FogAddr = SearchAndGetAddresses(0x0047BE75, 0x0047C115, 0x0047C325, BlueCreekFogSearchBytes, sizeof(BlueCreekFogSearchBytes), 0x00);

	// Checking address pointer
	if (!FogAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpBlueCreekFogReturnAddr = (void*)(FogAddr + 0x23);

	// Check for valid code before updating
	if (!CheckMemoryAddress(jmpFogReturnAddr, "\x8B\x0D", 2) ||
		!CheckMemoryAddress(jmpBlueCreekFogReturnAddr, "\xC7\x05", 2))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Fog front and back addresses
	memcpy(&FogFrontPointer, (void*)((DWORD)jmpFogReturnAddr - 4), sizeof(DWORD));
	FogBackPointer = (void*)((DWORD)FogFrontPointer - 4);

	// New environment fog RGB
	constexpr BYTE NewEnvFogSearchBytes[]{ 0x90, 0x90, 0x90, 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x0D };
	NewEnvFogRGB = (void*)ReadSearchedAddresses(0x004798ED, 0x00479B8D, 0x00479D9D, NewEnvFogSearchBytes, sizeof(NewEnvFogSearchBytes), 0x09);

	// Checking address pointer
	if (!NewEnvFogRGB)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Fog parameters for final boss area address 1
	constexpr BYTE FinalBossAddr1SearchBytes[]{ 0x90, 0x90, 0x90, 0x81, 0xEC, 0xB4, 0x02, 0x00, 0x00, 0xA1 };
	DWORD FinalBossAddr1 = SearchAndGetAddresses(0x0050221D, 0x0050254D, 0x00501E6D, FinalBossAddr1SearchBytes, sizeof(FinalBossAddr1SearchBytes), 0x09);

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
	DWORD FinalBossAddr2 = SearchAndGetAddresses(0x005038B5, 0x00503BE5, 0x00503505, FinalBossAddr2SearchBytes, sizeof(FinalBossAddr2SearchBytes), 0x02);

	// Checking address pointer
	if (!FinalBossAddr2)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpFinalAreaBossAddr2 = (void*)(FinalBossAddr2 + 5);

	// Get room ID address
	RoomIDAddr = GetRoomIDPointer();

	// Get cutscene ID address
	CutsceneIDAddr = GetCutsceneIDPointer();

	// Get cutscene camera position address
	CutscenePosAddr = GetCutscenePosPointer();

	// Get Camera in-game position Y
	InGameCameraPosYAddr = GetInGameCameraPosYPointer();

	// Checking address pointers
	if (!RoomIDAddr || !CutsceneIDAddr || !CutscenePosAddr || !InGameCameraPosYAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get cutscene ID or position address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Updating Fog Parameters...";
	WriteJMPtoMemory((BYTE*)((DWORD)jmpFogReturnAddr - 5), *FogAdjustmentASM);
	WriteJMPtoMemory((BYTE*)((DWORD)jmpBlueCreekFogReturnAddr - 10), *BlueCreekFogAdjustmentASM, 10);
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
		FogSpeed = (float*)ReadSearchedAddresses(0x0048683D, 0x00486ADD, 0x00486CED, SearchBytes, sizeof(SearchBytes), 0x14);
		if (!FogSpeed)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	static float *JamesFogInfluence = nullptr;
	if (!JamesFogInfluence)
	{
		RUNONCE();

		constexpr BYTE SearchBytes[]{ 0xC1, 0xE0, 0x18, 0x0B, 0xC3, 0x55, 0x89, 0x84, 0x24, 0x80, 0x00, 0x00, 0x00, 0xE8 };
		JamesFogInfluence = (float*)ReadSearchedAddresses(0x00488ACB, 0x00488D6B, 0x00488F7B, SearchBytes, sizeof(SearchBytes), 0x14);
		if (!JamesFogInfluence)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	LOG_ONCE("Setting Fog Speed Fix...");

	switch (GetRoomID())
	{
	case 0x03:
	case 0x04:
	case 0x08:
	case 0x8E:
	case 0xB0:
	case 0xBB:
	case 0xBD:
	case 0xC0:
	case 0xC2:
	case 0xCA:
	case 0xC4:
	case 0xD3:
	case 0xD4:
	{
		constexpr float value = 0.25f;
		if (*FogSpeed != value)
		{
			*FogSpeed = value;
		}
		break;
	}
	case 0x07:
	case 0x8F:
	case 0x90:
	{
		constexpr float value = 0.50f;
		if (*FogSpeed != value)
		{
			*FogSpeed = value;
		}
		break;
	}
	}

	static bool ValueSet = false;

	// Prevents fog from "sticking" to James during certain parts of the Forest trail
	if (GetRoomID() == 0x03 && GetJamesPosY() >= 1125.0f && GetJamesPosY() <= 1575.0f)
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
