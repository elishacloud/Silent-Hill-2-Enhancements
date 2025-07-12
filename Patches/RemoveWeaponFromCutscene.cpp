/**
* Copyright (C) 2025 Elisha Riedlinger
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
#include "Logging\Logging.h"

namespace {
#define WT_NONE WEAPONTYPE::WT_NONE

	WEAPONTYPE LastWeaponRender = WT_NONE;
	WEAPONTYPE LastWeaponHandGrip = WT_NONE;

	WEAPONTYPE* pWeaponRenderAddr = nullptr;
	WEAPONTYPE* pWeaponHandGripAddr = nullptr;

	BYTE* pInGameVoiceEvent = nullptr;

	bool IsWeaponStored = false;

	const CUTSCENEID CheckCutscene[] = {
		CS_APT_JUICE_CHUTE,
		CS_APT_RPT_FIGHT,
		CS_MARIA_HOTEL_WRONG_WAY,
		CS_BOWL_MARIA,
		CS_BOWL_MARIA_JAMES,
		CS_MARIA_BOWL_WRONG_WAY,
		CS_HSP_LAURA_ENTERS,
		CS_HSP_ROOF_FALL,
		CS_HSP_ROOF_RECOVER,
		CS_HSP_ALT_SHELF_PUSH,
		CS_HOLE_JUMP_1,
		CS_PRIS_WELL,
		CS_HOLE_JUMP_2,
		CS_HOLE_JUMP_3,
		CS_HOLE_JUMP_3_RECOVER,
		CS_HOLE_JUMP_4,
		CS_HOLE_JUMP_4_RECOVER,
		CS_HOLE_JUMP_5,
		CS_HOLE_JUMP_5_RECOVER,
		CS_EDI_BOSS_1_INTRO,
		CS_EDI_BOSS_2_INTRO,
		CS_HTL_LAURA_PIANO };

	bool IsDefinedCutscene()
	{
		// Get weapon and in voice event addresses
		RUNCODEONCE({
			GetWeaponRender();
			GetWeaponHandGrip();
			pWeaponRenderAddr = WeaponRenderAddr;
			pWeaponHandGripAddr = WeaponHandGripAddr;
			pInGameVoiceEvent = GetInGameVoiceEvent();
			}
		);

		// Check addresses
		if (!pWeaponRenderAddr || !pWeaponHandGripAddr || !pInGameVoiceEvent)
		{
			LOG_ONCE(__FUNCTION__ << " Error: failed to find weapon or in voice event memory addresses!");
			return false;
		}

		// Get game events
		ROOMID CurrentRoom = (ROOMID)GetRoomID();
		CUTSCENEID CurrentCutscene = (CUTSCENEID)GetCutsceneID();
		DWORD InGameVoiceEvent = *pInGameVoiceEvent;

		// Check the cutscene in room R_LAB_TOP_G
		if (CurrentRoom == R_LAB_TOP_G && (InGameVoiceEvent != 0 || CurrentCutscene == CS_PS_ANGELA_SCREAM))
		{
			return true;
		}

		// Check for other cutscenes
		for (auto& cutsceen : CheckCutscene)
		{
			if (cutsceen == CurrentCutscene)
			{
				return true;
			}
		}

		// Default to false
		return false;
	}
}

void PatchRemoveWeaponFromCutscene()
{
	bool IsInDefinedCutsceen = IsDefinedCutscene();
	static bool LastIsInDefinedCutsceen = false;
	CUTSCENEID CurrentCutscene = (CUTSCENEID)GetCutsceneID();

	// Remove weapon in specified cutsceen
	if (IsInDefinedCutsceen)
	{
		if (!IsWeaponStored &&														// Has not already stored weapon
			IsInDefinedCutsceen != LastIsInDefinedCutsceen &&						// Change from last frame
			(*pWeaponRenderAddr != WT_NONE || *pWeaponHandGripAddr != WT_NONE))		// James is holding a weapon
		{
			LOG_ONCE(__FUNCTION__ << " Removing weapon during cutsceen: " << CurrentCutscene);

			IsWeaponStored = true;

			// Store current weapon
			LastWeaponRender = *pWeaponRenderAddr;
			LastWeaponHandGrip = *pWeaponHandGripAddr;

			// Clear weapon
			*pWeaponRenderAddr = WT_NONE;
			*pWeaponHandGripAddr = WT_NONE;
		}
	}
	// Restore weapon after cutscene exits
	else if (IsWeaponStored && CurrentCutscene == CS_NONE)
	{
		IsWeaponStored = false;

		// Restore last weapon state
		*pWeaponRenderAddr = LastWeaponRender;
		*pWeaponHandGripAddr = LastWeaponHandGrip;
	}

	// Store last state
	LastIsInDefinedCutsceen = IsInDefinedCutsceen;
}

bool CheckForSkipFrameCutscene()
{
	bool IsInDefinedCutsceen = IsDefinedCutscene();
	static bool LastIsInDefinedCutsceen = false;

	bool ret = (IsInDefinedCutsceen != LastIsInDefinedCutsceen);

	// Store last state
	LastIsInDefinedCutsceen = IsInDefinedCutsceen;

	return ret;
}
