/**
* Copyright (C) 2017 Elisha Riedlinger
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
#include "nocd_10.h"
#include "nocd_DC.h"
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
			Log() << "Updating CD check function memory addresses";

			// Write to memory
			*((DWORD *)((DWORD)CDCheckAddr)) = 0xC3;

			// Restore protection
			VirtualProtect(CDCheckAddr, 2, oldProtect, &oldProtect);
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

	// Find address for copy protection
	CDCheckAddr = GetAddressOfData(CDCopyProtection, 24, 2, 0x02000000);
	CDCheckAddr = (CDCheckAddr) ? (void*)((DWORD)CDCheckAddr - 15) : nullptr;

	// Address found
	if ((CDCheckAddr) ? (memcmp(CDCopyProtectionTest, CDCheckAddr, 6) == 0) : false)
	{
		// Log message
		Log() << "Found copy protection function at address: " << CDCheckAddr;

		void *CodeAddr = nullptr;
		void *EntryPointAddr = nullptr;
		const BYTE *CodeData;

		if (memcmp(CDCodeDatav10, CDCodeAddrv10, 32) == 0)
		{
			CodeAddr = CDCodeAddrv10;
			EntryPointAddr = EntryPointAddrv10;
			CodeData = &CDCodeDatav10[0];
		}
		else if (memcmp(CDCodeDatavDC, CDCodeAddrvDC, 32) == 0)
		{
			CodeAddr = CDCodeAddrvDC;
			EntryPointAddr = EntryPointAddrvDC;
			CodeData = &CDCodeDatavDC[0];
		}

		// Found code to be updated
		if (CodeAddr)
		{
			Log() << "Found code to be updated! " << CodeAddr;

			// Update code data
			DWORD oldProtect;
			if (VirtualProtect(CodeAddr, CDCodeSize + 8, PAGE_EXECUTE_READWRITE, &oldProtect))
			{
				Log() << "Updating code data memory";

				// Write to memory
				for (size_t x = 0; x < CDCodeSize; x++)
				{
					*((DWORD *)((DWORD)CodeAddr + x)) = CodeData[x];
				}

				// Restore protection
				VirtualProtect(CodeAddr, CDCodeSize + 8, oldProtect, &oldProtect);

				// Update code protection
				if (VirtualProtect(CDCheckAddr, 12, PAGE_EXECUTE_READWRITE, &oldProtect))
				{
					Log() << "Updating code protection memory";

					// Write to memory
					*((DWORD *)((DWORD)CDCheckAddr)) = 0xE9;	// jmp
					*((DWORD *)((DWORD)CDCheckAddr + 1)) = (DWORD)EntryPointAddr;

					// Restore protection
					VirtualProtect(CDCheckAddr, 12, oldProtect, &oldProtect);
				}
			}
		}
	}
	else
	{
		Log() << "Could not find copy protection function address in memory!";
	}
}
