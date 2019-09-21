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
#include "Common\Utils.h"
#include "Logging\Logging.h"

void DisableCDCheck()
{
	// Find address for CD check
	constexpr BYTE CDFuncBlock[] = { 0x81, 0xEC, 0x08, 0x04, 0x00, 0x00, 0xA1 };
	void *CDCheckAddr = (void*)SearchAndGetAddresses(0x00408760, 0x004088C0, 0x004088D0, CDFuncBlock, sizeof(CDFuncBlock), 0x00);

	// Address found
	constexpr BYTE CDBlockTest[] = { 0x33, 0x84, 0x24, 0x08, 0x04, 0x00, 0x00, 0x53 };
	if ((CDCheckAddr) ? !CheckMemoryAddress((void*)((DWORD)CDCheckAddr + 11), (void*)CDBlockTest, sizeof(CDBlockTest)) : true)
	{
		Logging::Log() << __FUNCTION__ << " Error: Could not find CD check function address in memory!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Bypassing CD check...";
	UpdateMemoryAddress(CDCheckAddr, "\xC3", 1);
}
