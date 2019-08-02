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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Variables for ASM
void *jmpClosetCutsceneReturnAddr;
DWORD ClosetCutsceneObject;
float f_ClosetCutsceneCameraPos = -21376.56445f;
DWORD ClosetCutsceneCameraPos = *(DWORD*)(&f_ClosetCutsceneCameraPos);

// ASM function to update to Fix RPT Apartment Closet Cutscene blur
__declspec(naked) void __stdcall ClosetCutsceneASM()
{
	__asm
	{
		push edx
		mov edx, dword ptr ds : [CutsceneIDAddr]	// moves cutscene ID pointer to edx
		cmp dword ptr ds : [edx], 0x0E
		jne near NotInCutscene
		push eax
		mov edx, dword ptr ds : [CutscenePosAddr]	// moves cutscene camera pos ID pointer to edx
		mov eax, dword ptr ds : [ClosetCutsceneCameraPos]
		cmp dword ptr ds : [edx], eax
		pop eax
		je near ExitASM

	NotInCutscene:
		mov edx, dword ptr ds : [ClosetCutsceneObject]
		mov dword ptr ds : [edx], eax

	ExitASM:
		pop edx
		jmp jmpClosetCutsceneReturnAddr
	}
}

// Update SH2 code to Fix RPT Apartment Closet Cutscene blur
void SetClosetCutscene()
{
	// Get Apartment Closet Cutscene address
	constexpr BYTE SearchBytesClosetCutscene[]{ 0xFF, 0xFF, 0x90, 0x90, 0x8A, 0x4C, 0x24, 0x08, 0xB8, 0x05, 0x00, 0x00, 0x00, 0xA3 };
	DWORD ClosetCutsceneAddr = SearchAndGetAddresses(0x0047832C, 0x004785CC, 0x004787DC, SearchBytesClosetCutscene, sizeof(SearchBytesClosetCutscene), 0x17);
	if (!ClosetCutsceneAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpClosetCutsceneReturnAddr = (void*)(ClosetCutsceneAddr + 0x05);
	memcpy(&ClosetCutsceneObject, (void*)(ClosetCutsceneAddr + 0x01), sizeof(DWORD));

	// Get cutscene ID address
	CutsceneIDAddr = GetCutsceneIDPointer();

	// Get cutscene camera pos address
	CutscenePosAddr = GetCutscenePosPointer();

	// Checking address pointer
	if (!CutscenePosAddr || !CutsceneIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get room ID or cutscene ID address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Setting RPT Apartment Closet Cutscene Fix...";
	WriteJMPtoMemory((BYTE*)ClosetCutsceneAddr, *ClosetCutsceneASM, 0x05);
}

// Update SH2 code to Fix RPT Apartment Closet Cutscene
void UpdateClosetCutscene(DWORD *SH2_CutsceneID, float *SH2_CutsceneCameraPos)
{
	// Update SH2 code to enable Lighting Transition fix
	static bool RunFixOnce = true;
	if (RunFixOnce)
	{
		RunFixOnce = false;
		SetClosetCutscene();
	}

	// Get Address1
	static DWORD Address1 = NULL;
	if (!Address1)
	{
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		constexpr BYTE SearchBytes[]{ 0x7C, 0xDC, 0xB0, 0x0C, 0x5F, 0xC6, 0x05 };

		// Get Room 312 Shadow address
		DWORD SearchAddress = (DWORD)CheckMultiMemoryAddress((void*)0x004799D4, (void*)0x00479C74, (void*)0x00479E84, (void*)SearchBytes, sizeof(SearchBytes));

		// Search for address
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " searching for memory address!";
			SearchAddress = (DWORD)GetAddressOfData(SearchBytes, sizeof(SearchBytes), 1, 0x004794D4, 2600);
		}

		// Checking address pointer
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}

		// Get address pointer
		SearchAddress = SearchAddress + 0x07;
		memcpy(&Address1, (void*)(SearchAddress), sizeof(DWORD));
	}

	// Get Address3
	static DWORD Address3 = NULL;
	if (!Address3)
	{
		static bool RunOnce = false;
		if (RunOnce)
		{
			return;
		}
		RunOnce = true;

		constexpr BYTE SearchBytes[]{ 0x5E, 0x83, 0xC4, 0x10, 0xC3, 0xC6, 0x46, 0x07, 0x00, 0x8B, 0x15 };

		// Get Room 312 Shadow address
		DWORD SearchAddress = (DWORD)CheckMultiMemoryAddress((void*)0x0043F9BF, (void*)0x0043FB7F, (void*)0x0043FB7F, (void*)SearchBytes, sizeof(SearchBytes));

		// Search for address
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " searching for memory address!";
			SearchAddress = (DWORD)GetAddressOfData(SearchBytes, sizeof(SearchBytes), 1, 0x0043F4BF, 2600);
		}

		// Checking address pointer
		if (!SearchAddress)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}

		// Get address pointer
		SearchAddress = SearchAddress - 0x1B;
		memcpy(&Address3, (void*)(SearchAddress), sizeof(DWORD));
	}

	// Get flashlight render address
	FlashLightRenderAddr = GetFlashLightRenderPointer();

	// Checking address pointer
	if (!FlashLightRenderAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get flashlight render address!";
		return;
	}

	// Darken & blur more closet bars
	static bool ValueSet1 = false;
	if (*SH2_CutsceneID == 0x0E && *SH2_CutsceneCameraPos == -21376.56445f)
	{
		if (!ValueSet1)
		{
			BYTE ByteValue = 15;
			UpdateMemoryAddress((void*)Address1, &ByteValue, sizeof(BYTE));		// "Time of Day"
		}
		ValueSet1 = true;
	}
	else if (ValueSet1)
	{
		BYTE ByteValue = 11;
		UpdateMemoryAddress((void*)Address1, &ByteValue, sizeof(BYTE));		// "Time of Day"
		ValueSet1 = false;
	}

	// Adjust room/dynamic objects for one shot
	static bool ValueSet2 = false;
	if (*SH2_CutsceneID == 0x0E  && *SH2_CutsceneCameraPos == -20133.99805f)
	{
		if (!ValueSet2)
		{
			float Value = -0.7f;
			UpdateMemoryAddress((void*)(Address3 + 0x00), &Value, sizeof(float));		// PC location textures + models color R (set 1)
			UpdateMemoryAddress((void*)(Address3 + 0x04), &Value, sizeof(float));		// PC location textures + models color G (set 1)
			UpdateMemoryAddress((void*)(Address3 + 0x08), &Value, sizeof(float));		// PC location textures + models color B (set 1)
			Value = 0.3f;
			UpdateMemoryAddress((void*)(Address3 + 0x20), &Value, sizeof(float));		// PC location textures + models color R (set 2)
			UpdateMemoryAddress((void*)(Address3 + 0x24), &Value, sizeof(float));		// PC location textures + models color G (set 2)
			UpdateMemoryAddress((void*)(Address3 + 0x28), &Value, sizeof(float));		// PC location textures + models color B (set 2)
		}
		ValueSet2 = true;
	}
	else if (ValueSet2)
	{
		float Value = 0.150000006f;
		UpdateMemoryAddress((void*)(Address3 + 0x00), &Value, sizeof(float));		// PC location textures + models color R (set 1)
		UpdateMemoryAddress((void*)(Address3 + 0x04), &Value, sizeof(float));		// PC location textures + models color G (set 1)
		UpdateMemoryAddress((void*)(Address3 + 0x08), &Value, sizeof(float));		// PC location textures + models color B (set 1)
		Value = 0.04500000179f;
		UpdateMemoryAddress((void*)(Address3 + 0x20), &Value, sizeof(float));		// PC location textures + models color R (set 2)
		UpdateMemoryAddress((void*)(Address3 + 0x24), &Value, sizeof(float));		// PC location textures + models color G (set 2)
		UpdateMemoryAddress((void*)(Address3 + 0x28), &Value, sizeof(float));		// PC location textures + models color B (set 2)
		ValueSet2 = false;
	}

	// Disable the flashlight during cutscene
	static bool ValueSet3 = false;
	if (*SH2_CutsceneID == 0x0E && *SH2_CutsceneCameraPos == -22256.61133f)
	{
		if (!ValueSet3)
		{
			*FlashLightRenderAddr = 0;													// Flashlight render
			float Value = 0.0f;
			UpdateMemoryAddress((void*)(Address3 + 0x60), &Value, sizeof(float));		// Flashlight light R
			UpdateMemoryAddress((void*)(Address3 + 0x64), &Value, sizeof(float));		// Flashlight light G
			UpdateMemoryAddress((void*)(Address3 + 0x68), &Value, sizeof(float));		// Flashlight light B
		}
		ValueSet3 = true;
	}
	else if (ValueSet3)
	{
		ValueSet3 = false;
	}
}
