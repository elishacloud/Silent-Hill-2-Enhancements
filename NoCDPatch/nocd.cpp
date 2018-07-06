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
#include "nocd.h"
#include "..\Common\Utils.h"
#include "..\Common\Logging.h"

void DisableCDCheck()
{
	// Find address for CD check
	void *CDCheckAddr = GetAddressOfData(CDFuncBlock, 7, 4, 0x00401000, 0x00010000);

	// Address found
	if ((CDCheckAddr) ? (memcmp(CDBlockTest, ((DWORD *)((DWORD)CDCheckAddr + 11)), 8) == 0) : false)
	{
		// Log message
		Log() << "Found CD check function at address: " << CDCheckAddr;

		// Make memory writeable
		DWORD oldProtect;
		if (VirtualProtect(CDCheckAddr, 2, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			Log() << "Bypassing CD check...";

			// Write to memory
			*((DWORD *)((DWORD)CDCheckAddr)) = 0xC3;

			// Restore protection
			VirtualProtect(CDCheckAddr, 2, oldProtect, &oldProtect);

			// Flush cache
			FlushInstructionCache(GetCurrentProcess(), CDCheckAddr, 2);
		}
		else
		{
			Log() << "Could not write to memory!";
		}
	}
	else
	{
		Log() << "Could not find CD check function address in memory!";
	}
}
