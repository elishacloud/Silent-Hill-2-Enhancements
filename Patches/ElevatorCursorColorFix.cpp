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

// Hotel Employee Elevator Cursor Color
// 00 = Gray
// 01 = Blue
// 02 = Red
// 04 = Green

void* CursorColorPointerAddr = nullptr;
void* jmpReturnCursorColorBugFix = nullptr;

// ASM function to update cursor color
__declspec(naked) void __stdcall CursorColorBugFixASM()
{
	__asm
	{
		push ecx
		mov ecx, dword ptr ds : [CursorColorPointerAddr]
		mov eax, dword ptr ds : [ecx]
		and eax, 0xFF808080
		or eax, 0x00808080
		mov dword ptr ds : [ecx], eax
		pop ecx
		jmp jmpReturnCursorColorBugFix
	}
}

// Patch SH2 code to Fix Elevator Cursor Color
void PatchElevatorCursorColor()
{
	BYTE* ElevadorPushAddr = (BYTE*)(
		GameVersion == SH2V_10 ? 0x0057A671 :
		GameVersion == SH2V_11 ? 0x0057AF21 :
		GameVersion == SH2V_DC ? 0x0057A841 : NULL);

	BYTE* CursorColorASMAddr = (BYTE*)(
		GameVersion == SH2V_10 ? 0x004A2E29 :
		GameVersion == SH2V_11 ? 0x004A30D9 :
		GameVersion == SH2V_DC ? 0x004A2999 : NULL);

	// Check variable addresses
	if (!ElevadorPushAddr || !CursorColorASMAddr ||
		*(DWORD*)ElevadorPushAddr != 0xE856F633 || *(DWORD*)CursorColorASMAddr != 0x00A7850F)
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}
	jmpReturnCursorColorBugFix = CursorColorASMAddr + 0xAD;
	CursorColorPointerAddr = (BYTE*)*(DWORD*)(CursorColorASMAddr + 0x08);

	UpdateMemoryAddress(ElevadorPushAddr, "\x6A\x04\x90", 3);	// Push 4 NOP
	WriteJMPtoMemory(CursorColorASMAddr + 1, CursorColorBugFixASM);
	UpdateMemoryAddress(CursorColorASMAddr, "\x0F\x85", 2);	// JNE (jump not equal)
}
