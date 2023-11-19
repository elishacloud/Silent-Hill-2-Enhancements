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
#include "Common\FileSystemHooks.h"
#include "Logging\Logging.h"

HRESULT PlayWavFile(const char*);

void RunPlayAdditionalSounds()
{
	static BYTE* CanUseFlashlight =
		GameVersion == SH2V_10 ? (BYTE*)0x01F7DC2C :
		GameVersion == SH2V_11 ? (BYTE*)0x01F8182C :
		GameVersion == SH2V_DC ? (BYTE*)0x01F8082C : nullptr;

	static BYTE* WorldColorR =
		GameVersion == SH2V_10 ? (BYTE*)0x00942C50 :
		GameVersion == SH2V_11 ? (BYTE*)0x00946850 :
		GameVersion == SH2V_DC ? (BYTE*)0x00945850 : nullptr;

	static BYTE* WorldColorG =
		GameVersion == SH2V_10 ? (BYTE*)0x00942C51 :
		GameVersion == SH2V_11 ? (BYTE*)0x00946851 :
		GameVersion == SH2V_DC ? (BYTE*)0x00945851 : nullptr;

	static BYTE* WorldColorB =
		GameVersion == SH2V_10 ? (BYTE*)0x00942C52 :
		GameVersion == SH2V_11 ? (BYTE*)0x00946852 :
		GameVersion == SH2V_DC ? (BYTE*)0x00945852 : nullptr;


	BYTE EventIndex = GetEventIndex();
	BYTE FlashLightSwitch = GetFlashlightSwitch();
	static BYTE LastFlashLightSwitch = FlashLightSwitch;

	if (LastFlashLightSwitch != FlashLightSwitch && (*CanUseFlashlight == 1 &&
		(GetRoomID() != 0x24 /*Angela apt room*/ && GetRoomID() != 0x89 /*Maria in prison*/ && GetRoomID() != 0x8F /*Eddie boss room 1*/ && GetRoomID() != 0x90 /*Eddie boss room 2*/ && GetRoomID() != 0xA2  /*Hotel room 302*/)) ||
		(LastFlashLightSwitch != FlashLightSwitch && (GetRoomID() == 0x04 /*Town East*/ || GetRoomID() == 0x08 /*Town West*/) && (WorldColorR == 0 && WorldColorG == 0 && WorldColorB == 0)))
	{
		// play flashlight_off.wav
		if (FlashLightSwitch == 0)
		{
			PlayWavFile((std::string(GetModPath("")) + "\\sound\\extra\\flashlight_off.wav").c_str());
		}
		// play flashlight_on.wav
		else if (FlashLightSwitch == 1)
		{
			PlayWavFile((std::string(GetModPath("")) + "\\sound\\extra\\flashlight_on.wav").c_str());
		}
	}
	LastFlashLightSwitch = FlashLightSwitch;
}
