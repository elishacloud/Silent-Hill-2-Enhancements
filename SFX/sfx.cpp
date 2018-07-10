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
#include <fstream>
#include <string>
#include "sfx.h"
#include "..\Common\Utils.h"
#include "..\Common\Logging.h"

void UpdateSFXAddr()
{
	// Find address for SFX indexes
	void *sfxAddr = GetAddressOfData(sfxBlock, 24, 4);

	// Address found
	if (sfxAddr)
	{
		// Log message
		Log() << "Found SFX pointer address at: " << sfxAddr;

		// Get sddata.bin file path
		char myPath[MAX_PATH];
		GetModuleFileNameA(nullptr, myPath, MAX_PATH);
		char* p_pName = strrchr(myPath, '\\') + 1;
		strcpy_s(p_pName, MAX_PATH - strlen(myPath), "data\\sound\\sddata.bin");

		// Open sddata.bin file
		Log() << "Opening " << myPath;
		std::ifstream infile;
		infile.open(myPath, std::ios::binary | std::ios::in | std::ios::ate);
		if (!infile.is_open())
		{
			Log() << "Could not open sddata.bin file!";

			// Exiting
			return;
		}
		DWORD size = (DWORD)infile.tellg();

		// Define vars
		UINT IndexCount = 0;
		DWORD NewSFXAddr[417] = { 0 };
		const DWORD BlockSize = 8192;
		std::string chunk;
		chunk.resize(BlockSize + 5);

		// Loop through sddata.bin
		DWORD x = 0;
		while (x < size - 5 && IndexCount != 417)
		{
			// Read a chunk of bytes (extra 5 bytes in case the "RIFF" string spans multiple chunks)
			infile.seekg(x);
			if (size > x + BlockSize + 5)
			{
				infile.read(&chunk[0], BlockSize + 5);
			}
			else
			{
				chunk.resize(size - x);
				infile.read(&chunk[0], size - x);
			}

			// Search for "RIFF" the magic number for a WAV file
			size_t Position = chunk.find("RIFF");
			if (Position != std::string::npos)
			{
				// If found add to array
				NewSFXAddr[IndexCount] = x + Position;
				IndexCount++;
				x += Position + 5;
			}
			else
			{
				x += BlockSize;
			}
		}

		// Close file
		infile.close();

		// Log results
		if (IndexCount == 417)
		{
			Log() << "Found all WAV file indexes in sddata.bin";
		}
		else
		{
			Log() << "Could not find all the indexes in sddata.bin!  Found " << IndexCount;
		}

		// Update SFX address array
		DWORD oldProtect;
		if (VirtualProtect(sfxAddr, 700 * sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			Log() << "Updating SFX memory addresses...";

			// Write to memory
			for (x = 0; x < 700; x++)
			{
				*((DWORD *)((DWORD)sfxAddr + x * sizeof(DWORD))) = NewSFXAddr[SFXAddrMapping[x]];
			}

			// Restore protection
			VirtualProtect(sfxAddr, 700 * sizeof(DWORD), oldProtect, &oldProtect);
		}
		else
		{
			Log() << "Could not write to memory!";
		}

		// Find address for sddata.bin file pointer
		const DWORD start = 0x00401000;
		const DWORD distance = 0x00127FFF;
		sfxAddr = GetAddressOfData(sfxPtrvDC, 5, 1, start, distance);																		// Directors Cut
		sfxAddr = ((DWORD)sfxAddr < start || (DWORD)sfxAddr > start + distance) ? GetAddressOfData(sfxPtrv10, 5, 1, start, distance) : sfxAddr;	// v1.0
		sfxAddr = ((DWORD)sfxAddr < start || (DWORD)sfxAddr > start + distance) ? GetAddressOfData(sfxPtrv11, 5, 1, start, distance) : sfxAddr;	// v1.1
		sfxAddr = ((DWORD)sfxAddr < start || (DWORD)sfxAddr > start + distance) ? nullptr : sfxAddr;

		if (sfxAddr)
		{
			// Log message
			Log() << "Found sddata.bin pointer at address: " << sfxAddr;

			// Alocate memory
			char *PtrBytes;
			PtrBytes = new char[size + 1];

			// Update sddata.bin pointer address
			if (VirtualProtect(sfxAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect))
			{
				Log() << "Updating sddata.bin pointer memory addresses...";

				// Write to memory
				*((DWORD *)((DWORD)sfxAddr + 1)) = (DWORD)PtrBytes;

				// Restore protection
				VirtualProtect(sfxAddr, 5, oldProtect, &oldProtect);
			}
			else
			{
				Log() << "Could not write to memory!";
			}
		}
		else
		{
			Log() << "Could not find sddata.bin pointer address in memory!";
		}
	}
	else
	{
		Log() << "Could not find SFX pointer address in memory!";
	}
}
