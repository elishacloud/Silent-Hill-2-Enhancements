/**
* Copyright (C) 2023 mercury501
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

#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Patches\Patches.h"
#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

injector::hook_back<int32_t(__cdecl*)(int32_t, float, DWORD)> orgPlaySoundFun;

int16_t LastOptionsSelectedItem;
int8_t LastOptionsPage;
int8_t LastOptionsSubPage;

int32_t PlaySaveSound_Hook(int32_t SoundId, float volume, DWORD param3)
{
	//TODO  orgPlaySoundFun.fun(S_SAVE_GAME, volume, param3);
	Logging::Log() << __FUNCTION__;
	return 0x10;
}

int32_t PlayLoadSound_Hook(int32_t SoundId, float volume, DWORD param3)
{
	//TODO  orgPlaySoundFun.fun(S_SAVE_GAME, volume, param3);
	Logging::Log() << __FUNCTION__;
	return 0x10;
}

void HandleMenuSounds()
{
	if (!MenuSoundsFix)
		return;

	int8_t OptionsPage = GetOptionsPage();
	int8_t OptionsSubPage = GetOptionsSubPage();
	int16_t SelectedOption = GetSelectedOption();

	if (((OptionsPage == 8 && OptionsSubPage == 0 /*In Movies Menu*/) || 
		(OptionsPage == 2 && OptionsSubPage == 0 /*In Main Options */ || OptionsSubPage == 1 /*In Game Options*/) || 
		(OptionsPage == 7 && OptionsSubPage == 0 /*In Advanced Options*/)) &&
		!(LockScreenPosition && OptionsPage == 7 && OptionsSubPage == 0 /*In Advanced Options*/ && SelectedOption == 1 /*"Screen Position" option is selected*/) &&
		SelectedOption != LastOptionsSelectedItem &&
		OptionsPage == LastOptionsPage &&
		OptionsSubPage == LastOptionsSubPage &&
		GetTransitionState() == 0x00)
	{	
		// Play change selection sound
		orgPlaySoundFun.fun(0x2710, 1.0, 0);
	}

	LastOptionsSelectedItem = SelectedOption;
	LastOptionsPage = OptionsPage;
	LastOptionsSubPage = OptionsSubPage;
}

void PatchMenuSounds()
{
	LastOptionsSelectedItem = GetSelectedOption();
	LastOptionsPage = GetOptionsPage();
	LastOptionsSubPage = GetOptionsSubPage();

	// Play Sound address, same for all binaries
	uint32_t* PlaySoundAddress = (uint32_t*)0x0040282E;

	uint32_t* LoadGameSoundPauseMenu = (uint32_t*)0x00454A64; //TODO addresses
	uint32_t* LoadGameSoundNewGame = (uint32_t*)0x0049870c;
	uint32_t* LoadGameSoundContinue = (uint32_t*)0x00497f84;
	uint32_t* SaveGameSoundRedSquares = (uint32_t*)0x004476DB;

	// Hook load/save game sounds
	injector::MakeCALL(LoadGameSoundPauseMenu, PlayLoadSound_Hook, true);
	injector::MakeCALL(LoadGameSoundNewGame, PlayLoadSound_Hook, true);
	injector::MakeCALL(LoadGameSoundContinue, PlayLoadSound_Hook, true);
	injector::MakeCALL(SaveGameSoundRedSquares, PlaySaveSound_Hook, true);

	orgPlaySoundFun.fun = (int32_t(__cdecl*)(int32_t, float, DWORD))PlaySoundAddress;
}