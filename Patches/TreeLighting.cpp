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
#include "Common\Settings.h"
#include "Logging\Logging.h"

// Variables for ASM
void *jmpTreeLightingAddr;

// ASM functions to fix tree lighting conditions for Maria & Leave endings
__declspec(naked) void __stdcall TreeLightingASM()
{
	__asm
	{
		mov ebp, dword ptr ss : [esp + 0x54]
		push esi
		push edi
		push eax
		mov eax, dword ptr ds : [CutsceneIDAddr]
		cmp dword ptr ds : [eax], 0x5A	// Maria Ending
		je near MariaLeaveEndings
		cmp dword ptr ds : [eax], 0x5D	// Leave Ending
		je near MariaLeaveEndings
		pop eax
		push 1
		jmp near ExitASM

		MariaLeaveEndings:
		pop eax
		push 2

		ExitASM:
		jmp jmpTreeLightingAddr
	}
}

// Patch SH2 code to fix tree lighting conditions for Maria & Leave endings
void PatchTreeLighting()
{
	// Get address
	constexpr BYTE SearchBytes[]{ 0x8B, 0x6C, 0x24, 0x54, 0x56, 0x57, 0x6A, 0x01, 0xE8 };
	DWORD Address = SearchAndGetAddresses(0x004A13C5, 0x004A1675, 0x004A0F35, SearchBytes, sizeof(SearchBytes), 0x00, __FUNCTION__);

	// Checking address pointer
	if (!Address)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpTreeLightingAddr = (void*)(Address + 0x08);

	// Get cutscene ID address
	CutsceneIDAddr = GetCutsceneIDPointer();

	// Checking address pointer
	if (!CutsceneIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get cutscene ID address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Fixing tree lighting conditions...";
	WriteJMPtoMemory((BYTE*)Address, *TreeLightingASM, 8);
}

// Update tree lighting color
void RunTreeColor()
{
	// Get Address
	static DWORD ColorAddress = NULL;
	if (!ColorAddress)
	{
		RUNONCE();

		// Get address
		constexpr BYTE SearchBytes[]{ 0x90, 0x90, 0x90, 0x90, 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x0D };
		ColorAddress = ReadSearchedAddresses(0x004798EC, 0x00479B8C, 0x00479D9C, SearchBytes, sizeof(SearchBytes), 0x0A, __FUNCTION__);

		// Checking address pointer
		if (!ColorAddress)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	// Tree lighting color fix for ending cutscene (horizon fog color)
	if (GetCutsceneID() == 0x5D && WidescreenFix)
	{
		constexpr BYTE Red = 125;
		constexpr BYTE Green = 135;
		constexpr BYTE Blue = 150;
		if (*(BYTE*)(ColorAddress + 0x00) != Red || *(BYTE*)(ColorAddress + 0x01) != Green || *(BYTE*)(ColorAddress + 0x02) != Blue)
		{
			constexpr DWORD ColorValue = (Red) | (Green << 8) | (Blue << 16);
			UpdateMemoryAddress((BYTE*)(ColorAddress + 0x00), &ColorValue, sizeof(BYTE) * 3);
		}
	}
	// Hotel Room 312 light fix (game world color)
	else if (GetRoomID() == 0xA2 && GetTransitionState() != 2 && GetEventIndex())
	{
		constexpr BYTE Red = 255;
		constexpr BYTE Green = 255;
		constexpr BYTE Blue = 227;
		if (*(BYTE*)(ColorAddress + 0x30) != Red || *(BYTE*)(ColorAddress + 0x31) != Green || *(BYTE*)(ColorAddress + 0x32) != Blue)
		{
			constexpr DWORD ColorValue = (Red) | (Green << 8) | (Blue << 16);
			UpdateMemoryAddress((BYTE*)(ColorAddress + 0x30), &ColorValue, sizeof(BYTE) * 3);
		}
	}
}
