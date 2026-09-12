/**
* Copyright (C) 2024 mercury501
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

#include "Common\Utils.h"
#include "Patches\Patches.h"

float* FirstAddr = nullptr;
float* SecondAddr = nullptr;

bool EnteredCutscene = false;

void RunPhRotationFix()
{
	if (!FirstAddr || !SecondAddr)
	{
		Logging::LogDebug() << __FUNCTION__ << " No addresses set for PH Rotation Fix. Skipping patch.";
		return;
	}

	bool IsInCutscene = GetCutsceneID() == CS_HTL_ALT_RPT_BOSS_FINISH;

	if (IsInCutscene)
	{
		EnteredCutscene = true;
		Logging::LogDebug() << "Entered Cutscene: CS_HTL_ALT_RPT_BOSS_FINISH";
		return;
	}
	else if (!IsInCutscene && EnteredCutscene)
	{
		EnteredCutscene = false;
		Logging::LogDebug() << "Exited Cutscene: CS_HTL_ALT_RPT_BOSS_FINISH";

		*FirstAddr = 0.0;
		*SecondAddr = 2.5;
	}
}

void PatchPhRotationAfterBossFight()
{
	Logging::Log() << "Patching PH Rotation After Boss Fight";

	if (GameVersion == SH2V_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Couldn't determine game version.";
		return;
	}

	FirstAddr = GameVersion == SH2V_10 ?
		(float*)0x1FB2690 :
		GameVersion == SH2V_11 ?
		(float*)0x1FB6290 :
		(float*)0x1FB5290;

	SecondAddr = GameVersion == SH2V_10 ?
		(float*)0x1FB295C :
		GameVersion == SH2V_11 ?
		(float*)0x1FB655C :
		(float*)0x1FB555C;
}