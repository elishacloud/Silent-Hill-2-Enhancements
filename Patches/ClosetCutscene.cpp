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

// Update SH2 code to Fix RPT Apartment Closet Cutscene
void UpdateClosetCutscene(DWORD *SH2_CutsceneID, float *SH2_CutsceneCameraPos)
{
	// Darken & blur more closet bars
	static bool ValueSet1 = false;
	float CutsceneCameraPos = -21376.56445f;
	if (*SH2_CutsceneID == 0x0E && *SH2_CutsceneCameraPos == CutsceneCameraPos)
	{
		if (!ValueSet1)
		{
			DWORD Address = 0x009428E2;
			BYTE ByteValue = 15;
			UpdateMemoryAddress((void*)Address, &ByteValue, sizeof(BYTE));		// "Time of Day"
			Address = 0x005999CA;
			ByteValue = 7;
			UpdateMemoryAddress((void*)Address, &ByteValue, sizeof(BYTE));		// Closet bar blur intensity
		}
		ValueSet1 = true;
	}
	else if (ValueSet1)
	{
		DWORD Address = 0x009428E2;
		BYTE ByteValue = 11;
		UpdateMemoryAddress((void*)Address, &ByteValue, sizeof(BYTE));		// "Time of Day"
		Address = 0x005999CA;
		ByteValue = 3;
		UpdateMemoryAddress((void*)Address, &ByteValue, sizeof(BYTE));		// Closet bar blur intensity
		ValueSet1 = false;
	}

	// Adjust room/dynamic objects for one shot
	static bool ValueSet2 = false;
	CutsceneCameraPos = -20133.99805f;
	DWORD Address = 0x00942A50;
	if (*SH2_CutsceneID == 0x0E  && *SH2_CutsceneCameraPos == CutsceneCameraPos)
	{
		if (!ValueSet2)
		{
			float Value = -0.2f;
			UpdateMemoryAddress((void*)(Address + 0x00), &Value, sizeof(float));		// PC location textures + models color R (set 1)
			UpdateMemoryAddress((void*)(Address + 0x04), &Value, sizeof(float));		// PC location textures + models color G (set 1)
			UpdateMemoryAddress((void*)(Address + 0x08), &Value, sizeof(float));		// PC location textures + models color B (set 1)
			Value = 1.0f;
			UpdateMemoryAddress((void*)(Address + 0x20), &Value, sizeof(float));		// PC location textures + models color R (set 2)
			UpdateMemoryAddress((void*)(Address + 0x24), &Value, sizeof(float));		// PC location textures + models color G (set 2)
			UpdateMemoryAddress((void*)(Address + 0x28), &Value, sizeof(float));		// PC location textures + models color B (set 2)
		}
		ValueSet2 = true;
	}
	else if (ValueSet2)
	{
		float Value = 0.150000006f;
		UpdateMemoryAddress((void*)(Address + 0x00), &Value, sizeof(float));		// PC location textures + models color R (set 1)
		UpdateMemoryAddress((void*)(Address + 0x04), &Value, sizeof(float));		// PC location textures + models color G (set 1)
		UpdateMemoryAddress((void*)(Address + 0x08), &Value, sizeof(float));		// PC location textures + models color B (set 1)
		Value = 0.04500000179f;
		UpdateMemoryAddress((void*)(Address + 0x20), &Value, sizeof(float));		// PC location textures + models color R (set 2)
		UpdateMemoryAddress((void*)(Address + 0x24), &Value, sizeof(float));		// PC location textures + models color G (set 2)
		UpdateMemoryAddress((void*)(Address + 0x28), &Value, sizeof(float));		// PC location textures + models color B (set 2)
		ValueSet2 = false;
	}
}
