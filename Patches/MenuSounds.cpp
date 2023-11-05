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

BYTE* ConfirmAdvancedOptionsReturn = nullptr;
BYTE* DiscardAdvancedOptionsReturn = nullptr;

BYTE* TargetOptionsPageAddr = nullptr;

bool PlayConfirmSound = false;
bool PlayCancelSound = false;

#pragma warning(suppress: 4740)
__declspec(naked) void __stdcall ConfirmAdvancedOptions()
{
	PlayConfirmSound = true;
	*TargetOptionsPageAddr = 0x01;

	__asm
	{
		jmp ConfirmAdvancedOptionsReturn;
	}
}

#pragma warning(suppress: 4740)
__declspec(naked) void __stdcall DiscardAdvancedOptions()
{
	PlayCancelSound = true;
	*TargetOptionsPageAddr = 0x01;

	__asm
	{
		jmp DiscardAdvancedOptionsReturn;
	}
}

void HandleMenuSounds()
{
	if (!MenuSoundsFix)
		return;

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
	BYTE* ConfirmAdvancedOptionsAddr = (BYTE*)SearchAndGetAddresses(0x00464DCB, 0x0046505B, 0x0046526B, ConfirmAdvancedOptionsSearchBytes, sizeof(ConfirmAdvancedOptionsSearchBytes), 0x147, __FUNCTION__);

	if (!ConfirmAdvancedOptionsAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find ConfirmAdvancedOptions address!";
		return;
	}

	TargetOptionsPageAddr = (BYTE*)ReadSearchedAddresses(0x00464DCB, 0x0046505B, 0x0046526B, ConfirmAdvancedOptionsSearchBytes, sizeof(ConfirmAdvancedOptionsSearchBytes), 0x149, __FUNCTION__);

	BYTE* DiscardAdvancedOptionsAddr = ConfirmAdvancedOptionsAddr + 0x66;

	ConfirmAdvancedOptionsReturn = ConfirmAdvancedOptionsAddr + 0x07;
	DiscardAdvancedOptionsReturn = DiscardAdvancedOptionsAddr + 0x07;

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

	constexpr BYTE MovieSelectionDecreasedSearchBytes[]{ 0x83, 0xC1, 0x09, 0x83, 0xC4, 0x0C, 0x3B, 0xD1 };
	BYTE* MovieSelectionDecreasedAddr = (BYTE*)SearchAndGetAddresses(0x00466491, 0x00466731, 0x00466941, MovieSelectionDecreasedSearchBytes, sizeof(MovieSelectionDecreasedSearchBytes), -0x12, __FUNCTION__);

	if (!MovieSelectionDecreasedAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find MovieSelectionDecreased address!";
		return;
	}

	constexpr BYTE PauseMenuQuitNoBytes[]{ 0x85, 0xC0, 0x74, 0x2B, 0x56, 0x68 };
	BYTE* PauseMenuQuitNoAddr = (BYTE*)SearchAndGetAddresses(0x004026E8, 0x004026E8, 0x004026E8, PauseMenuQuitNoBytes, sizeof(PauseMenuQuitNoBytes), 0x0B, __FUNCTION__);

	if (!PauseMenuQuitNoAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find PauseMenuQuitNo address!";
		return;
	}

	BYTE* MovieSelectionIncreasedAddr = MovieSelectionDecreasedAddr + 0x62;

	// Play Sound address, same for all binaries
	uint32_t* PlaySoundAddress = (uint32_t*)0x0040282E;

	// Get a reference to the PlaySound function
	orgPlaySoundFun.fun = injector::GetBranchDestination(PlaySoundAddress).get();

	// NOP the game's own sounds on changed option
	UpdateMemoryAddress(OptionsChangedSoundAddr, "\x90\x90\x90\x90\x90", 0x05);
	UpdateMemoryAddress(MovieSelectionDecreasedAddr, "\x90\x90\x90\x90\x90", 0x05);
	UpdateMemoryAddress(MovieSelectionIncreasedAddr, "\x90\x90\x90\x90\x90", 0x05);

	// NOP the game's own sounds on changed pause selection
	UpdateMemoryAddress(PauseSelectionChangedAddr, "\x90\x90\x90\x90\x90", 0x05);
	UpdateMemoryAddress(PauseSelectionChangedAddr + 0x01D0, "\x90\x90\x90\x90\x90", 0x05);
	UpdateMemoryAddress(PauseSelectionChangedAddr + 0x021C, "\x90\x90\x90\x90\x90", 0x05);

	// Replace the confirm sound with cancel sound when selecting No in the quit game screen
	UpdateMemoryAddress(PauseMenuQuitNoAddr, "\x13", 0x01);

	// Hook instructions called when confirming and discarding advanced options to play sounds
	WriteJMPtoMemory(ConfirmAdvancedOptionsAddr, ConfirmAdvancedOptions, 0x07);
	WriteJMPtoMemory(DiscardAdvancedOptionsAddr, DiscardAdvancedOptions, 0x07);
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
		OptionsSubPage == LastOptionsSubPage && 
		!(IsHardwareSoundEnabled() && OptionsPage == 7 && OptionsSubPage == 0 && SelectedOption == 9); // Avoid playing the sound if Hardware 3D Used option isn't active

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