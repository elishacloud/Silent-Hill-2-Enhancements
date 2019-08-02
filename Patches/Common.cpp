/**
* Copyright (C) 2019 Elisha Riedlinger
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

// Variables
void *RoomIDAddr = nullptr;
void *CutsceneIDAddr = nullptr;
void *CutscenePosAddr = nullptr;
void *JamesPosAddr = nullptr;
BYTE *FlashLightRenderAddr = nullptr;

void *GetRoomIDPointer()
{
	if (RoomIDAddr)
	{
		return RoomIDAddr;
	}

	// Predefined code bytes
	constexpr BYTE RoomIDSearchBytes[]{ 0x83, 0xF8, 0x04, 0x0F, 0x87, 0xCE, 0x00, 0x00, 0x00 };
	constexpr BYTE RoomCallBytes[]{ 0x83, 0x3D };

	// Get room ID address
	void *RoomFunctAddr = CheckMultiMemoryAddress((void*)0x0052A4A0, (void*)0x0052A7D0, (void*)0x0052A0F0, (void*)RoomIDSearchBytes, sizeof(RoomIDSearchBytes));

	// Search for address
	if (!RoomFunctAddr)
	{
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		RoomFunctAddr = GetAddressOfData(RoomIDSearchBytes, sizeof(DWORD), 1, 0x00529F39, 1800);
	}

	// Checking address pointer
	if (!RoomFunctAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find room ID function address!";
		return nullptr;
	}
	RoomFunctAddr = (void*)((DWORD)RoomFunctAddr + 0xD7);

	// Check address
	if (!CheckMemoryAddress(RoomFunctAddr, (void*)RoomCallBytes, sizeof(RoomCallBytes)))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return nullptr;
	}
	RoomFunctAddr = (void*)((DWORD)RoomFunctAddr + 0x02);

	memcpy(&RoomIDAddr, RoomFunctAddr, sizeof(DWORD));

	return RoomIDAddr;
}

void *GetCutsceneIDPointer()
{
	if (CutsceneIDAddr)
	{
		return CutsceneIDAddr;
	}

	// Predefined code bytes
	constexpr BYTE CutsceneIDSearchBytes[]{ 0x8B, 0x56, 0x08, 0x89, 0x10, 0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x50, 0xC3 };
	constexpr BYTE CutsceneCallBytes[]{ 0xA1 };

	// Get cutscene ID address
	void *CutsceneFunctAddr = CheckMultiMemoryAddress((void*)0x004A0293, (void*)0x004A0543, (void*)0x0049FE03, (void*)CutsceneIDSearchBytes, sizeof(CutsceneIDSearchBytes));

	// Search for address
	if (!CutsceneFunctAddr)
	{
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		CutsceneFunctAddr = GetAddressOfData(CutsceneIDSearchBytes, sizeof(CutsceneIDSearchBytes), 1, 0x0049FBA5, 2600);
	}

	// Checking address pointer
	if (!CutsceneFunctAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find cutscene ID function address!";
		return nullptr;
	}
	CutsceneFunctAddr = (void*)((DWORD)CutsceneFunctAddr + 0x1D);

	// Check address
	if (!CheckMemoryAddress(CutsceneFunctAddr, (void*)CutsceneCallBytes, sizeof(CutsceneCallBytes)))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return nullptr;
	}
	CutsceneFunctAddr = (void*)((DWORD)CutsceneFunctAddr + 0x01);

	memcpy(&CutsceneIDAddr, CutsceneFunctAddr, sizeof(DWORD));

	return CutsceneIDAddr;
}

void *GetCutscenePosPointer()
{
	if (CutscenePosAddr)
	{
		return CutscenePosAddr;
	}

	// Predefined code bytes
	constexpr BYTE CutscenePosSearchBytes[]{ 0x40, 0x88, 0x54, 0x24, 0x0B, 0x88, 0x4C, 0x24, 0x0A, 0x8B, 0x4C, 0x24, 0x08, 0x8B, 0xD1, 0x89, 0x0D };

	// Get cutscene Pos address
	void *CutsceneFunctAddr = CheckMultiMemoryAddress((void*)0x004A04DB, (void*)0x004A078B, (void*)0x004A004B, (void*)CutscenePosSearchBytes, sizeof(CutscenePosSearchBytes));

	// Search for address
	if (!CutsceneFunctAddr)
	{
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		CutsceneFunctAddr = GetAddressOfData(CutscenePosSearchBytes, sizeof(CutscenePosSearchBytes), 1, 0x0049FEAA, 1800);
	}

	// Checking address pointer
	if (!CutsceneFunctAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find cutscene Pos function address!";
		return nullptr;
	}
	CutsceneFunctAddr = (void*)((DWORD)CutsceneFunctAddr + 0x11);

	memcpy(&CutscenePosAddr, CutsceneFunctAddr, sizeof(DWORD));

	return CutscenePosAddr;
}

void *GetJamesPosPointer()
{
	if (JamesPosAddr)
	{
		return JamesPosAddr;
	}

	// Predefined code bytes
	constexpr BYTE JamesPosSearchBytes[]{ 0x4A, 0x8D, 0x88, 0xCC, 0x02, 0x00, 0x00, 0x89, 0x88, 0x94, 0x01, 0x00, 0x00, 0x8B, 0xC1, 0x75, 0xEF, 0x33, 0xC9, 0x89, 0x88, 0x94, 0x01, 0x00, 0x00, 0xB8 };

	// Get James Pos address
	void *JamesPosition = (float*)ReadSearchedAddresses(0x00538070, 0x005383A0, 0x00537CC0, JamesPosSearchBytes, sizeof(JamesPosSearchBytes), -0x10);

	// Checking address pointer
	if (!JamesPosition)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find James Pos function address!";
		return nullptr;
	}
	JamesPosAddr = (float*)((DWORD)JamesPosition + 0x1C);

	return JamesPosAddr;
}

BYTE *GetFlashLightRenderPointer()
{
	if (FlashLightRenderAddr)
	{
		return FlashLightRenderAddr;
	}

	// Predefined code bytes
	constexpr BYTE FlashLightRenderSearchBytes[]{ 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x33, 0xC0, 0x66, 0xA3 };

	// Get address for flashlight render
	FlashLightRenderAddr = (BYTE*)ReadSearchedAddresses(0x0050A1D6, 0x0050A506, 0x00509E26, FlashLightRenderSearchBytes, sizeof(FlashLightRenderSearchBytes), 0x14);

	// Checking address pointer
	if (!FlashLightRenderAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find James Pos function address!";
		return nullptr;
	}

	return FlashLightRenderAddr;
}
