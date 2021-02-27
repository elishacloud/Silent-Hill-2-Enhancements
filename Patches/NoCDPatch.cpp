/**
* Copyright (C) 2021 Elisha Riedlinger
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

void PatchCDCheck()
{
	// Check for CD patch
	constexpr BYTE CDCheckAddredBlock[] = { 0xEC, 0x08, 0x04, 0x00, 0x00, 0xA1 };
	void *CDCheckAddr = CheckMultiMemoryAddress((void*)0x00408761, (void*)0x004088C1, (void*)0x004088D1, (void*)CDCheckAddredBlock, sizeof(CDCheckAddredBlock));
	if (CDCheckAddr && !CheckMemoryAddress((void*)((DWORD)CDCheckAddr - 1), "\x81", 0x01, false))
	{
		Logging::Log() << "CD patch already set!";
		return;
	}

	// Address found
	constexpr BYTE CDBlockTest[] = { 0x33, 0x84, 0x24, 0x08, 0x04, 0x00, 0x00, 0x53 };
	if (!CDCheckAddr || !CheckMemoryAddress((void*)((DWORD)CDCheckAddr + 10), (void*)CDBlockTest, sizeof(CDCheckAddr)))
	{
		Logging::Log() << __FUNCTION__ << " Error: Could not find CD check function address in memory!";
		return;
	}
	CDCheckAddr = (void*)((DWORD)CDCheckAddr - 1);

	// Update SH2 code
	Logging::Log() << "Bypassing CD check...";
	UpdateMemoryAddress(CDCheckAddr, "\xC3", 1);
}
