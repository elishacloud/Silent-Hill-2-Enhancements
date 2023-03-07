/**
* Copyright (C) 2022 The Machine Ambassador, mercury501
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

void* jmp_return;
void* jmp_to_loop;

DWORD* muteSound;
DWORD EventIndex;
BYTE MenuEventIndex;

// BGM Fading out instructions
#pragma warning(suppress: 4740)
__declspec(naked) void __stdcall FixInventoryBGMBugASM()
{
	EventIndex = GetEventIndex();
	MenuEventIndex = GetMenuEvent();

	if (MenuEventIndex == 0x0D /*[normal gameplay]*/ || (MenuEventIndex == 0x11 /*[load game menu]*/ && 
		EventIndex == 0x00 /*[load game menu]*/) && LastEventIndex == 0x10)
	{
		if (EventIndex == 0x0B /*[game result screen]*/)
		{
			*muteSound = 0x0F;
			__asm
			{
				jmp jmp_return
			}
		}

		if (EventIndex > 0x03 && EventIndex < 0x0A /*[just about every type of menu]*/ || 
			EventIndex == 0x10 /*[pause menu]*/ || MenuEventIndex == 0x11 /*[normal gameplay]*/)
		{
			__asm
			{
				jmp jmp_return
			}
		}
	}
	__asm
	{
		jmp jmp_to_loop
	}

}

void PatchInventoryBGMBug()
{
	constexpr BYTE BuggyBGMBytes[] = { 0x83, 0xF8, 0x04, 0x75, 0x0D, 0x68 };
	const DWORD BuggyBGMAddr = SearchAndGetAddresses(0x005166E8, 0x00516A18, 0x00516338, BuggyBGMBytes, sizeof(BuggyBGMBytes), -0x1F, __FUNCTION__);

	// Check errors
	if (!BuggyBGMAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	memcpy(&muteSound, (DWORD*)(BuggyBGMAddr - 0x04), sizeof(DWORD));
	jmp_return = reinterpret_cast<void*>(BuggyBGMAddr + 0x24);
	jmp_to_loop = reinterpret_cast<void*>(BuggyBGMAddr + 0x31);

	// Update SH2 code
	Logging::Log() << "Fixing Inventory BGM...";
	WriteJMPtoMemory(reinterpret_cast<BYTE*>(BuggyBGMAddr), *FixInventoryBGMBugASM, 0x24);
}