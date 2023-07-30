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

BYTE* StoredOptionsResolution = nullptr;

BYTE* ConfirmAdvancedOptionsReturn = nullptr;
BYTE* DiscardAdvancedOptionsReturn = nullptr;

bool PlayConfirmSound = false;
bool PlayCancelSound = false;

__declspec(naked) void __stdcall ConfirmAdvancedOptions()
{
	PlayConfirmSound = true;

	__asm
	{
		mov byte ptr[StoredOptionsResolution], al //TODO check

		jmp ConfirmAdvancedOptionsReturn;
	}
}

__declspec(naked) void __stdcall DiscardAdvancedOptions()
{
	PlayCancelSound = true;

	__asm
	{
		mov dl, byte ptr[StoredOptionsResolution]

		jmp DiscardAdvancedOptionsReturn;
	}
}

void HandleMenuSounds()
{
	if (!MenuSoundsFix)
		return;

	int8_t OptionsPage = GetOptionsPage();
	int8_t OptionsSubPage = GetOptionsSubPage();
	int16_t SelectedOption = GetSelectedOption();

	if (((OptionsPage == 8 && OptionsSubPage == 0) || // In Movies Menu
		(OptionsPage == 2 && OptionsSubPage == 0 || OptionsSubPage == 1) || // Subpage 0 = main options, 1 = game options
		(OptionsPage == 7 && OptionsSubPage == 0)) &&
		!(LockScreenPosition && OptionsPage == 7 && OptionsSubPage == 0 && // In advanced options
		SelectedOption == 1) && // "Screen Position" option is selected
		SelectedOption != LastOptionsSelectedItem &&
		OptionsPage == LastOptionsPage &&
		OptionsSubPage == LastOptionsSubPage &&
		GetTransitionState() == 0x00)
	{	
		// Play change selection sound
		orgPlaySoundFun.fun(0x2710, 1.0f, 0);
	} 
	else if (PlayCancelSound)
	{
		orgPlaySoundFun.fun(0x2713, 1.0f, 0);
		PlayCancelSound = false;
	}
	else if (PlayConfirmSound)
	{
		orgPlaySoundFun.fun(0x2712, 1.0f, 0);
		PlayConfirmSound = false;
	}

	LastOptionsSelectedItem = SelectedOption;
	LastOptionsPage = OptionsPage;
	LastOptionsSubPage = OptionsSubPage;
}

void PatchMenuSounds()
{
	Logging::Log() << "Patching Menu Sounds...";

	LastOptionsSelectedItem = GetSelectedOption();
	LastOptionsPage = GetOptionsPage();
	LastOptionsSubPage = GetOptionsSubPage();

	BYTE* ConfirmAdvancedOptionsAddr = (BYTE*)0x00464e2f; //TODO addresses
	BYTE* DiscardAdvancedOptionsAddr = (BYTE*)0x00464F29;

	ConfirmAdvancedOptionsReturn = ConfirmAdvancedOptionsAddr + 0x05;
	DiscardAdvancedOptionsReturn = DiscardAdvancedOptionsAddr + 0x06;

	DWORD TempOptionsRes;
	memcpy(&TempOptionsRes, ConfirmAdvancedOptionsAddr + 0x01, sizeof(DWORD));

	StoredOptionsResolution = (BYTE*)TempOptionsRes;

	// Play Sound address, same for all binaries
	uint32_t* PlaySoundAddress = (uint32_t*)0x0040282E;

	// Get a reference to the PlaySound function
	orgPlaySoundFun.fun = injector::GetBranchDestination(PlaySoundAddress).get();

	// Hook instructions called when confirming and discarding advanced options to play sounds
	WriteJMPtoMemory(ConfirmAdvancedOptionsAddr, ConfirmAdvancedOptions, 0x05);
	WriteJMPtoMemory(DiscardAdvancedOptionsAddr, DiscardAdvancedOptions, 0x06);
}