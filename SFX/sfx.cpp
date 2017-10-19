/**
* Copyright (C) 2017 Elisha Riedlinger
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

#include <fstream>
#include <string>
#include "sfx.h"
#include "..\Common\Logging.h"

// Search memory for byte array
void *GetAddressOfData(const void *data, size_t len)
{
	HANDLE hProcess = GetCurrentProcess();
	if (hProcess)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);

		MEMORY_BASIC_INFORMATION info;
		std::string chunk;
		BYTE* p = 0;
		while (p < si.lpMaximumApplicationAddress)
		{
			if (VirtualQueryEx(hProcess, p, &info, sizeof(info)) == sizeof(info))
			{
				p = (BYTE*)info.BaseAddress;
				chunk.resize(info.RegionSize);
				SIZE_T bytesRead;
				if (ReadProcessMemory(hProcess, p, &chunk[0], info.RegionSize, &bytesRead))
				{
					for (size_t i = 0; i < (bytesRead - len); i += 4)
					{
						if (memcmp(data, &chunk[i], len) == 0)
						{
							return (BYTE*)p + i;
						}
					}
				}
				p += info.RegionSize;
			}
			else
			{
				return nullptr;
			}
		}
	}
	return nullptr;
}

void UpdateSFXAddr()
{
	// Find address for SFX indexes
	void *sfxAddr = GetAddressOfData(sfxBlock, 24);

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
			infile.read(&chunk[0], BlockSize + 5);

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

		// Make memory writeable
		DWORD oldProtect;
		if (VirtualProtect(sfxAddr, 700 * sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			Log() << "Updating SFX memory addresses";

			// Write to memory
			for (int x = 1; x < 700; x++)
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
	}
	else
	{
		Log() << "Could not find pointer address in memory!";
	}
}
