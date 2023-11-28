/**
* Copyright (C) 2023 The Machine Ambassador, mercury501
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
DWORD LastRoomID;
bool SoundFixFlag;
DWORD EventIndex;
BYTE MenuEventIndex;

// BGM Fading out instructions
#pragma warning(suppress: 4740)
__declspec(naked) void __stdcall FixInventoryBGMBugASM()
{
	EventIndex = GetEventIndex();
	MenuEventIndex = GetMenuEvent();

	if (MenuEventIndex == 13 /*In-game*/ || 
		(MenuEventIndex == 17 /*Load Screen*/ && EventIndex == EVENT_LOAD_SCR))
	{
		if (EventIndex == EVENT_GAME_RESULT_11)
		{
			*muteSound = 0x0F;
			__asm
			{
				jmp jmp_return
			}
		}

		if (EventIndex == EVENT_IN_GAME || EventIndex == EVENT_MAP || EventIndex == EVENT_INVENTORY || EventIndex == EVENT_OPTIONS_FMV ||
			EventIndex == EVENT_MEMO_LIST || EventIndex == EVENT_SAVE_SCREEN || EventIndex == EVENT_PAUSE_MENU || MenuEventIndex == 17 /*Load Screen*/)
		{
			SoundFixFlag = (LastRoomID != GetRoomID());
			LastRoomID = GetRoomID();

			if (SoundFixFlag || EventIndex == EVENT_IN_GAME)
			{
				__asm
				{
					jmp jmp_return
				}
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

	muteSound = (DWORD*)*(DWORD*)(BuggyBGMAddr - 0x04);
	jmp_return = reinterpret_cast<void*>(BuggyBGMAddr + 0x24);
	jmp_to_loop = reinterpret_cast<void*>(BuggyBGMAddr + 0x31);

	// Update SH2 code
	Logging::Log() << "Fixing Inventory BGM...";
	WriteJMPtoMemory(reinterpret_cast<BYTE*>(BuggyBGMAddr), *FixInventoryBGMBugASM, 0x24);
}