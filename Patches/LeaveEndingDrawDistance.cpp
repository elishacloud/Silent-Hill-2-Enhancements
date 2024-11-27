/**
* Copyright (C) 2024 mercury501
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

#include "Common\Utils.h"
#include "Patches\Patches.h"
#include "Logging\Logging.h"

void PatchLeaveEndingCemeteryDrawDistance()
{
	DWORD CemeteryDrawDistance = GameVersion == SH2V_10 ? 0x008E7878 :
		GameVersion == SH2V_11 ? 0x008EB548 :
		GameVersion == SH2V_DC ? 0x008EA548 :
		NULL;

	if (CemeteryDrawDistance == NULL)
	{
		Logging::Log() << __FUNCTION__ << " Couldn't parse game version.";
		return;
	}

	float DrawDistanceValue = 40000.0f;

	UpdateMemoryAddress((BYTE*)CemeteryDrawDistance, &DrawDistanceValue, sizeof(float));

	Logging::Log() << __FUNCTION__ << " Patched L. ending draw distance!";
}