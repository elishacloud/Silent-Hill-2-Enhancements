/**
* Copyright (C) 2021 Elisha Riedlinger
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

void PatchBinary()
{
	// Find address for call code
	constexpr BYTE DCCallSearchBytes[] = { 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x68, 0xE8, 0x03, 0x00, 0x00, 0xE8 };
	void *DCCallPatchAddr = (void*)SearchAndGetAddresses(0x00408A29, 0x00408BD9, 0x00408BE9, DCCallSearchBytes, sizeof(DCCallSearchBytes), 0x32);

	// Address found
	if (!DCCallPatchAddr || !(CheckMemoryAddress(DCCallPatchAddr, "\xE8", 0x01) || CheckMemoryAddress(DCCallPatchAddr, "\xB8", 0x01)))
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Find address for health cross animation
	constexpr BYTE HealthAnimationSearchBytes[] = { 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x75, 0x08, 0xC7, 0x44, 0x24, 0x00, 0x00, 0x00, 0x80, 0x3F };
	void *HealthAnimationPatchAddr = (void*)SearchAndGetAddresses(0x00476374, 0x00476614, 0x00476824, HealthAnimationSearchBytes, sizeof(HealthAnimationSearchBytes), 0x24);

	// Address found
	if (!HealthAnimationPatchAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	constexpr BYTE TextLayer1SearchBytes[] = { 0x81, 0xC7, 0x0C, 0x01, 0x00, 0x00, 0x84, 0xC0, 0x6A, 0x62, 0x57, 0x74, 0x15, 0xA1 };
	DWORD TextLayer1Addr = SearchAndGetAddresses(0x004620F8, 0x0046235A, 0x0046235A, TextLayer1SearchBytes, sizeof(TextLayer1SearchBytes), 0x28);

	constexpr BYTE TextLayer2SearchBytes[] = { 0x83, 0xC4, 0x10, 0x84, 0xC0, 0x6A, 0x64, 0x8D, 0xB7, 0x0E, 0x01, 0x00, 0x00, 0x56, 0x74, 0x0B, 0x8B, 0x15 };
	DWORD TextLayer2Addr = SearchAndGetAddresses(0x00461EA3, 0x00462115, 0x00462115, TextLayer2SearchBytes, sizeof(TextLayer2SearchBytes), 0x20);

	DWORD WeaponArrowSpacingAddr = 0, ViewArrowSpacingAddr = 0;
	if (GameVersion != SH2V_DC)
	{
		constexpr BYTE ArrowSpacingSearchBytes[] = { 0x5E, 0x5D, 0x5B, 0x8B, 0x4C, 0x24, 0x48, 0x33, 0x4C, 0x24, 0x4C, 0x83, 0xC4, 0x4C, 0xE9 };
		WeaponArrowSpacingAddr = SearchAndGetAddresses(0x0046416E, 0x004643E7, 0x00000000, ArrowSpacingSearchBytes, sizeof(ArrowSpacingSearchBytes), 0xC2);
		ViewArrowSpacingAddr = WeaponArrowSpacingAddr + 0x38E;
	}
	else
	{
		constexpr BYTE ArrowSpacingSearchBytes[] = { 0x6A, 0x2F, 0x50, 0xEB, 0x09, 0x8B, 0x0D };
		WeaponArrowSpacingAddr = SearchAndGetAddresses(0x00000000, 0x00000000, 0x004646A1, ArrowSpacingSearchBytes, sizeof(ArrowSpacingSearchBytes), 0x0B);
		ViewArrowSpacingAddr = WeaponArrowSpacingAddr + 0x396;
	}

	// Address found
	if (!TextLayer1Addr || !TextLayer2Addr || !WeaponArrowSpacingAddr || !ViewArrowSpacingAddr ||
		*(WORD*)TextLayer1Addr != 0x466A || *(WORD*)TextLayer2Addr != 0x466A ||
		*(WORD*)WeaponArrowSpacingAddr != 0x466A || *(WORD*)ViewArrowSpacingAddr != 0x466A)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Patching binary...";
	// Weapon Control
	UpdateMemoryAddress((void*)(TextLayer1Addr + 0x01), "\x2E", 1);			// Text Layer 1 (Weapon Control)
	UpdateMemoryAddress((void*)(TextLayer2Addr + 0x01), "\x2E", 1);			// Text Layer 2 (Weapon Control)
	UpdateMemoryAddress((void*)(WeaponArrowSpacingAddr + 0x01), "\x2E", 1);		// Arrow Spacing (Weapon Control)
	// View Control
	UpdateMemoryAddress((void*)(TextLayer1Addr + 0x133), "\x2E", 1);		// Text Layer 1 (View Control)
	UpdateMemoryAddress((void*)(TextLayer2Addr + 0x104), "\x2E", 1);		// Text Layer 2 (View Control)
	UpdateMemoryAddress((void*)(ViewArrowSpacingAddr + 0x01), "\x2E", 1);		// Arrow Spacing (View Control)
	// Reverse health cross animation process
	UpdateMemoryAddress(HealthAnimationPatchAddr, "\x00\x00\x00\x00", 4);
	// From emoose binary
	UpdateMemoryAddress(DCCallPatchAddr, "\xB8", 1);
}
