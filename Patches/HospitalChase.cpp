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

// Update SH2 code to Fix RPT Hospital Elevator Stabbing Animation
void UpdateHospitalChase(DWORD *SH2_RoomID)
{
	static bool ValueSet = false;

	float *JamesPosition = (float*)0x01FB101C;
	if (*SH2_RoomID == 0x5B && *JamesPosition > 33185.0f)
	{
		if (!ValueSet)
		{
			DWORD Address = 0x01FB1E00;
			DWORD Value = (DWORD)-1;
			UpdateMemoryAddress((void*)(Address + 0x00), &Value, sizeof(DWORD));
			UpdateMemoryAddress((void*)(Address + 0x0C), &Value, sizeof(DWORD));
		}
		ValueSet = true;
	}
	else
	{
		ValueSet = false;
	}
}
