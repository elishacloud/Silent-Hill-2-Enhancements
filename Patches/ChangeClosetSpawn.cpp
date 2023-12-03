/**
* Copyright (C) 2023 Elisha Riedlinger
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

void PatchClosetSpawn()
{
	// Get James load position Apt307 address
	constexpr BYTE SearchBytes[]{ 0x6A, 0x01, 0x6A, 0x07, 0x68, 0x01, 0x02, 0x00, 0x00, 0xE8 };
	float *Apt307JamesLoadPosZ = (float*)SearchAndGetAddresses(0x00599226, 0x00599AD6, 0x005993F6, SearchBytes, sizeof(SearchBytes), 0x24, __FUNCTION__);
	float *Apt307JamesLoadRotX = (float*)((DWORD)Apt307JamesLoadPosZ + 0x11);

	// Checking address pointer
	if (!Apt307JamesLoadPosZ)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Updating James' spawn point position...";
	float Value = -101525.0f;
	UpdateMemoryAddress((BYTE*)Apt307JamesLoadPosZ, &Value, sizeof(float));
	Value = 0.3f;
	UpdateMemoryAddress((BYTE*)Apt307JamesLoadRotX, &Value, sizeof(float));
}

void RunClosetSpawn()
{
	// Load Camera FOV address
	GetCameraFOV();

	// Checking address pointer
	if (!CameraFOVAddr)
	{
		RUNONCE();

		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	static DWORD LastCutscene = 0;
	DWORD CutsceneID = GetCutsceneID();
	if (LastCutscene == CS_APT_RPT_CLOSET && CutsceneID != CS_APT_RPT_CLOSET)
	{
		*CameraFOVAddr = 448.0f;
	}
	LastCutscene = CutsceneID;
}
