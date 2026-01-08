/**
* Copyright (C) 2026 Murugo
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

constexpr int kMetEddieGameFlag = 0x55;

BYTE* GameFlagPtr = 0;
void(*shDisplayControlEntry)(uint32_t*, uint32_t, int);
void(*shDisplayControlExec)(uint32_t*);

bool IsGameFlagSet(int flag)
{
	return GameFlagPtr[flag >> 3] & (1 << (flag & 0x07));
}

// Prevents casting light out of the side of the elevator during the cutscene that takes place
// after the hospital chase.
void Hospital1FDisplayControl()
{
	uint32_t display_list[] = { 0, 0, 0 };
	shDisplayControlEntry(display_list, /*room=*/0x97, /*no=*/GetCutsceneID() == CS_HSP_ALT_RPT_CHASE_3 ? 1 : 0);
	shDisplayControlExec(display_list);
}

// Shows a hint on the door to Blue Creek Apartments Room 109 if the player has not yet found
// Eddie.
void BlueCreek1FDisplayControl()
{
	uint32_t display_list[] = { 0, 0, 0 };
	shDisplayControlEntry(display_list, /*room=*/0x5C, /*no=*/IsGameFlagSet(kMetEddieGameFlag) ? 1 : 0);
	shDisplayControlExec(display_list);
}

__declspec(naked) void __stdcall BlueCreek1FHandlerASM()
{
	__asm
	{
		call BlueCreek1FDisplayControl
		add esp, 0x10
		ret
	}
}

// Toggles visibility of map meshes if certain conditions are met.
void PatchMapMeshToggle()
{
	const BYTE StageDataSearchBytes[]{ 0x48, 0x83, 0xF8, 0x39, 0x0F, 0x87, 0x63, 0x01, 0x00, 0x00 };
	const DWORD StageDataAddr = SearchAndGetAddresses(0x00494EC0, 0x00495160, 0x00495370, StageDataSearchBytes, sizeof(StageDataSearchBytes), 0x00, __FUNCTION__);

	const BYTE DisplayControlSearchBytes[]{ 0x6A, 0xFF, 0x8D, 0x44, 0x24, 0x08, 0x6A, 0x27, 0x50 };
	const DWORD DisplayControlAddr = SearchAndGetAddresses(0x0058BE95, 0x0058C745, 0x0058C065, DisplayControlSearchBytes, sizeof(DisplayControlSearchBytes), 0x0A, __FUNCTION__);

	constexpr BYTE GameFlagSearchBytes[]{ 0x83, 0xFE, 0x01, 0x55, 0x57, 0xBD, 0x00, 0x01, 0x00, 0x00 };
	GameFlagPtr = (BYTE*)ReadSearchedAddresses(0x0048AA9E, 0x0048AD3E, 0x0048AF4E, GameFlagSearchBytes, sizeof(GameFlagSearchBytes), 0x24, __FUNCTION__);

	const BYTE BlueCreek1FHandlerSearchBytes[]{ 0xC1, 0xE8, 0x0E, 0xF7, 0xD0, 0x83, 0xE0, 0x01 };
	const DWORD BlueCreek1FHandlerAddr = SearchAndGetAddresses(0x0059864A, 0x00598EFA, 0x0059881A, BlueCreek1FHandlerSearchBytes, sizeof(BlueCreek1FHandlerSearchBytes), 0x29, __FUNCTION__);

	if (!StageDataAddr || !DisplayControlAddr || !GameFlagPtr || !BlueCreek1FHandlerAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	const DWORD Hospital1FStageDataAddr = *(DWORD*)(StageDataAddr + 0x96);

	shDisplayControlEntry = (void(*)(uint32_t*, uint32_t, int))(DisplayControlAddr + 0x04 + *(DWORD*)DisplayControlAddr);
	shDisplayControlExec = (void(*)(uint32_t*))(DisplayControlAddr + 0x8C + *(DWORD*)(DisplayControlAddr + 0x88));

	Logging::Log() << "Patching various map mesh toggles...";

	DWORD* Hospital1FDisplayControlAddr = (DWORD*)Hospital1FDisplayControl;
	UpdateMemoryAddress((void*)(Hospital1FStageDataAddr + 0x20), &Hospital1FDisplayControlAddr, sizeof(DWORD*));
	WriteJMPtoMemory((BYTE*)BlueCreek1FHandlerAddr, BlueCreek1FHandlerASM, 0x05);
}
