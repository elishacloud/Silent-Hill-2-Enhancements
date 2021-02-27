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

void PatchCreatureVehicleSpawn()
{
	// Get Lying Figures address
	constexpr BYTE SearchBytes[]{ 0x89, 0x4E, 0x10, 0xC6, 0x46, 0x06, 0x00, 0x88, 0x46, 0x03, 0xD9, 0x86, 0x90, 0x00, 0x00, 0x00, 0xD8, 0x1D };
	DWORD CreatureAddr = SearchAndGetAddresses(0x004C5C42, 0x004C5EF2, 0x004C57B2, SearchBytes, sizeof(SearchBytes), 0x00);
	if (!CreatureAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Fixing behavior of Lying Figures that are hiding under vehicles...";
	UpdateMemoryAddress((void*)CreatureAddr, "\x90\x90\x90", 3);
}
