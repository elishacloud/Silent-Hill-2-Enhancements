/**
* Copyright (C) 2024 Elisha Riedlinger
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
#include <shlwapi.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\FileSystemHooks.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "Common\Unicode.h"

// Default values
float LowHealthXPos = 0.85f;
float LowHealthYPos = 0.95f;
float LowHealthHeight = 0.1f;

void PatchLowHealthIndicator()
{
	// Check if LowHealthFade.png exists 
	std::vector<const char*> Paths = { GetLangPath(""), GetModPath(""), "data" };
	if (!std::any_of(Paths.begin(), Paths.end(), [](auto& entry) { return PathFileExists((std::string(entry) + "\\pic\\etc\\LowHealthFade.png").c_str()); }))
	{
		Logging::Log() << "Warning: 'LowHealthFade.png' not found!";
		LowHealthIndicatorStyle = 1;
		return;
	}

	// Set variables for new fullscreen style
	LowHealthXPos = 0.0f;
	LowHealthYPos = 1.001f;
	LowHealthHeight = 0.25f;

	DWORD BaseMemoryAddress =
		GameVersion == SH2V_10 ? 0x004F684C :
		GameVersion == SH2V_11 ? 0x004F6AFC :
		GameVersion == SH2V_DC ? 0x004F63BC : NULL;

	// Checking address pointer
	if (!BaseMemoryAddress || *(BYTE*)BaseMemoryAddress != 0xD8 || *(BYTE*)((DWORD)BaseMemoryAddress + 1) != 0x0D)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Updating Low Health Indicator...";
	float* Value = &LowHealthXPos;
	UpdateMemoryAddress((BYTE*)(BaseMemoryAddress + 2), &Value, sizeof(float));
	Value = &LowHealthYPos;
	UpdateMemoryAddress((BYTE*)(BaseMemoryAddress + 12), &Value, sizeof(float));
	UpdateMemoryAddress((BYTE*)(BaseMemoryAddress + 24), &Value, sizeof(float));
	Value = &LowHealthHeight;
	UpdateMemoryAddress((BYTE*)(BaseMemoryAddress + 44), &Value, sizeof(float));
}
