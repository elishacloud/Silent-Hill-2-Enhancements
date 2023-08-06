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

bool OptionsOrMovieMenuChanged();
bool PauseSelectionChanged();

int16_t LastOptionsSelectedItem;
int8_t LastOptionsPage;
int8_t LastOptionsSubPage;

int8_t LastPauseSelection;
int8_t LastPauseQuitIndex;

BYTE* StoredOptionsResolutionAddr = nullptr;
int8_t StoredOptionsResolution = NULL;

BYTE* ConfirmAdvancedOptionsReturn = nullptr;
BYTE* DiscardAdvancedOptionsReturn = nullptr;

bool PlayConfirmSound = false;
bool PlayCancelSound = false;

__declspec(naked) void __stdcall ConfirmAdvancedOptions()
{
	PlayConfirmSound = true;

	__asm
	{
		mov byte ptr[StoredOptionsResolutionAddr], al

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

	// Save the current stored opt resolution to use in asm when discarding
	StoredOptionsResolution = *StoredOptionsResolutionAddr;

	if ((OptionsOrMovieMenuChanged() || PauseSelectionChanged()) &&
		GetTransitionState() == 0x00)
	{	
		// Play change selection sound
		orgPlaySoundFun.fun(0x2710, 1.0f, 0);
	} 
	else if (PlayCancelSound)
	{
		// Play cancel selection sound
		orgPlaySoundFun.fun(0x2713, 1.0f, 0);
		PlayCancelSound = false;
	}
	else if (PlayConfirmSound)
	{
		// Play confirm selection sound
		orgPlaySoundFun.fun(0x2712, 1.0f, 0);
		PlayConfirmSound = false;
	}
}

void PatchMenuSounds()
{
	Logging::Log() << "Patching Menu Sounds...";

	LastOptionsSelectedItem = GetSelectedOption();
	LastOptionsPage = GetOptionsPage();
	LastOptionsSubPage = GetOptionsSubPage();

	constexpr BYTE ConfirmAdvancedOptionsSearchBytes[]{ 0x75, 0x0E, 0x83, 0xFF, 0x04, 0x74, 0x09 };
	BYTE* ConfirmAdvancedOptionsAddr = (BYTE*)SearchAndGetAddresses(0x00464DCB, 0x0046505B, 0x0046526B, ConfirmAdvancedOptionsSearchBytes, sizeof(ConfirmAdvancedOptionsSearchBytes), 0x64, __FUNCTION__);

	if (!ConfirmAdvancedOptionsAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find ConfirmAdvancedOptions address!";
		return;
	}

	BYTE* DiscardAdvancedOptionsAddr = ConfirmAdvancedOptionsAddr + 0xFA;

	ConfirmAdvancedOptionsReturn = ConfirmAdvancedOptionsAddr + 0x05;
	DiscardAdvancedOptionsReturn = DiscardAdvancedOptionsAddr + 0x06;

	constexpr BYTE OptionsChangedSoundSearchBytes[]{ 0xBF, 0x0F, 0x00, 0x00, 0x00, 0x2B, 0xFD };
	DWORD* OptionsChangedSoundAddr = (DWORD*)SearchAndGetAddresses(0x0045FDC3, 0x00460023, 0x00460023, OptionsChangedSoundSearchBytes, sizeof(OptionsChangedSoundSearchBytes), -0x67, __FUNCTION__);

	if (!OptionsChangedSoundAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find OptionsChangedSound address!";
		return;
	}

	constexpr BYTE PauseSelectionChangedSearchBytes[]{ 0xF7, 0xD8, 0x1B, 0xC0, 0x40, 0x50 };
	BYTE* PauseSelectionChangedAddr = (BYTE*)SearchAndGetAddresses(0x0040276E, 0x0040276E, 0x0040276E, PauseSelectionChangedSearchBytes, sizeof(PauseSelectionChangedSearchBytes), -0x0A, __FUNCTION__);

	if (!PauseSelectionChangedAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find PauseSelectionChanged address!";
		return;
	}

	DWORD TempOptionsRes;
	memcpy(&TempOptionsRes, ConfirmAdvancedOptionsAddr + 0x01, sizeof(DWORD));

	StoredOptionsResolutionAddr = (BYTE*)TempOptionsRes;

	// Play Sound address, same for all binaries
	uint32_t* PlaySoundAddress = (uint32_t*)0x0040282E;

	// Get a reference to the PlaySound function
	orgPlaySoundFun.fun = injector::GetBranchDestination(PlaySoundAddress).get();

	// NOP the game's own sounds on changed option
	UpdateMemoryAddress(OptionsChangedSoundAddr, "\x90\x90\x90\x90\x90", 0x05);

	// NOP the game's own sounds on changed pause selection
	UpdateMemoryAddress(PauseSelectionChangedAddr, "\x90\x90\x90\x90\x90", 0x05);
	UpdateMemoryAddress(PauseSelectionChangedAddr + 0x01D0, "\x90\x90\x90\x90\x90", 0x05);
	UpdateMemoryAddress(PauseSelectionChangedAddr + 0x021C, "\x90\x90\x90\x90\x90", 0x05);

	// Hook instructions called when confirming and discarding advanced options to play sounds
	WriteJMPtoMemory(ConfirmAdvancedOptionsAddr, ConfirmAdvancedOptions, 0x05);
	WriteJMPtoMemory(DiscardAdvancedOptionsAddr, DiscardAdvancedOptions, 0x06);
}

bool OptionsOrMovieMenuChanged()
{
	int8_t OptionsPage = GetOptionsPage();
	int8_t OptionsSubPage = GetOptionsSubPage();
	int16_t SelectedOption = GetSelectedOption();

	bool result = ((OptionsPage == 8 && OptionsSubPage == 0) || // In Movies Menu
		(OptionsPage == 2 && OptionsSubPage == 0 || OptionsSubPage == 1) || // Subpage 0 = main options, 1 = game options
		(OptionsPage == 7 && OptionsSubPage == 0)) &&
		!(LockScreenPosition && OptionsPage == 7 && OptionsSubPage == 0 && // In advanced options
			SelectedOption == 1) && // "Screen Position" option is selected
		SelectedOption != LastOptionsSelectedItem &&
		OptionsPage == LastOptionsPage &&
		OptionsSubPage == LastOptionsSubPage;

	LastOptionsSelectedItem = SelectedOption;
	LastOptionsPage = OptionsPage;
	LastOptionsSubPage = OptionsSubPage;

	return result;
}

bool PauseSelectionChanged()
{
	int8_t PauseMenuSelection = GetPauseMenuButtonIndex();
	int8_t PauseMenuQuitIndex = GetPauseMenuQuitIndex();

	bool result = GetEventIndex() == 0x10 && GetMenuEvent() == 0x0D &&
		(LastPauseQuitIndex != PauseMenuQuitIndex || LastPauseSelection != PauseMenuSelection);

	LastPauseQuitIndex = PauseMenuQuitIndex;
	LastPauseSelection = PauseMenuSelection;

	return result;
}