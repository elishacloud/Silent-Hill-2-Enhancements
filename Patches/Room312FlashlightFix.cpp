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
#include "Logging\Logging.h"

BYTE* InhibitFlashlightAddr = nullptr;

bool InhibitedFlashlightFlag = false;

void CheckRoom312Flashlight()
{
	if (!InhibitFlashlightAddr)
	{
		InhibitFlashlightAddr = GameVersion == SH2V_10 ? (BYTE*)0x0048EBC8 :
			GameVersion == SH2V_11 ? (BYTE*)0x0048EE68 :
			GameVersion == SH2V_DC ? (BYTE*)0x0048F078 :
			nullptr;

		if (!InhibitFlashlightAddr)
		{
			Logging::Log() << __FUNCTION__ << " Couldn't identify game version.";
			return;
		}

		Logging::Log() << __FUNCTION__ << " Initialized room 312 flashlight fix!";
	}

	bool InhibitFlashlight = GetRoomID() == R_HTL_RM_312 && (GetInventoryItem() & 0x80);

	if (InhibitFlashlight && !InhibitedFlashlightFlag)
	{
		UpdateMemoryAddress(InhibitFlashlightAddr, "\x90\xE9", 0x02);
		InhibitedFlashlightFlag = true;
	}
	else if (!InhibitFlashlight && InhibitedFlashlightFlag)
	{
		UpdateMemoryAddress(InhibitFlashlightAddr, "\x0F\x84", 0x02);
		InhibitedFlashlightFlag = false;
	}
}