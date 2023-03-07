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
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Variables for ASM
void *MemoBottomBrightnessAddr = nullptr;
void *MemoTopBrightnessAddr = nullptr;

// ASM functions to fix memo brightness
__declspec(naked) void __stdcall MemoBrightnessASM()
{
	__asm
	{
		push ecx
		mov ecx, dword ptr ds : [MemoBottomBrightnessAddr]
		mov dword ptr ds : [ecx], eax // Writes RGB for Bottom Layer
		mov ecx, dword ptr ds : [MemoTopBrightnessAddr]
		mov dword ptr ds : [ecx], eax // Writes RGB for Top Layer
		pop ecx
		retn
	}
}

// Patch SH2 code to fix memo brightness
void PatchMemoBrightnes()
{
	// Get memo brightness address
	constexpr BYTE MemoBrightnessSearchBytes[]{ 0x8B, 0xC6, 0xC1, 0xE0, 0x08, 0x0B, 0xC6, 0xC1, 0xE0, 0x08, 0x81, 0xE1, 0x00, 0x00, 0x00, 0xFF, 0x0B, 0xC1, 0x0B, 0xC6, 0x68 };
	DWORD MemoBrightnessAddr = SearchAndGetAddresses(0x0049914D, 0x004993FD, 0x00498CBD, MemoBrightnessSearchBytes, sizeof(MemoBrightnessSearchBytes), 0x22, __FUNCTION__);

	// Checking address pointer
	if (!MemoBrightnessAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	MemoBottomBrightnessAddr = (void*)*(DWORD*)(MemoBrightnessAddr + 1);
	MemoTopBrightnessAddr = (void*)((DWORD)MemoBottomBrightnessAddr + 0x80);

	Logging::Log() << "Fixing memo brightness...";
	WriteCalltoMemory((BYTE*)MemoBrightnessAddr, *MemoBrightnessASM);
}
