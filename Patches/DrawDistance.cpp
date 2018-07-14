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
#include "..\Common\Utils.h"
#include "..\Common\Logging.h"

// Predefined code bytes
constexpr BYTE DDStartAddr[] = { 0xC7, 0x05, 0x58 };
constexpr BYTE DDSearchAddr[] = { 0x94, 0x00, 0x00, 0x60, 0xEA, 0x45 };

// New draw distance
constexpr float DrawDistance = 8500.0f;

// Update SH2 code for Draw Distance
void UpdateDrawDistance()
{
	// Loop variables
	const DWORD SizeOfBytes = 10;
	BYTE ByteData[SizeOfBytes] = { NULL };
	DWORD StartAddr = 0x0047C000;
	DWORD EndAddr = 0x004FFFFF;
	bool ExitFlag = false;

	// Get data bytes from code
	while (!ExitFlag && StartAddr < EndAddr)
	{
		// Get next address
		void *NextAddr = GetAddressOfData(DDSearchAddr, sizeof(DDSearchAddr), 1, StartAddr, EndAddr - StartAddr);
		if (!NextAddr)
		{
			Log() << __FUNCTION__ << " Error: could not find binary data!";
			return;
		}
		StartAddr = (DWORD)NextAddr + SizeOfBytes;
		NextAddr = (void*)((DWORD)NextAddr - 4);

		// Check if this is the correct address and store bytes
		if (CheckMemoryAddress(NextAddr, (void*)DDStartAddr, sizeof(DDStartAddr)))
		{
			ExitFlag = true;
			memcpy(ByteData, NextAddr, SizeOfBytes);
		}
	}

	// Check if data bytes are found
	if (ByteData[0] != DDStartAddr[0] || ByteData[1] != DDStartAddr[1] || ByteData[2] != DDStartAddr[2])
	{
		Log() << __FUNCTION__ << " Error: binary data does not match!";
		return;
	}

	// Logging
	Log() << "Increasing the Draw Distance...";

	// Reset variables for next loop
	StartAddr = 0x0047C000;
	EndAddr = 0x005FFFFF;

	// Update Draw Distance
	while (StartAddr < EndAddr)
	{
		// Get next address
		void *NextAddr = GetAddressOfData(ByteData, SizeOfBytes, 1, StartAddr, EndAddr - StartAddr);
		if (!NextAddr)
		{
			return;
		}
		StartAddr = (DWORD)NextAddr + SizeOfBytes;

		// Write new Draw Distance
		UpdateMemoryAddress((void*)((DWORD)NextAddr + 6), (void*)&DrawDistance, sizeof(float));
	}
}
