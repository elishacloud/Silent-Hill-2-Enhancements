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
DWORD FlashlightClockValue;
void *jmpFlashlightClock;

// ASM functions to fix an issue with lighting in a certain room
__declspec(naked) void __stdcall FlashlightClockASM()
{
	__asm
	{
		and edx, FlashlightClockValue
		jmp jmpFlashlightClock
	}
}

void PatchFlashlightClockPush()
{
	// Get address
	constexpr BYTE SearchBytes[]{ 0xD8, 0xD9, 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x75, 0x0A, 0xDD, 0xD8, 0x89, 0x3D };
	void *Address = (void*)SearchAndGetAddresses(0x0048E81B, 0x0048EABB, 0x0048ECCB, SearchBytes, sizeof(SearchBytes), 0x3D);

	if (!Address)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	jmpFlashlightClock = (void*)((DWORD)Address + 0x06);
	FlashlightClockValue = 0xFFFFFF3F;

	// Update SH2 code
	Logging::Log() << "Fixing Flashlight Clock Push...";
	WriteJMPtoMemory((BYTE*)Address, *FlashlightClockASM, 6);
}

void RunFlashlightClockPush()
{
	if (GetRoomID() == 0x18 && GetInGameCameraPosY() == -112.6049728f)
	{
		constexpr DWORD NewValue = 0xFFFFFFEF;
		if (FlashlightClockValue != NewValue)
		{
			FlashlightClockValue = NewValue;
		}
	}
	else
	{
		constexpr DWORD NewValue = 0xFFFFFF3F;
		if (FlashlightClockValue != NewValue)
		{
			FlashlightClockValue = NewValue;
		}
	}
}
