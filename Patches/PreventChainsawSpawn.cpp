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

void PatchPreventChainsawSpawn()
{
	// Get chiansaw spawn address
	constexpr BYTE SearchBytes[]{ 0x85, 0xF6, 0x7E, 0x0F, 0xF6, 0xC1, 0x40, 0x74, 0x0A, 0x81, 0x0D };
	DWORD ChainsawAddr = SearchAndGetAddresses(0x0048AB80, 0x0048AE20, 0x0048B030, SearchBytes, sizeof(SearchBytes), -0x12);
	if (!ChainsawAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Add fix to prevent chainsaw from spawning on first playthrough...";
	UpdateMemoryAddress((void*)ChainsawAddr, "\x00", 1);
}
