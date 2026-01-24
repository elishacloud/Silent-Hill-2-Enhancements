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
constexpr int kUsedBlueGemFirstTimeGameFlag = 0x2D5;
constexpr int kUsedBlueGemSecondTimeGameFlag = 0x2D6;
constexpr int kUsedVideoTapeGameFlag = 0x1DB;
constexpr int kPlacedCopperRingGameFlag = 0xE3;
constexpr int kPlacedLeadRingGameFlag = 0xE4;
constexpr int kEnteredApartmentGate = 0x31;

BYTE* GameFlagPtr = 0;
void(*shDisplayControlEntry)(uint32_t*, uint32_t, int);
void(*shDisplayControlExec)(uint32_t*);
WORD* HotelRoom312MemoFlagAddr = 0;

void* jmpHospitalGardenHandlerReturnAddr1 = nullptr;
void* jmpHospitalGardenHandlerReturnAddr2 = nullptr;
void* jmpHotelRoom312HandlerReturnAddr1 = nullptr;
void* jmpHotelRoom312HandlerReturnAddr2 = nullptr;
void* jmpHospital3FHandlerReturnAddr1 = nullptr;
void* jmpHospital3FHandlerReturnAddr2 = nullptr;

bool IsGameFlagSet(int flag)
{
	return GameFlagPtr[flag >> 3] & (1 << (flag & 0x07));
}

// Prevents casting light out of the side of the elevator during the cutscene that takes place
// after the hospital chase.
void Hospital1FDisplayControl()
{
	if (GetRoomID() != R_HSP_ALT_LOBBY) return;

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

// Shows a hint in the hospital garden that suggests using the blue gem (first time).
void HospitalGardenDisplayControl()
{
	uint32_t display_list[] = { 0, 0, 0 };
	const bool has_blue_gem = (InventoryItemPointer()[0x07] & 0x40) > 0;
	shDisplayControlEntry(display_list, /*room=*/0xAA, /*no=*/has_blue_gem ? 0 : -1);
	shDisplayControlExec(display_list);
}

__declspec(naked) void __stdcall HospitalGardenHandlerASM()
{
	__asm
	{
		cmp eax, 0x49
		jnz ExitASM
		call HospitalGardenDisplayControl
		jmp jmpHospitalGardenHandlerReturnAddr1

	ExitASM:
		jmp jmpHospitalGardenHandlerReturnAddr2
	}
}

// Shows a hint on the shipping dock that suggests using the blue gem (second time).
void ShippingDockDisplayControl()
{
	uint32_t display_list[] = { 0, 0, 0 };
	shDisplayControlEntry(display_list, /*room=*/0x01, /*no=*/IsGameFlagSet(kUsedBlueGemFirstTimeGameFlag) ? 0 : -1);
	shDisplayControlExec(display_list);
}

__declspec(naked) void __stdcall ShippingDockHandlerASM()
{
	__asm
	{
		call ShippingDockDisplayControl
		ret
	}
}

// Shows a hint in Lakeview Hotel Room 312 that suggests using the blue gem (third time).
void HotelRoom312DisplayControl()
{
	uint32_t display_list[] = { 0, 0, 0 };
	const bool show_hint = IsGameFlagSet(kUsedBlueGemSecondTimeGameFlag) && !IsGameFlagSet(kUsedVideoTapeGameFlag);
	shDisplayControlEntry(display_list, /*room=*/0x5B, /*no=*/show_hint ? 0 : -1);
	shDisplayControlExec(display_list);

	// Disable memo text in Lakeview Hotel Room 312 while the hint is visible.
	*HotelRoom312MemoFlagAddr = show_hint ? kUsedBlueGemSecondTimeGameFlag : 0;
}

__declspec(naked) void __stdcall HotelRoom312HandlerASM()
{
	__asm
	{
		cmp eax, 0xA2
		mov dword ptr ds : [esp], 0x00
		jnz ExitASM
		call HotelRoom312DisplayControl
		jmp jmpHotelRoom312HandlerReturnAddr1

		ExitASM :
		jmp jmpHotelRoom312HandlerReturnAddr2
	}
}

// Shows the lead and copper rings on the hands of the Otherworld Hospital 3F
// stairwell door.
void Hospital3FDisplayControl()
{
	uint32_t display_list[] = { 0, 0, 0 };
	const bool placed_copper_ring = IsGameFlagSet(kPlacedCopperRingGameFlag);
	const bool placed_lead_ring = IsGameFlagSet(kPlacedLeadRingGameFlag);
	if (placed_copper_ring)
	{
		shDisplayControlEntry(display_list, /*room=*/0xD6, /*no=*/0);
	}
	if (placed_lead_ring)
	{
		shDisplayControlEntry(display_list, /*room=*/0xD6, /*no=*/1);
	}
	if (!placed_copper_ring && !placed_lead_ring)
	{
		shDisplayControlEntry(display_list, /*room=*/0xD6, /*no=*/-1);
	}
	shDisplayControlExec(display_list);
}

__declspec(naked) void __stdcall Hospital3FHandlerASM()
{
	__asm
	{
		push eax
		cmp eax, 0x57
		jne ExitASM
		call Hospital3FDisplayControl

	ExitASM:
		pop eax
		sub eax, 0x54
		jz ExitASM2
		jmp jmpHospital3FHandlerReturnAddr1

	ExitASM2:
		jmp jmpHospital3FHandlerReturnAddr2
	}
}

// Shows a hint in the motorhome before visiting the apartments.
void MotorhomeDisplayControl()
{
	if (GetRoomID() != R_MOTORHOME) return;

	uint32_t display_list[] = { 0, 0, 0 };
	shDisplayControlEntry(display_list, /*room=*/0x05, /*no=*/IsGameFlagSet(kEnteredApartmentGate) ? -1 : 0);
	shDisplayControlExec(display_list);
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

	const BYTE HospitalGardenHandlerSearchBytes[]{ 0x83, 0xF8, 0x49, 0x0F, 0x85, 0x4E, 0x01, 0x00, 0x00 };
	const DWORD HospitalGardenHandlerAddr = SearchAndGetAddresses(0x0058BFFC, 0x0058C8AC, 0x0058C1CC, HospitalGardenHandlerSearchBytes, sizeof(HospitalGardenHandlerSearchBytes), 0x00, __FUNCTION__);

	const BYTE ShippingDockHandlerSearchBytes[]{ 0x50, 0x68, 0xF0, 0x3C, 0x00, 0x00, 0xE8 };
	const DWORD ShippingDockHandlerAddr = SearchAndGetAddresses(0x0057ED1E, 0x0057F5CE, 0x0057EEEE, ShippingDockHandlerSearchBytes, sizeof(ShippingDockHandlerSearchBytes), 0x0F, __FUNCTION__);

	const BYTE HotelRoom312HandlerSearchBytes[] { 0x3D, 0xA2, 0x00, 0x00, 0x00, 0xC7, 0x44, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00 };
	const DWORD HotelRoom312HandlerAddr = SearchAndGetAddresses(0x00578788, 0x00579038, 0x00578958, HotelRoom312HandlerSearchBytes, sizeof(HotelRoom312HandlerSearchBytes), 0x00, __FUNCTION__);

	const BYTE Hospital3FHandlerSearchBytes[]{ 0x83, 0xE8, 0x54, 0x0F, 0x84, 0xBD, 0x00, 0x00, 0x00, 0x48 };
	const DWORD Hospital3FHandlerAddr = SearchAndGetAddresses(0x00589DE0, 0x0058A690, 0x00589FB0, Hospital3FHandlerSearchBytes, sizeof(Hospital3FHandlerSearchBytes), 0x00, __FUNCTION__);

	if (!StageDataAddr || !DisplayControlAddr || !GameFlagPtr || !BlueCreek1FHandlerAddr || !HospitalGardenHandlerAddr || !ShippingDockHandlerAddr || !HotelRoom312HandlerAddr || !Hospital3FHandlerAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	const DWORD Hospital1FStageDataAddr = *(DWORD*)(StageDataAddr + 0x96);
	const DWORD Hotel3FStageDataAddr = *(DWORD*)(StageDataAddr + 0x108);
	const DWORD MotorhomeStageDataAddr = *(DWORD*)(StageDataAddr + 0x18);
	jmpHospitalGardenHandlerReturnAddr1 = (void*)(HospitalGardenHandlerAddr + 0x09);
	jmpHospitalGardenHandlerReturnAddr2 = (void*)(HospitalGardenHandlerAddr + 0x157);
	jmpHotelRoom312HandlerReturnAddr1 = (void*)(HotelRoom312HandlerAddr + 0x0F);
	jmpHotelRoom312HandlerReturnAddr2 = (void*)(HotelRoom312HandlerAddr + 0x89);
	jmpHospital3FHandlerReturnAddr1 = (void*)(Hospital3FHandlerAddr + 0x09);
	jmpHospital3FHandlerReturnAddr2 = (void*)(Hospital3FHandlerAddr + 0xC6);
	const DWORD ShippingDockInjectAddr = *(DWORD*)(ShippingDockHandlerAddr) + ShippingDockHandlerAddr + 0xC4;
	HotelRoom312MemoFlagAddr = (WORD*)(*(DWORD*)Hotel3FStageDataAddr + 0x130);

	shDisplayControlEntry = (void(*)(uint32_t*, uint32_t, int))(DisplayControlAddr + 0x04 + *(DWORD*)DisplayControlAddr);
	shDisplayControlExec = (void(*)(uint32_t*))(DisplayControlAddr + 0x8C + *(DWORD*)(DisplayControlAddr + 0x88));

	Logging::Log() << "Patching various map mesh toggles...";

	// Use display control function directly for stages that do not have a handler function.
	DWORD* Hospital1FDisplayControlAddr = (DWORD*)Hospital1FDisplayControl;
	DWORD* MotorhomeDisplayControlAddr = (DWORD*)MotorhomeDisplayControl;
	UpdateMemoryAddress((void*)(Hospital1FStageDataAddr + 0x20), &Hospital1FDisplayControlAddr, sizeof(DWORD*));
	UpdateMemoryAddress((void*)(MotorhomeStageDataAddr + 0x20), &MotorhomeDisplayControlAddr, sizeof(DWORD*));

	// Inject custom display control into existing stage handlers.
	WriteJMPtoMemory((BYTE*)BlueCreek1FHandlerAddr, BlueCreek1FHandlerASM, 0x05);
	WriteJMPtoMemory((BYTE*)HospitalGardenHandlerAddr, HospitalGardenHandlerASM, 0x09);
	WriteJMPtoMemory((BYTE*)ShippingDockInjectAddr, ShippingDockHandlerASM, 0x05);
	WriteJMPtoMemory((BYTE*)HotelRoom312HandlerAddr, HotelRoom312HandlerASM, 0x0F);
	WriteJMPtoMemory((BYTE*)Hospital3FHandlerAddr, Hospital3FHandlerASM, 0x09);
}
