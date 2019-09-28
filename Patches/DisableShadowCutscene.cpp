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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

void UpdateShadowCutscene(DWORD *SH2_CutsceneID)
{
	// Get shadow address
	static BYTE *Address = nullptr;
	if (!Address)
	{
		RUNONCE();

		// Get address for shadows
		constexpr BYTE SearchBytes[]{ 0x00, 0x33, 0xC9, 0x3B, 0xD3, 0x0F, 0x94, 0xC1, 0xA3 };
		Address = (BYTE*)ReadSearchedAddresses(0x00462DD5, 0x00463045, 0x00463045, SearchBytes, sizeof(SearchBytes), -0xA0);
		if (!Address)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	// Set shadow
	static bool ValueSet = false;
	if (*SH2_CutsceneID == 0x03 || *SH2_CutsceneID == 0x2E)
	{
		if (!ValueSet)
		{
			BYTE Value = 0x00;
			UpdateMemoryAddress(Address, &Value, sizeof(BYTE));
			ValueSet = true;
		}
	}
	else if (ValueSet)
	{
		BYTE Value = 0x01;
		UpdateMemoryAddress(Address, &Value, sizeof(BYTE));
		ValueSet = false;
	}
}
