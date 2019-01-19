/**
* Copyright (C) 2018 Elisha Riedlinger
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

// Predefined code bytes
constexpr BYTE FogSearchBytes[]{ 0x8B, 0xF8, 0x81, 0xE7, 0xFF, 0x00, 0x00, 0x00, 0xC1, 0xE7, 0x10, 0x25, 0x00, 0xFF, 0x00, 0xFF, 0x0B, 0xF7, 0x0B, 0xF0, 0x56 };
constexpr BYTE FogCheckBytes[]{ 0x8B, 0x0D };
constexpr BYTE BlueCreekFogSearchBytes[]{ 0x85, 0xC0, 0xDF, 0xE0, 0x0F, 0x84, 0x90, 0x01, 0x00, 0x00, 0xF6, 0xC4, 0x44, 0x7A, 0x2A };
constexpr BYTE BlueCreekFogCheckBytes[]{ 0xC7, 0x05 };

// Variables for ASM
DWORD CameraConditionFlag = 0x00;
void *RoomIDPtr;
void *CutsceneIDPtr;
void *CameraPosPtr;
void *FogFrontPointer;
void *FogBackPointer;
void *jmpFogReturnAddr;
void *jmpBlueCreekFogReturnAddr;

// Fog values
constexpr float NewFrontFog = 2200.0f;
constexpr float NewBackFog = 2800.0f;
constexpr float OriginalFrontFog = 8000.0f;
constexpr float OriginalBackFog = 12000.0f;
constexpr float BlueCreekNewFog = 4200.0f;
constexpr float BlueCreekOriginalFog = 3000.0f;

// ASM functions to adjust fog values for Angela cutscene
__declspec(naked) void __stdcall FogAdjustmentASM()
{
	__asm
	{
		pushf
		push ecx
		cmp CameraConditionFlag, 0x01
		je near CheckCutsceneID								// jumps to check cutscene ID if camera conditions were already met
		mov eax, dword ptr ds : [CameraPosPtr]
		cmp dword ptr ds : [eax], 0x4767DA5D
		jne near ConditionsNotMet							// jumps if camera is not facing the door

	CheckCutsceneID:
		mov eax, dword ptr ds : [CutsceneIDPtr]
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
		popf
		jmp jmpFogReturnAddr
	}
}

// ASM functions to adjust fog values for Blue Creek Apt Room 209
__declspec(naked) void __stdcall BlueCreekFogAdjustmentASM()
{
	__asm
	{
		pushf
		push eax
		push ecx
		mov eax, dword ptr ds : [RoomIDPtr]
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
		popf
		jmp jmpBlueCreekFogReturnAddr
	}
}

void UpdateFogParameters()
{
	// Get Fog address
	DWORD FogAddr = (DWORD)CheckMultiMemoryAddress((void*)0x00479E71, (void*)0x0047A111, (void*)0x0047A321, (void*)FogSearchBytes, sizeof(FogSearchBytes));

	// Search for address
	if (!FogAddr)
	{
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		FogAddr = (DWORD)GetAddressOfData(FogSearchBytes, sizeof(FogSearchBytes), 1, 0x00479831, 1800);
	}

	// Checking address pointer
	if (!FogAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpFogReturnAddr = (void*)(FogAddr - 0x1A);

	// Get Blue Creek return address
	FogAddr = (DWORD)CheckMultiMemoryAddress((void*)0x0047BE75, (void*)0x0047C115, (void*)0x0047C325, (void*)BlueCreekFogSearchBytes, sizeof(BlueCreekFogSearchBytes));

	// Search for address
	if (!FogAddr)
	{
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		FogAddr = (DWORD)GetAddressOfData(BlueCreekFogSearchBytes, sizeof(BlueCreekFogSearchBytes), 1, 0x0047B835, 1800);
	}

	// Checking address pointer
	if (!FogAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpBlueCreekFogReturnAddr = (void*)(FogAddr + 0x23);

	// Check for valid code before updating
	if (!CheckMemoryAddress(jmpFogReturnAddr, (void*)FogCheckBytes, sizeof(FogCheckBytes)) ||
		!CheckMemoryAddress(jmpBlueCreekFogReturnAddr, (void*)BlueCreekFogCheckBytes, sizeof(BlueCreekFogCheckBytes)))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Fog front and back addresses
	memcpy(&FogFrontPointer, (void*)((DWORD)jmpFogReturnAddr - 4), sizeof(DWORD));
	FogBackPointer = (void*)((DWORD)FogFrontPointer - 4);

	// Get room ID address
	RoomIDPtr = GetRoomIDPointer();

	// Get cutscene ID address
	CutsceneIDPtr = GetCutsceneIDPointer();

	// Get cutscene camera position address
	CameraPosPtr = GetCutscenePosPointer();

	// Checking address pointers
	if (!RoomIDPtr || !CutsceneIDPtr || !CameraPosPtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get cutscene ID or position address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Updating Fog Parameters...";
	WriteJMPtoMemory((BYTE*)((DWORD)jmpFogReturnAddr - 5), *FogAdjustmentASM);
	WriteJMPtoMemory((BYTE*)((DWORD)jmpBlueCreekFogReturnAddr - 10), *BlueCreekFogAdjustmentASM, 10);
}
