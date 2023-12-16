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
DWORD EscPressAddress;
void *jmpCutsceneEscReturnAddr;
void *jmpCutsceneKeyPressReturnAddr;

// ASM function to disables Esc key during in-game cutscenes
__declspec(naked) void __stdcall CutsceneEscASM()
{
	__asm
	{
		push eax
		movzx eax, byte ptr ds : [IsInFakeFadeout]
		test eax, eax
		pop eax
		jnz near ExitAsm
		mov eax, dword ptr ds : [EscPressAddress]
		mov eax, dword ptr ds : [eax]

	ExitAsm:
		jmp dword ptr ds : [jmpCutsceneEscReturnAddr]
	}
}

// ASM function to disable all key presses besides Esc key
__declspec(naked) void __stdcall CutsceneKeyPressASM()
{
	__asm
	{
		add esp, 8
		push eax
		movzx eax, byte ptr ds : [IsInFakeFadeout]
		test eax, eax
		pop eax
		jnz near ExitAsm
		mov [esi + ebx * 0x02], ax

	ExitAsm:
		jmp dword ptr ds : [jmpCutsceneKeyPressReturnAddr]
	}
}

// Run SH2 code to Fix an issue where the game will hang when Esc is pressed while transition is active
void RunHangOnEsc()
{
	static bool RunOnlyOnce = true;
	if (RunOnlyOnce)
	{
		RunOnlyOnce = false;

		// Get Esc address
		constexpr BYTE EscSearchBytes[]{ 0x8B, 0x44, 0x24, 0x04, 0x8D, 0x04, 0x40, 0xC1, 0xE0, 0x05, 0x0F, 0xBF, 0x80 };
		DWORD EscAddress = SearchAndGetAddresses(0x00457B90, 0x00457DF0, 0x00457DF0, EscSearchBytes, sizeof(EscSearchBytes), -0x30, __FUNCTION__);
		if (!EscAddress)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
		jmpCutsceneEscReturnAddr = (void*)(EscAddress + 0x05);
		EscPressAddress = *(DWORD*)(EscAddress + 0x01);

		// Get key press address
		constexpr BYTE KeyPressSearchBytes[]{ 0x57, 0x8B, 0xF8, 0x03, 0xF7, 0x6B, 0xF6, 0x54, 0x81, 0xC6 };
		DWORD KeyPressAddress = SearchAndGetAddresses(0x00446149, 0x004462E9, 0x004462E9, KeyPressSearchBytes, sizeof(KeyPressSearchBytes), 0x2A, __FUNCTION__);
		if (!KeyPressAddress)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
		jmpCutsceneKeyPressReturnAddr = (void*)(KeyPressAddress + 0x07);

		// Update SH2 code
		WriteJMPtoMemory((BYTE*)EscAddress, *CutsceneEscASM, 5);
		WriteJMPtoMemory((BYTE*)KeyPressAddress, *CutsceneKeyPressASM, 7);
	}

	static DWORD Address = NULL;
	if (!Address)
	{
		RUNONCE();

		constexpr BYTE SearchBytes[]{ 0x8B, 0x10, 0x6A, 0x00, 0x6A, 0x1B, 0x50, 0xFF, 0x92, 0xC8, 0x00, 0x00, 0x00, 0x81, 0xC4, 0x68, 0x01, 0x00, 0x00, 0xC3 };
		Address = ReadSearchedAddresses(0x0044C615, 0x0044C7B5, 0x0044C7B5, SearchBytes, sizeof(SearchBytes), 0x26, __FUNCTION__);
		if (!Address)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
		Address += 0x04;
	}

	RUNCODEONCE(Logging::Log() << "Fixing Esc while transition is active...");

	// Prevent player from pressing Esc while transition is active
	if (*(DWORD*)Address != 0x03 && (GetTransitionState() == FADE_FROM_BLACK || (IsInBloomEffect && GetRoomID() == R_EDI_BOSS_RM_2) || IsInFakeFadeout))
	{
		DWORD Value = 3;
		UpdateMemoryAddress((void*)Address, &Value, sizeof(DWORD));
	}
}
