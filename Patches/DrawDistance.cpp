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
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Forward declaration
DWORD CheckUpdateMemory(void *dataSrc, void *dataDest, size_t size, bool SearchMemory);

// Predefined code bytes
constexpr BYTE DDStartAddr[] = { 0xC7, 0x05, 0x58 };
constexpr BYTE DDSearchAddr[] = { 0x94, 0x00, 0x00, 0x60, 0xEA, 0x45 };

// New draw distance
constexpr float DrawDistance = 8500.0f;

// Update SH2 code for Draw Distance
void UpdateDrawDistance()
{
	// Loop variables
	DWORD LoopCounter = 0;
	const DWORD SizeOfBytes = 10;
	BYTE SrcByteData[SizeOfBytes] = { NULL };
	BYTE DestByteData[SizeOfBytes] = { NULL };
	DWORD StartAddr = 0x0047C000;
	DWORD EndAddr = 0x004FFFFF;
	bool ExitFlag = false;

	// Get data bytes from code
	while (!ExitFlag && StartAddr < EndAddr && LoopCounter < 10)
	{
		LoopCounter++;

		// Get next address
		void *NextAddr = CheckMultiMemoryAddress((void*)0x0047C19D, (void*)0x0047C43D, (void*)0x0047C64D, (void*)DDSearchAddr, sizeof(DDSearchAddr));

		// Search for address
		if (!NextAddr)
		{
			Logging::Log() << __FUNCTION__ << " searching for memory address!";
			NextAddr = GetAddressOfData(DDSearchAddr, sizeof(DDSearchAddr), 1, StartAddr, EndAddr - StartAddr);
		}

		// Checking address pointer
		if (!NextAddr)
		{
			Logging::Log() << __FUNCTION__ << " Error: could not find binary data!";
			return;
		}
		StartAddr = (DWORD)NextAddr + SizeOfBytes;
		NextAddr = (void*)((DWORD)NextAddr - 4);

		// Check if this is the correct address and store bytes
		if (CheckMemoryAddress(NextAddr, (void*)DDStartAddr, sizeof(DDStartAddr)))
		{
			ExitFlag = true;
			memcpy(SrcByteData, NextAddr, SizeOfBytes);
			memcpy(DestByteData, NextAddr, SizeOfBytes);
			memcpy(DestByteData + 6, (void*)&DrawDistance, sizeof(float));
		}
	}

	// Check if data bytes are found
	if (SrcByteData[0] != DDStartAddr[0] || SrcByteData[1] != DDStartAddr[1] || SrcByteData[2] != DDStartAddr[2])
	{
		Logging::Log() << __FUNCTION__ << " Error: binary data does not match!";
		return;
	}

	// Logging
	Logging::Log() << "Increasing the Draw Distance...";

	bool SearchMemoryFlag = false;
	void *NextAddr = nullptr;

	// Address 1
	NextAddr = CheckMultiMemoryAddress((void*)0x0047C199, (void*)0x0047C439, (void*)0x0047C649, (void*)SrcByteData, SizeOfBytes);
	SearchMemoryFlag = CheckUpdateMemory(NextAddr, DestByteData, SizeOfBytes, SearchMemoryFlag);

	// Address 2
	NextAddr = CheckMultiMemoryAddress((void*)0x0057E7C5, (void*)0x0057F075, (void*)0x0057E995, (void*)SrcByteData, SizeOfBytes);
	SearchMemoryFlag = CheckUpdateMemory(NextAddr, DestByteData, SizeOfBytes, SearchMemoryFlag);

	// Address 3
	NextAddr = CheckMultiMemoryAddress((void*)0x00587F77, (void*)0x00588827, (void*)0x00588147, (void*)SrcByteData, SizeOfBytes);
	SearchMemoryFlag = CheckUpdateMemory(NextAddr, DestByteData, SizeOfBytes, SearchMemoryFlag);

	// Address 4
	NextAddr = CheckMultiMemoryAddress((void*)0x00587FB7, (void*)0x00588867, (void*)0x00588187, (void*)SrcByteData, SizeOfBytes);
	SearchMemoryFlag = CheckUpdateMemory(NextAddr, DestByteData, SizeOfBytes, SearchMemoryFlag);

	// Address 5
	NextAddr = CheckMultiMemoryAddress((void*)0x00594FE6, (void*)0x00595896, (void*)0x005951B6, (void*)SrcByteData, SizeOfBytes);
	SearchMemoryFlag = CheckUpdateMemory(NextAddr, DestByteData, SizeOfBytes, SearchMemoryFlag);

	// Address 6
	NextAddr = CheckMultiMemoryAddress((void*)0x0059EC1B, (void*)0x0059F4CB, (void*)0x0059EDEB, (void*)SrcByteData, SizeOfBytes);
	SearchMemoryFlag = CheckUpdateMemory(NextAddr, DestByteData, SizeOfBytes, SearchMemoryFlag);

	// Update all SH2 code with new DrawDistance values
	if (SearchMemoryFlag)
	{
		Logging::Log() << __FUNCTION__ << " searching for memory address!";
		if (!ReplaceMemoryBytes(SrcByteData, DestByteData, SizeOfBytes, 0x0047C000, 0x005FFFFF - 0x0047C000))
		{
			Logging::Log() << __FUNCTION__ << " Error: replacing pointer!";
		}
	}
}

DWORD CheckUpdateMemory(void *dataSrc, void *dataDest, size_t size, bool SearchMemoryFlag)
{
	if (dataSrc)
	{
		UpdateMemoryAddress(dataSrc, dataDest, size);
	}
	else
	{
		SearchMemoryFlag = true;
	}
	return SearchMemoryFlag;
}
