/**
* Copyright (C) 2023 mercury501, Polymega
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

#include "Logging\Logging.h"
#include "Patches.h"
#include "Common\Utils.h"

float* FinalBossDrawDistanceAddr = nullptr;

const float FinalBossFixValue = -30000.f;
const float FinalBossOriginalWalkway = -8000.f;
const float FinalBossOriginalFloor = -4000.f;
const float FinalBossOriginalBoxSpawn = -12000.f;

void HandleFinalBossRoomFix()
{
	if (GetJamesPosY() < -14600.f && GetRoomID() == 0xBB)
	{
		UpdateMemoryAddress(GetFinalBossBottomWalkwaySpawnPointer(), &FinalBossFixValue, sizeof(float));
		UpdateMemoryAddress(GetFinalBossBottomFloorSpawnPointer(), &FinalBossFixValue, sizeof(float));
		UpdateMemoryAddress(GetFinalBossBlackBoxSpawnPointer(), &FinalBossFixValue, sizeof(float));		
	}
	else
	{
		UpdateMemoryAddress(GetFinalBossBottomWalkwaySpawnPointer(), &FinalBossOriginalWalkway, sizeof(float));
		UpdateMemoryAddress(GetFinalBossBottomFloorSpawnPointer(), &FinalBossOriginalFloor, sizeof(float));
		UpdateMemoryAddress(GetFinalBossBlackBoxSpawnPointer(), &FinalBossOriginalBoxSpawn, sizeof(float));
	}
}

float* GetFinalBossDrawDistancePointer()
{
	if (FinalBossDrawDistanceAddr)
	{
		return FinalBossDrawDistanceAddr;
	}

	// Address is retrieved like so because it's only reference is a float** not byte aligned
	float* DrawDistancePtr = GameVersion == SH2V_10 ? (float*) 0x00800614 :
		GameVersion == SH2V_11 ? (float*)0x008041FC :
		GameVersion == SH2V_DC ? (float*)0x008031FC : nullptr;

	// Checking address pointer
	if (!DrawDistancePtr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find FinalBossDrawDistance  address!";
		return nullptr;
	}

	FinalBossDrawDistanceAddr = DrawDistancePtr;

	return FinalBossDrawDistanceAddr;
}

void PatchFinalBossRoom()
{
	Logging::Log() << "Patching final boss black box...";

	*GetFinalBossDrawDistancePointer() = 30000.f;
}