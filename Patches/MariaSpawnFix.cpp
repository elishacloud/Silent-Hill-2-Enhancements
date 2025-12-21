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
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Prevents Maria from spawning out of bounds in certain rooms after reuniting with her outside of
// Pete's Bowl-O-Rama or in the basement of the Otherworld Hospital.
void RunMariaSpawnFix()
{
	// If enabled, when loading the next room, the game will set Maria's position to her last saved
	// position from the current save file.
	static WORD* UseSavedPos = nullptr;

	if (!UseSavedPos)
	{
		RUNONCE();

		constexpr BYTE SearchBytes[]{ 0x33, 0xC9, 0xB8, 0x21, 0x00, 0x00, 0x00, 0xBA };
		UseSavedPos = (WORD*)ReadSearchedAddresses(0x0052D9FB, 0x0052DD2B, 0x0052D64B, SearchBytes, sizeof(SearchBytes), -0x21, __FUNCTION__);
		if (!UseSavedPos)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	if (GetCutsceneID() == CS_BOWL_MARIA_JAMES || GetCutsceneID() == CS_HSP_ALT_MARIA_BASEMENT ||
		(GetRoomID() == R_HSP_ALT_BASEMENT && GetEventIndex() == EVENT_GAME_FMV))
	{
		*UseSavedPos = 0;
	}
}
