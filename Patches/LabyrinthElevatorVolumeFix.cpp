/**
* Copyright (C) 2025 Murugo
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
#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

constexpr float kFrameTimeThirtyFPS = 1.0f / 30.0f;
constexpr float kFrameTimeSixtyFPS = 1.0f / 60.0f;

__declspec(naked) void __stdcall GetElevatorAudioFrameTimeASM()
{
	__asm
	{
		push eax
		mov al, byte ptr ds : [SetSixtyFPS]
		test al, al
		pop eax
		je ThirtyFPS
		fld dword ptr ds : [kFrameTimeSixtyFPS]
		ret

	ThirtyFPS:
		fld dword ptr ds : [kFrameTimeThirtyFPS]
		ret
	}
}

// Adjusts the delta time calculation used to fade the sound of the elevator leading to the labyrinth to avoid hitching.
void PatchLabyrinthElevatorVolumeFix()
{
	const BYTE ElevatorVolumeSearchBytes[]{ 0x68, 0x00, 0x00, 0x80, 0x3F, 0x68, 0x53, 0x46, 0x00, 0x00 };
	const DWORD ElevatorVolumeAddr = SearchAndGetAddresses(0x00582ED2, 0x00583782, 0x005830A2, ElevatorVolumeSearchBytes, sizeof(ElevatorVolumeSearchBytes), 0x00, __FUNCTION__);
	if (!ElevatorVolumeAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Patching Labyrinth Elevator Volume Fix...";

	const DWORD AudioFrameTimeOffsets[]{ 0x12, 0x7A, 0xA2, 0xD4, 0x127, 0x2F7, 0x340, 0x3A5, 0x3EC, 0x41D, 0x441, 0x4B1, 0x4C4, 0x4DB, 0x4F3, 0x523, 0x53B };
	for (const DWORD offset : AudioFrameTimeOffsets)
	{
		WriteCalltoMemory((BYTE*)(ElevatorVolumeAddr + offset), *GetElevatorAudioFrameTimeASM, 0x05);
	}

	// Play the sound at full volume (do not multiply volume by 0.8).
	UpdateMemoryAddress((void*)(ElevatorVolumeAddr + 0x8C), "\x90\x90\x90\x90\x90\x90", 6);
}
