/**
* Copyright (C) 2024 Elisha Riedlinger
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
void *jmpHotel312ReturnAddr;
DWORD HotelRoomObject;
void *jmpChairShadow1ReturnAddr;
void *jmpChairShadow2ReturnAddr;
void *jmpRoom312BloomReturnAddr;
DWORD Room312BloomObject;

// ASM function to update to Fix Hotel Room 312 Shadow Flicker
__declspec(naked) void __stdcall HotelRoom312ASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [RoomIDAddr]		// moves room ID pointer to eax
		cmp dword ptr ds : [eax], R_HTL_RM_312		// Hotel Room 312
		je near InHotelRoom

	// NotInHotelRoom
		mov eax, dword ptr ds : [HotelRoomObject]
		mov edx, [edx + eax]
		jmp near ExitASM

	InHotelRoom:
		mov edx, 0x00000000

	ExitASM:
		pop eax
		jmp jmpHotel312ReturnAddr
	}
}

// ASM function to update to Fix the Chair Shadow 1 in Room 312
__declspec(naked) void __stdcall ChairShadow1ASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [CutsceneIDAddr] 	// moves cutscene ID pointer to eax
		cmp dword ptr ds : [eax], CS_HTL_VHS_WATCH_AFTER
		pop eax
		je near IsHotelCutscene
		fstp dword ptr ds : [eax + 0x14]

	IsHotelCutscene:
		mov eax, [edi]
		jmp jmpChairShadow1ReturnAddr
	}
}

// ASM function to update to Fix the Chair Shadow 2 in Room 312
__declspec(naked) void __stdcall ChairShadow2ASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [CutsceneIDAddr] 	// moves cutscene ID pointer to eax
		cmp dword ptr ds : [eax], CS_HTL_VHS_WATCH_AFTER
		pop eax
		je near IsHotelCutscene
		fstp dword ptr ds : [eax + 0x14]

	IsHotelCutscene:
		lea eax, [esp + 0x90]
		jmp jmpChairShadow2ReturnAddr
	}
}

// ASM function to update to Fix the Room 312 Bloom Effect
__declspec(naked) void __stdcall Room312BloomASM()
{
	__asm
	{
		push eax
		mov eax, dword ptr ds : [RoomIDAddr]		// moves room ID pointer to eax
		cmp dword ptr ds : [eax], R_HTL_RM_312		// Hotel Room 312
		je near InHotelRoom
		mov eax, dword ptr ds : [Room312BloomObject]
		mov dword ptr ds : [eax], edi

	InHotelRoom:
		pop eax
		jmp jmpRoom312BloomReturnAddr
	}
}

// Set Room 312 to use correct resolution
void SetRoom312Resolution(void *WidthAddress)
{
	// Get Hotel Room 312 render texture width address
	constexpr BYTE SearchBytesRenderTexture[]{ 0x56, 0x6A, 0x00, 0x6A, 0x15, 0x8B, 0xCF, 0x6A, 0x01, 0xD3, 0xEB, 0x6A, 0x01, 0x53, 0x8B, 0x1D };
	static DWORD Room312RenderTexture = SearchAndGetAddresses(0x00476FCE, 0x0047726E, 0x0047747E, SearchBytesRenderTexture, sizeof(SearchBytesRenderTexture), 0x10, __FUNCTION__);
	if (!Room312RenderTexture)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get Hotel Room 312 viewport width address
	constexpr BYTE SearchBytesViewport[]{ 0x8B, 0x08, 0x52, 0x8B, 0x54, 0x24, 0x10, 0x52, 0x50, 0xFF, 0x51, 0x7C, 0xE8 };
	static DWORD Room312Viewport = SearchAndGetAddresses(0x00478F8E, 0x0047922E, 0x0047943E, SearchBytesViewport, sizeof(SearchBytesViewport), 0x26, __FUNCTION__);
	if (!Room312Viewport)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Setting Hotel Room 312 Resolution Fix...";
	void *WidthAddr = WidthAddress;
	UpdateMemoryAddress((void*)Room312RenderTexture, &WidthAddr, sizeof(DWORD));
	UpdateMemoryAddress((void*)Room312Viewport, &WidthAddr, sizeof(DWORD));
}

// Patch SH2 code to Fix Hotel Room 312 Shadow Flicker
void PatchRoom312ShadowFix()
{
	// Get Hotel Room 312 address
	constexpr BYTE SearchBytesHotelRoom312[]{ 0x8B, 0x08, 0x6A, 0x01, 0x6A, 0x0F, 0x50, 0xBB, 0x08, 0x00, 0x00, 0x00, 0xFF, 0x91, 0xC8, 0x00, 0x00, 0x00, 0xA1 };
	DWORD HotelRoom312Addr = SearchAndGetAddresses(0x005B018B, 0x005B0ABB, 0x005B03DB, SearchBytesHotelRoom312, sizeof(SearchBytesHotelRoom312), 0x34, __FUNCTION__);
	if (!HotelRoom312Addr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpHotel312ReturnAddr = (void*)(HotelRoom312Addr + 0x06);
	memcpy(&HotelRoomObject, (void*)(HotelRoom312Addr + 0x02), sizeof(DWORD));

	// Get Chair Shadow address
	constexpr BYTE SearchBytesChairShadow[]{ 0xD9, 0x58, 0x14, 0x8B, 0x07, 0x8B, 0x48, 0x3C, 0xD9, 0x41, 0x30, 0x83, 0xC1, 0x30 };
	DWORD ChairShadowAddr1 = SearchAndGetAddresses(0x005A7483, 0x005A7D33, 0x005A7653, SearchBytesChairShadow, sizeof(SearchBytesChairShadow), 0x00, __FUNCTION__);
	if (!ChairShadowAddr1)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpChairShadow1ReturnAddr = (void*)(ChairShadowAddr1 + 0x05);
	DWORD ChairShadowAddr2 = ChairShadowAddr1 + 0x30;
	jmpChairShadow2ReturnAddr = (void*)(ChairShadowAddr2 + 0x0A);

	// Get Room 312 Bloom Effect address
	constexpr BYTE SearchBytesRoom312Bloom[]{ 0xC6, 0x46, 0x10, 0x03, 0xC6, 0x46, 0x11, 0x19, 0xC6, 0x46, 0x12, 0xD8, 0xC6, 0x46, 0x14, 0x0C };
	DWORD Room312BloomAddr = SearchAndGetAddresses(0x00579011, 0x005798C1, 0x005791E1, SearchBytesRoom312Bloom, sizeof(SearchBytesRoom312Bloom), -0x10, __FUNCTION__);
	if (!Room312BloomAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpRoom312BloomReturnAddr = (void*)(Room312BloomAddr + 0x06);
	memcpy(&Room312BloomObject, (void*)(Room312BloomAddr + 0x02), sizeof(DWORD));

	// Fix buggy shadow behaviors
	constexpr BYTE SearchBytesBuggyShadows[]{ 0x8B, 0x08, 0x6A, 0x01, 0x6A, 0x07, 0x50, 0xFF, 0x91 };
	DWORD BuggyShadowsAddr = ReadSearchedAddresses(0x00578C02, 0x005794B2, 0x00578DD2, SearchBytesBuggyShadows, sizeof(SearchBytesBuggyShadows), 0x1f, __FUNCTION__);
	if (!BuggyShadowsAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	DWORD BuggyShadowsAddr1 = BuggyShadowsAddr + 0x20;
	DWORD BuggyShadowsAddr2 = BuggyShadowsAddr + 0x2C;

	// Get room ID address
	GetRoomIDPointer();

	// Get cutscene ID address
	GetCutsceneIDPointer();

	// Checking address pointer
	if (!RoomIDAddr || !CutsceneIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get room ID or cutscene ID address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Setting Hotel Room 312 Shadow Flicker Fix...";
	float Value = 18344.0f;
	UpdateMemoryAddress((void*)BuggyShadowsAddr1, &Value, sizeof(float));
	UpdateMemoryAddress((void*)BuggyShadowsAddr2, &Value, sizeof(float));
	WriteJMPtoMemory((BYTE*)HotelRoom312Addr, *HotelRoom312ASM, 0x06);
	WriteJMPtoMemory((BYTE*)ChairShadowAddr1, *ChairShadow1ASM, 0x05);
	WriteJMPtoMemory((BYTE*)ChairShadowAddr2, *ChairShadow2ASM, 0x0A);
	WriteJMPtoMemory((BYTE*)Room312BloomAddr, *Room312BloomASM, 0x06);
}
