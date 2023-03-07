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
DWORD TimerAddr;
DWORD TimerVal;

// ASM function to update to enable Alternate Stomp
__declspec(naked) void __stdcall AlternateStompASM()
{
	__asm
	{
		cmp cl, 0x07
		jne near ExitCode // jumps to ExitCode if not Light Stomp

	// Random Number Generator (RNG)
		push edx
		mov edx, dword ptr ds : [TimerAddr]
		fld dword ptr ds : [edx] // Loads Timer
		fistp dword ptr ds : [TimerVal] // Stores Timer as Integer
		mov dl, byte ptr ds : [TimerVal]
		and dl, 0x01
		pop edx
		je near ExitCode // jumps to ExitCode if TimerInt32 is not Even (50/50)

	// Enable Heavy Stomp
		mov cl, 0x03

	ExitCode:
		mov byte ptr ds : [eax + 0x00000493], cl
		ret
	}
}

// Update SH2 code to enable Alternate Stomp
void PatchAlternateStomp()
{
	// Get Alternate Stomp address
	constexpr BYTE SearchBytesAlternateStomp[]{ 0x84, 0xD2, 0x75, 0x0F, 0x80, 0xF9, 0xC0, 0x0F, 0x97, 0xC1, 0x80, 0xC1, 0x06, 0x88, 0x88, 0x93, 0x04, 0x00, 0x00 };
	DWORD AlternateStompAddr = SearchAndGetAddresses(0x005345D4, 0x00534904, 0x00534224, SearchBytesAlternateStomp, sizeof(SearchBytesAlternateStomp), 0x0D, __FUNCTION__);

	// Get Timer address
	constexpr BYTE SearchBytesTimer[]{ 0x8B, 0xC8, 0x99, 0xBE, 0x3C, 0x00, 0x00, 0x00, 0xF7, 0xFE, 0x52, 0x99, 0xF7, 0xFE, 0xB8 };
	TimerAddr = ReadSearchedAddresses(0x00446ED7, 0x00447077, 0x00447077, SearchBytesTimer, sizeof(SearchBytesTimer), -0x0C, __FUNCTION__);

	// Check memory addresses
	if (!AlternateStompAddr || !TimerAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Enabling Alternate Stomp...";
	WriteCalltoMemory((BYTE*)AlternateStompAddr, *AlternateStompASM, 0x06);
}
