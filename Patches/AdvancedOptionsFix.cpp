/**
* Copyright (C) 2022 Bruno Russi
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
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "External\injector\include\injector\injector.hpp"
#include "Patches\Patches.h"

static uint32_t* ptrConfirmationPromptState;
static uint32_t* ptrSelectionIndex;

bool HookedDrawTextOverlay = false;

bool isConfirmationPromptOpen()
{
	if (*(uint8_t*)(ptrConfirmationPromptState) == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int iSelectionIndex()
{
	return *(uint8_t*)(ptrSelectionIndex);
}

__int16(__cdecl* DrawTextOverlay_orig)(); // Originally called sub_480550
#pragma warning(suppress: 4740)
__int16 __declspec(naked) DrawTextOverlay_hook()
{
	if (iSelectionIndex() > 2 && iSelectionIndex() < 7) // if index is 3, 4, 5 or 6
	{
		// Don't render the 3D effect if the confirmation prompt is open
		if (isConfirmationPromptOpen() && FixAdvancedOptions)
		{
			__asm {ret}
		}
	}

	__asm {jmp DrawTextOverlay_orig}
}

void PatchAdvancedOptions()
{
	Logging::Log() << "Enabling Advanced Options fix...";

	// Get pointer to the state of the confirmation prompt (0 or 1)
	auto pattern = hook::pattern("A0 ? ? ? ? 84 C0 0F 85 ? ? ? ? E8 ? ? ? ? 8B 0D ? ? ? ? 68");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	ptrConfirmationPromptState = *pattern.count(1).get(0).get<uint32_t*>(1);

	// Get pointer to the selection index (0 to 9)
	pattern = hook::pattern("66 A1 ? ? ? ? 8B 0D ? ? ? ? 80 3D");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	ptrSelectionIndex = *pattern.count(1).get(0).get<uint32_t*>(2);

	// sub_480550 pointer
	pattern = hook::pattern("E8 ? ? ? ? E8 ? ? ? ? 8B 15 ? ? ? ? 68 8B 01 00 00 6A 46 68 D1 00 00 00");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	DrawTextOverlay_orig = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).get();

	// General option state (On/Off, etc)
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? A0 ? ? ? ? 84 C0 0F 85 ? ? ? ? 0F BF 05");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeCALL(pattern.count(1).get_first(0), DrawTextOverlay_hook, true);

	// "Noise Effect"
	pattern = hook::pattern("0F 85 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeNOP(pattern.count(1).get_first(0), 6, true); // Option state

	// "High Res. Textures"
	pattern = hook::pattern("0F 85 ? ? ? ? E8 ? ? ? ? 8B 0D ? ? ? ? 68");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeNOP(pattern.count(1).get_first(0), 6, true); // Nop jne instruction that disables everything

	pattern = hook::pattern("E8 ? ? ? ? E8 ? ? ? ? 8B 15 ? ? ? ? 68 8B 01 00 00 6A 46 68 D1 00 00 00");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeCALL(pattern.count(1).get_first(0), DrawTextOverlay_hook, true); // Option title

	// "Resolution"
	pattern = hook::pattern("0F 85 ? ? ? ? E8 ? ? ? ? 8B 15");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeNOP(pattern.count(1).get_first(0), 6, true); // Nop jne instruction that disables everything

	pattern = hook::pattern("E8 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 68 8B 01 00 00 6A 46 68 CA 00 00 00");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeCALL(pattern.count(1).get_first(0), DrawTextOverlay_hook, true); // Option title

	// "Shadows
	pattern = hook::pattern("0F 85 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 68");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeNOP(pattern.count(1).get_first(0), 6, true); // Nop jne instruction that changes the selection

	pattern = hook::pattern("E8 ? ? ? ? E8 ? ? ? ? 8B 0D ? ? ? ? 68 8B 01 00 00 6A 46 68 CB 00 00 00");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeCALL(pattern.count(1).get_first(0), DrawTextOverlay_hook, true); // Option title

	// "Fog"
	pattern = hook::pattern("75 ? E8 ? ? ? ? 8B 0D ? ? ? ? 68 04 01 00 00 68 F1 00 00 00 68 B2 00 00 00");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeNOP(pattern.count(1).get_first(0), 2, true); // Nop jne instruction that changes the selection

	pattern = hook::pattern("E8 ? ? ? ? E8 ? ? ? ? 8B 15 ? ? ? ? 68 8B 01 00 00 6A 46 68 CC 00 00 00");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	injector::MakeCALL(pattern.count(1).get_first(0), DrawTextOverlay_hook, true); // Option title
}