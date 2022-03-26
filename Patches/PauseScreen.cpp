/**
* Copyright (C) 2022 Elisha Riedlinger
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
#include "Common\FileSystemHooks.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Common\Settings.h"

void *jmpOptionsExitAddr = nullptr;
void *PauseScreenCall1 = nullptr;
void *PauseScreenCall2 = nullptr;
void *PauseScreenCall3 = nullptr;

// ASM functions to return to gameplay after entering options menu from the pause screen
__declspec(naked) void __stdcall PauseScreenASM()
{
	__asm
	{
		call PauseScreenCall1 // fixes doubled text
		call PauseScreenCall2 // normal call used when returning to game play
		call PauseScreenCall3 // fixes incorrect text & enables sh2e shaders
		mov eax, 0x02
		retn
	}
}

void PatchPauseScreen()
{
	// Get Options exit address
	constexpr BYTE SearchBytesOptionsExit[]{ 0x08, 0x00, 0x00, 0x00, 0xEB, 0x0F, 0xB8, 0x10, 0x00, 0x00, 0x00, 0xA3 };
	BYTE *OptionsExitAddress = (BYTE*)SearchAndGetAddresses(0x004697AF, 0x00469A4F, 0x00469C5F, SearchBytesOptionsExit, sizeof(SearchBytesOptionsExit), 0x06);

	// Get call address
	constexpr BYTE SearchBytesGameCall[]{ 0x83, 0xC4, 0x0C, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x5B, 0xC3, 0x90 };
	DWORD GameCallAddr = SearchAndGetAddresses(0x00475839, 0x00475AD9, 0x00475CE9, SearchBytesGameCall, sizeof(SearchBytesGameCall), -0x5F);

	// Checking address pointer
	if (!OptionsExitAddress || !GameCallAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Set call addresses
	PauseScreenCall1 = (void*)(GameCallAddr + *(DWORD*)(GameCallAddr + 1) + 5);
	PauseScreenCall2 = (void*)(GameCallAddr + *(DWORD*)(GameCallAddr + 6) + 10);
	PauseScreenCall3 = (void*)(GameCallAddr + *(DWORD*)(GameCallAddr - 0xC0) - 0x9E);

	// Update SH2 code
	Logging::Log() << "Setting Pause Menu Fix...";
	WriteCalltoMemory(OptionsExitAddress, PauseScreenASM);
}
