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

void PatchBinary()
{
	// Find address for call code
	constexpr BYTE DCCallSearchBytes[] = { 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x68, 0xE8, 0x03, 0x00, 0x00, 0xE8 };
	void *DCCallPatchAddr = (void*)SearchAndGetAddresses(0x00408A29, 0x00408BD9, 0x00408BE9, DCCallSearchBytes, sizeof(DCCallSearchBytes), 0x32);

	// Address found
	if (!DCCallPatchAddr || !(CheckMemoryAddress(DCCallPatchAddr, "\xE8", 0x01) || CheckMemoryAddress(DCCallPatchAddr, "\xB8", 0x01)))
	{
		return;
	}

	// Update SH2 code
	Logging::Log() << "Patching binary...";
	UpdateMemoryAddress(DCCallPatchAddr, "\xB8", 1);
}
