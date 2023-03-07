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
#include "Common\FileSystemHooks.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

// Small Room (ps189)
constexpr float CatacombSmallRoomTexR = 2.5f; // "Small Room" Location Texture Red
constexpr float CatacombSmallRoomTexG = 2.0f; // "Small Room" Location Texture Green
constexpr float CatacombSmallRoomTexB = 2.0f; // "Small Room" Location Texture Blue

// Large Room (ps193)
constexpr float CatacombLargeRoomTexR = 3.15f; // "Large Room" Location Texture Red
constexpr float CatacombLargeRoomTexG = 2.8f; // "Large Room" Location Texture Green
constexpr float CatacombLargeRoomTexB = 2.8f; // "Large Room" Location Texture Blue

constexpr float CatacombLargeRoomFloorR = 0.0f; // "Large Room" Floor Red
constexpr float CatacombLargeRoomFloorG = -0.1f; // "Large Room" Floor Green
constexpr float CatacombLargeRoomFloorB = -0.1f; // "Large Room" Floor Blue

// Patch SH2 code to Fix Cemetery Lighting
void PatchCatacombsMeatRoom()
{
	// Get Cemetery Lighting address
	constexpr BYTE CatacombSearchBytes[]{ 0x00, 0xFE, 0x42, 0x00, 0x00, 0x31, 0x43, 0x00, 0x00, 0xFE, 0x42, 0x00, 0x00, 0xFE, 0x42, 0x9A, 0x99, 0x19, 0x3E, 0x9A, 0x99, 0x19, 0x3E, 0x9A, 0x99, 0x19 };
	DWORD CatacombSmallRoomTexAddr = SearchAndGetAddresses(0x007FBB91, 0x007FF779, 0x007FE779, CatacombSearchBytes, sizeof(CatacombSearchBytes), 0x3F, __FUNCTION__);

	// Checking address pointer
	if (!CatacombSmallRoomTexAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	DWORD CatacombLargeRoomTexAddr = CatacombSmallRoomTexAddr + 0x1F0;
	DWORD CatacombLargeRoomFloorAddr = CatacombSmallRoomTexAddr + 0x30;

	// Check for valid code before updating
	constexpr BYTE CatacombTexBytes[]{ 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 0x40 };
	constexpr BYTE CatacombFloorBytes[]{ 0xCD, 0xCC, 0xCC, 0x3D, 0xCD, 0xCC, 0xCC, 0x3D, 0xCD, 0xCC, 0xCC, 0x3D };
	if (!CheckMemoryAddress((void*)CatacombSmallRoomTexAddr, (void*)CatacombTexBytes, sizeof(CatacombTexBytes), __FUNCTION__) ||
		!CheckMemoryAddress((void*)CatacombLargeRoomTexAddr, (void*)CatacombTexBytes, sizeof(CatacombTexBytes), __FUNCTION__) ||
		!CheckMemoryAddress((void*)CatacombLargeRoomFloorAddr, (void*)CatacombFloorBytes, sizeof(CatacombFloorBytes), __FUNCTION__))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Updating the Catacomb Meat Cold Rooms color...";

	// Small Room (ps189)
	UpdateMemoryAddress((void*)CatacombSmallRoomTexAddr, (void*)&CatacombSmallRoomTexR, sizeof(DWORD));
	UpdateMemoryAddress((void*)(CatacombSmallRoomTexAddr + 4), (void*)&CatacombSmallRoomTexG, sizeof(DWORD));
	UpdateMemoryAddress((void*)(CatacombSmallRoomTexAddr + 8), (void*)&CatacombSmallRoomTexB, sizeof(DWORD));

	// Large Room (ps193)
	UpdateMemoryAddress((void*)CatacombLargeRoomTexAddr, (void*)&CatacombLargeRoomTexR, sizeof(DWORD));
	UpdateMemoryAddress((void*)(CatacombLargeRoomTexAddr + 4), (void*)&CatacombLargeRoomTexG, sizeof(DWORD));
	UpdateMemoryAddress((void*)(CatacombLargeRoomTexAddr + 8), (void*)&CatacombLargeRoomTexB, sizeof(DWORD));

	UpdateMemoryAddress((void*)CatacombLargeRoomFloorAddr, (void*)&CatacombLargeRoomFloorR, sizeof(DWORD));
	UpdateMemoryAddress((void*)(CatacombLargeRoomFloorAddr + 4), (void*)&CatacombLargeRoomFloorG, sizeof(DWORD));
	UpdateMemoryAddress((void*)(CatacombLargeRoomFloorAddr + 8), (void*)&CatacombLargeRoomFloorB, sizeof(DWORD));
}
