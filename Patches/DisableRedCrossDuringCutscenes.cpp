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
#include "Common\Settings.h"
#include "Logging\Logging.h"

// Variables for ASM
BYTE RedCrossFlag;
BYTE *RedCrossFlagPointer = &RedCrossFlag;
BYTE *RedCrossPointer;
void *jmpEnableAddr;
void *jmpDisableAddr;

// ASM functions to disable RedCross during cutscenes
__declspec(naked) void __stdcall RedCrossCutscenesASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [RedCrossFlagPointer]
		cmp byte ptr ds : [eax], 0x00
		jg near DisableHealthIndicator

		mov eax, RedCrossPointer
		cmp byte ptr ds : [eax], 0x00
		jg near DisableHealthIndicator

		mov eax, dword ptr ds : [CutsceneIDAddr]
		cmp dword ptr ds : [eax], 0x24
		je near EnableHealthIndicator
		cmp dword ptr ds : [eax], 0x00
		je near EnableHealthIndicator

	DisableHealthIndicator:
		pop eax
		jmp jmpDisableAddr

	EnableHealthIndicator:
		pop eax
		fld dword ptr ds : [ecx + 0x00000140]
		jmp jmpEnableAddr
	}
}

// Update SH2 code to disable RedCross during cutscenes
void UpdateRedCrossInCutscene()
{
	// Get RedCross address
	constexpr BYTE RedCrossSearchBytes[]{ 0xD9, 0x81, 0x40, 0x01, 0x00, 0x00, 0xD8, 0xA1, 0x3C, 0x01, 0x00, 0x00, 0xD8, 0xB1, 0x40, 0x01, 0x00, 0x00 };
	DWORD RedCrossAddr = SearchAndGetAddresses(0x004762D0, 0x00476570, 0x00476780, RedCrossSearchBytes, sizeof(RedCrossSearchBytes), 0x00);

	// Checking address pointer
	if (!RedCrossAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpEnableAddr = (void*)(RedCrossAddr + 0x06);
	jmpDisableAddr = (void*)(RedCrossAddr + 0xC2);

	// Get cutscene ID address
	CutsceneIDAddr = GetCutsceneIDPointer();

	// Checking address pointer
	if (!CutsceneIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get cutscene ID address!";
		return;
	}

	// Get memory pointer for RedCross Animation
	constexpr BYTE RedCrossPtrSearchBytes[]{ 0x84, 0xC0, 0x74, 0x1B, 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x4C, 0x24, 0x08, 0xA3 };
	DWORD RedCrossMemoryPtr = SearchAndGetAddresses(0x004EEACE, 0x004EED7E, 0x004EE63E, RedCrossPtrSearchBytes, sizeof(RedCrossPtrSearchBytes), 0x1E2);

	// Checking address pointer
	if (!RedCrossMemoryPtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
		return;
	}
	memcpy(&RedCrossPointer, (void*)(RedCrossMemoryPtr + 1), sizeof(DWORD));

	// Check for valid code before updating
	if (!CheckMemoryAddress(jmpDisableAddr, "\xC7\x05", 2) ||
		!CheckMemoryAddress((void*)RedCrossMemoryPtr, "\xA1", 1))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Update SH2 code
	RedCrossFlag = DisableRedCross;
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
