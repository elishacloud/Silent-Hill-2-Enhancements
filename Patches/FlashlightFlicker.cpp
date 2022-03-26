/**
* Copyright (C) 2022 Elisha Riedlinger
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

void PatchFlashlightFlicker()
{
	// Get address
	constexpr BYTE SearchBytes[]{ 0x00, 0x00, 0x00, 0x00, 0x74, 0x0F, 0xE8 };
	void *Address = (void*)SearchAndGetAddresses(0x0040170D, 0x0040170D, 0x0040170D, SearchBytes, sizeof(SearchBytes), 0x15);

	// Checking address pointer
	if (!Address)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Fixing Flashlight Flicker...";
	UpdateMemoryAddress(Address, "\x90\x90\x90\x90\x90", 5);
}
