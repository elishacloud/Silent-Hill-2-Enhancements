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
BYTE* ArrowHitboxYAxis = nullptr;

extern long BufferWidth, BufferHeight;

const float BaseBufferWidth = 1920.f;
const float BaseBufferHeight = 1080.f;

const float BaseXOffset = -235.f;
const float BaseYOffset = 170.f;

float CurrentXOffset = NULL;
float CurrentYOffset = NULL;

long LastBufferWidth = 0;
long LastBufferHeight = 0;


void PatchHospitalMapArrowHitbox()
{
    Logging::Log() << "Patching Hospital Map Arrow Hitbox";

    if (GameVersion == SH2V_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Couldn't determine game version.";
		return;
	}

	ArrowHitboxXAxis = GameVersion == SH2V_10 ?
		(BYTE*)0x006AF03C :
		GameVersion == SH2V_11 ?
		(BYTE*)0x006B001C :
		(BYTE*)0x006AF01C;

	ArrowHitboxYAxis = GameVersion == SH2V_10 ?
		(BYTE*)0x006AF038 :
		GameVersion == SH2V_11 ?
		(BYTE*)0x006B0018 :
		(BYTE*)0x006AF018;

    if (!ArrowHitboxXAxis || !ArrowHitboxYAxis)
    {
        Logging::Log() << __FUNCTION__ << " Error: Couldn't find arrow hitbox addresses.";
        return;
    }
}

void RunHospitalMapArrowHitboxFix()
{

    if (!ArrowHitboxXAxis || !ArrowHitboxYAxis)
    {
        RUNONCE();
        Logging::Log() << __FUNCTION__ << " Error: Couldn't find the arrow hitbox addresses.";
        return;
    }

    if (LastBufferHeight != BufferHeight || LastBufferWidth != BufferWidth)
    {
        if (BufferWidth <= 0 || BufferHeight <= 0)
        {
            return;
        }

        CurrentXOffset = BaseXOffset * ((float)BufferWidth / BaseBufferWidth);
        CurrentYOffset = BaseYOffset * ((float)BufferHeight / BaseBufferHeight);

        UpdateMemoryAddress(ArrowHitboxXAxis, &CurrentXOffset, sizeof(float));
        UpdateMemoryAddress(ArrowHitboxYAxis, &CurrentYOffset, sizeof(float));

        LastBufferWidth = BufferWidth;
        LastBufferHeight = BufferHeight;
    }
}