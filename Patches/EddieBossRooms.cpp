/**
* Copyright (C) 2018 Elisha Riedlinger
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
#include <Shlwapi.h>
#include "Hooking\FileSystemHooks.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Common\Logging.h"

// Predefined code bytes
constexpr BYTE EddieSearchBytes[]{ 0x00, 0xFE, 0x42, 0x00, 0x00, 0x31, 0x43, 0x00, 0x00, 0xFE, 0x42, 0x00, 0x00, 0xFE, 0x42, 0x9A, 0x99, 0x19, 0x3E, 0x9A, 0x99, 0x19, 0x3E, 0x9A, 0x99, 0x19 };
constexpr BYTE EddieTexBytes[]{ 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 0x40 };
constexpr BYTE EddieFloorBytes[]{ 0xCD, 0xCC, 0xCC, 0x3D, 0xCD, 0xCC, 0xCC, 0x3D, 0xCD, 0xCC, 0xCC, 0x3D };

// Small Room (ps189)
constexpr float EddiesSmallRoomTexR = 2.5f; // "Small Room" Location Texture Red
constexpr float EddiesSmallRoomTexG = 2.0f; // "Small Room" Location Texture Green
constexpr float EddiesSmallRoomTexB = 2.0f; // "Small Room" Location Texture Blue

// Large Room (ps193)
constexpr float EddiesLargeRoomTexR = 3.15f; // "Large Room" Location Texture Red
constexpr float EddiesLargeRoomTexG = 2.8f; // "Large Room" Location Texture Green
constexpr float EddiesLargeRoomTexB = 2.8f; // "Large Room" Location Texture Blue

constexpr float EddiesLargeRoomFloorR = 0.0f; // "Large Room" Floor Red
constexpr float EddiesLargeRoomFloorG = -0.1f; // "Large Room" Floor Green
constexpr float EddiesLargeRoomFloorB = -0.1f; // "Large Room" Floor Blue

// Update SH2 code to Fix Cemetery Lighting
void UpdateEddieBossRooms()
{
	// Check that UseCustomModFolder is enabled
	if (!UseCustomModFolder)
	{
		Log() << __FUNCTION__ << " Could not load fix.  This fix requires 'UseCustomModFolder' to be enabled!";
		return;
	}

	// Get required map file paths
	wchar_t Map189[MAX_PATH];
	wcscpy_s(Map189, MAX_PATH, ModPathW);
	wcscat_s(Map189, MAX_PATH, L"\\bg\\ps\\ps189.map");
	wchar_t Map193[MAX_PATH];
	wcscpy_s(Map193, MAX_PATH, ModPathW);
	wcscat_s(Map193, MAX_PATH, L"\\bg\\ps\\ps193.map");

	// Check for required map files
	if (!PathFileExists(Map189) || !PathFileExists(Map193))
	{
		Log() << __FUNCTION__ << " Could not load fix, required map files 'ps189.map' and 'ps193.map' missing!";
		return;
	}

	// Get Cemetery Lighting address
	DWORD EddiesSmallRoomTexAddr = (DWORD)GetAddressOfData(EddieSearchBytes, sizeof(EddieSearchBytes), 1, 0x007FB950, 1800);
	if (!EddiesSmallRoomTexAddr)
	{
		Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	EddiesSmallRoomTexAddr += 0x3F;
	DWORD EddiesLargeRoomTexAddr = EddiesSmallRoomTexAddr + 0x1F0;
	DWORD EddiesLargeRoomFloorAddr = EddiesSmallRoomTexAddr + 0x30;

	// Check for valid code before updating
	if (!CheckMemoryAddress((void*)EddiesSmallRoomTexAddr, (void*)EddieTexBytes, sizeof(EddieTexBytes)) ||
		!CheckMemoryAddress((void*)EddiesLargeRoomTexAddr, (void*)EddieTexBytes, sizeof(EddieTexBytes)) ||
		!CheckMemoryAddress((void*)EddiesLargeRoomFloorAddr, (void*)EddieFloorBytes, sizeof(EddieFloorBytes)))
	{
		Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return;
	}

	// Update SH2 code
	Log() << "Updating the Eddie Boss Rooms color...";

	// Small Room (ps189)
	UpdateMemoryAddress((void*)EddiesSmallRoomTexAddr, (void*)&EddiesSmallRoomTexR, sizeof(DWORD));
	UpdateMemoryAddress((void*)(EddiesSmallRoomTexAddr + 4), (void*)&EddiesSmallRoomTexG, sizeof(DWORD));
	UpdateMemoryAddress((void*)(EddiesSmallRoomTexAddr + 8), (void*)&EddiesSmallRoomTexB, sizeof(DWORD));

	// Large Room (ps193)
	UpdateMemoryAddress((void*)EddiesLargeRoomTexAddr, (void*)&EddiesLargeRoomTexR, sizeof(DWORD));
	UpdateMemoryAddress((void*)(EddiesLargeRoomTexAddr + 4), (void*)&EddiesLargeRoomTexG, sizeof(DWORD));
	UpdateMemoryAddress((void*)(EddiesLargeRoomTexAddr + 8), (void*)&EddiesLargeRoomTexB, sizeof(DWORD));

	UpdateMemoryAddress((void*)EddiesLargeRoomFloorAddr, (void*)&EddiesLargeRoomFloorR, sizeof(DWORD));
	UpdateMemoryAddress((void*)(EddiesLargeRoomFloorAddr + 4), (void*)&EddiesLargeRoomFloorG, sizeof(DWORD));
	UpdateMemoryAddress((void*)(EddiesLargeRoomFloorAddr + 8), (void*)&EddiesLargeRoomFloorB, sizeof(DWORD));
}
