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

injector::hook_back<int32_t(__cdecl*)(int32_t, float, DWORD)> orgPlaySoundFun;

int16_t LastOptionsSelectedItem;
int8_t LastOptionsPage;
int8_t LastOptionsSubPage;


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
		GetTransitionState() == 0)
	{
		
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

	orgPlaySoundFun.fun = (int32_t(__cdecl*)(int32_t, float, DWORD))0x00515580; //TODO
}