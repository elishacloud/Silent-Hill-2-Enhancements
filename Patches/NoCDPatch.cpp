/**
* Copyright (C) 2018 Elisha Riedlinger
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
#include "..\Common\Utils.h"
#include "..\Common\Logging.h"

// Predefined code bytes
constexpr BYTE CDFuncBlock[] = { 0x81, 0xEC, 0x08, 0x04, 0x00, 0x00, 0xA1 };
constexpr BYTE CDBlockTest[] = { 0x33, 0x84, 0x24, 0x08, 0x04, 0x00, 0x00, 0x53 };

void DisableCDCheck()
{
	// Find address for CD check
	void *CDCheckAddr = GetAddressOfData(CDFuncBlock, sizeof(CDFuncBlock), 4, 0x00407DE0, 1800);

	// Address found
	if ((CDCheckAddr) ? (memcmp((void*)((DWORD)CDCheckAddr + 11), CDBlockTest, sizeof(CDBlockTest)) != 0) : true)
	{
		Log() << __FUNCTION__ << " Error: Could not find CD check function address in memory!";
		return;
	}

	// Make memory writeable
	DWORD oldProtect;
	if (!VirtualProtect(CDCheckAddr, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		Log() << __FUNCTION__ << " Error: Could not write to memory!";
		return;
	}

	Log() << "Bypassing CD check...";

	// Write to memory
	*((BYTE*)((DWORD)CDCheckAddr)) = 0xC3;

	// Restore protection
	VirtualProtect(CDCheckAddr, 1, oldProtect, &oldProtect);

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), CDCheckAddr, 1);
}
