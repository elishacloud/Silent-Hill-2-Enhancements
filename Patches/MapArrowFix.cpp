/**
* Copyright (C) 2026 mercury501
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

BYTE* ArrowHitboxXAxis = nullptr;
const float BaseXOffset = -240.f;

// Bottom arrow map hospital 2F

void PatchHospitalMapArrowHitbox()
{
    Logging::Log() << "Patching Hospital Map Arrow Hitbox";

    if (GameVersion == SH2V_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Couldn't determine game version.";
		return;
	}

	ArrowHitboxXAxis = GameVersion == SH2V_10 ?
		(BYTE*)0x0049D55D :
		GameVersion == SH2V_11 ?
		(BYTE*)0x0049D80D :
        GameVersion == SH2V_DC ?
		(BYTE*)0x0049D0CD :
        nullptr;

    if (!ArrowHitboxXAxis)
    {
        Logging::Log() << __FUNCTION__ << " Error: Couldn't find arrow hitbox addresses.";
        return;
    }

    ArrowHitboxXAxis += 2;

    const float* baseXOffsetPtr = &BaseXOffset;

    UpdateMemoryAddress(ArrowHitboxXAxis, &baseXOffsetPtr, sizeof(float));
}