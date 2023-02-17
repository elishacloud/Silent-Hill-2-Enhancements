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

// Patch the save/load BG images for each campaign
void PatchSaveBGImage()
{
	// Get fog addresses
	constexpr BYTE BGLoadSearchBytes[]{ 0x83, 0xC4, 0x0C, 0x3C, 0x02, 0x7D, 0x1D, 0x80, 0x3D };
	BYTE *BGLoadAddr = (BYTE*)SearchAndGetAddresses(0x0044B414, 0x0044B5B4, 0x0044B5B4, BGLoadSearchBytes, sizeof(BGLoadSearchBytes), 0x5);

	if (!BGLoadAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Enabling Save Background Images Fix...";
	UpdateMemoryAddress(BGLoadAddr, "\x90\x90", 2);
}

void RunSaveBGImage()
{
	static BYTE *ChapterIDPointer = GetChapterIDPointer();
	if (!ChapterIDPointer)
	{
		LOG_ONCE(__FUNCTION__ " Error: failed to find memory address!");
		return;
	}

	static BYTE LastChapterID = 0;
	BYTE ChapterID = GetMenuEvent();
	
	if (LastChapterID != 0x07 && ChapterID == 0x07)
	{
		*ChapterIDPointer = 0x00;
	}

	LastChapterID = ChapterID;
}
