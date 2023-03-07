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
#include "Logging\Logging.h"

// Variables for ASM
DWORD ConditionVal;
float PistonVal;
DWORD PistonList;
DWORD PistonItem;
void *jmpPistonReturnAddr;

// ASM functions to update Piston Position
__declspec(naked) void __stdcall PistonRoomASM()
{
	__asm
	{
		push ecx
		push eax
		mov eax, dword ptr ds : [CutsceneIDAddr] 		// moves cutscene ID to eax
		cmp dword ptr ds : [eax], 0x44
		jne near ConditionsNotMet						// jumps if not final "Abstract Daddy" cutscene
		mov ecx, PistonList
		lea eax, dword ptr ds : [esi * 4 + ecx]
		cmp eax, PistonItem
		jne near ConditionsNotMet						// jumps if not piston location address "01D83574"
		cmp ConditionVal, 0x01
		je near ConditionsMet							// jumps if all conditions have already been met
		mov eax, dword ptr ds : [CutscenePosAddr]		// moves cutscene ID to eax
		cmp dword ptr ds : [eax], 0x469E6CD4
		jne near ConditionsNotMet						// jumps if camera position x is not 20278.41

	ConditionsMet:
		pop eax
		mov ConditionVal, 0x01							// indicates all conditions have been met
		mov PistonVal, 0x40200000						// writes 2.50 float to address of your choice
		fadd PistonVal
		pop ecx
		jmp jmpPistonReturnAddr

	ConditionsNotMet:
		pop eax
		mov ConditionVal, 0x00							// indicates not all conditions have been met
		mov ecx, PistonList
		fadd dword ptr ds : [esi * 4 + ecx]				// original pointer
		pop ecx
		jmp jmpPistonReturnAddr
	}
}

// Patch SH2 code to Fix Piston Position
void PatchPistonRoom()
{
	// Get Piston address
	constexpr BYTE SearchBytesPiston[]{ 0x52, 0x8D, 0x44, 0x24, 0x10, 0x50, 0x56, 0x57, 0xE8 };
	DWORD PistonAddr = SearchAndGetAddresses(0x005814BA, 0x00581D6A, 0x0058168A, SearchBytesPiston, sizeof(SearchBytesPiston), 0x1F, __FUNCTION__);

	// Checking address pointer
	if (!PistonAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get Piston list address
	constexpr BYTE SearchBytesPistonList[]{ 0x3D, 0x8A, 0x00, 0x00, 0x00, 0x75, 0x2B, 0xBE, 0x70 };
	DWORD SearchAddress = SearchAndGetAddresses(0x00581326, 0x00581BD6, 0x005814F6, SearchBytesPistonList, sizeof(SearchBytesPistonList), 0x08, __FUNCTION__);

	// Checking address pointer
	if (!SearchAddress)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	memcpy(&PistonList, (void*)(SearchAddress), sizeof(DWORD));

	// Get addresses
	PistonItem = PistonList + 4;
	jmpPistonReturnAddr = (void*)(PistonAddr + 7);

	// Get cutscene ID address
	CutsceneIDAddr = GetCutsceneIDPointer();

	// Get cutscene camera position address
	CutscenePosAddr = GetCutscenePosPointer();

	// Checking address pointers
	if (!CutsceneIDAddr || !CutscenePosAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get cutscene ID or position address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Setting Piston Position Fix...";
	WriteJMPtoMemory((BYTE*)PistonAddr, *PistonRoomASM, 7);
}
