/**
* Copyright (C) 2020 Elisha Riedlinger
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
#include "patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Forward declaration
DWORD CheckUpdateMemory(void *dataSrc, void *dataDest, size_t size, bool SearchMemory);

// ASM functions to fix an issue with a certain water-filled hallway
__declspec(naked) void __stdcall DrawDistanceASM()
{
	__asm
	{
		mov ecx, dword ptr ds : [esp + 0x08]
		test ecx, ecx
		jna near Exit
		push edi
		mov edi, dword ptr ds : [esp + 0x08]
		mov eax, 0x00000001
		repe stosd
		pop edi
	Exit:
		ret
	}
}

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

	// Predefined code bytes
	constexpr BYTE DDStartAddr[] = { 0xC7, 0x05, 0x58 };
	constexpr BYTE DDSearchAddr[] = { 0x94, 0x00, 0x00, 0x60, 0xEA, 0x45 };

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
			float DrawDistance = 8500.0f;
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

	// Fix draw distance in water-filled hallway
	constexpr BYTE DrawFunctionSearchBytes[]{ 0x8D, 0x9E, 0xA0, 0x1A, 0x00, 0x00, 0x6A, 0x07, 0x53, 0x8B, 0xF8, 0xE8 };
	DWORD DrawFunctionAddr = SearchAndGetAddresses(0x004E6C42, 0x004E6EF2, 0x004E67B2, DrawFunctionSearchBytes, sizeof(DrawFunctionSearchBytes), 0x0B);
	if (!DrawFunctionAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	WriteCalltoMemory((BYTE*)DrawFunctionAddr, DrawDistanceASM);
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

void UpdateDynamicDrawDistance(DWORD *SH2_RoomID)
{
	// Get dynamic draw distance address
	static float *Address = nullptr;
	if (!Address)
	{
		RUNONCE();

		// Get address for dynamic draw distance
		constexpr BYTE SearchBytes[]{ 0xFF, 0xFF, 0x8D, 0x44, 0x24, 0x0C, 0x50, 0x68 };
		Address = (float*)ReadSearchedAddresses(0x0047D824, 0x0047DAC4, 0x0047DCD4, SearchBytes, sizeof(SearchBytes), 0x08);
		if (!Address)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
		Address += 0x02;
	}

	// Set dynamic draw distance
	static bool ValueSet = false;
	if (*SH2_RoomID == 0x03)
	{
		if (!ValueSet)
		{
			float Value = 44.0f;
			UpdateMemoryAddress(Address, &Value, sizeof(float));
			ValueSet = true;
		}
	}
	else if (*SH2_RoomID == 0x90)
	{
		if (!ValueSet)
		{
			float Value = 2.0f;
			UpdateMemoryAddress(Address, &Value, sizeof(float));
			ValueSet = true;
		}
	}
	else if (ValueSet)
	{
		float Value = 1.0f;
		UpdateMemoryAddress(Address, &Value, sizeof(float));
		ValueSet = false;
	}
}
