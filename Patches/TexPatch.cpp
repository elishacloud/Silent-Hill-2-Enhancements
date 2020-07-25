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
#include <Shlwapi.h>
#include <filesystem>
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\FileSystemHooks.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

DWORD GetTexBufferSize()
{
	DWORD size = 0;

	for (const auto& path : {
		"data\\pic",
		"data\\pic\\add",
		"data\\pic\\apt",
		"data\\pic\\dls",
		"data\\pic\\effect",
		"data\\pic\\etc",
		"data\\pic\\hsp",
		"data\\pic\\htl",
		"data\\pic\\item",
		"data\\pic\\map",
		"data\\pic\\out",
		"data\\pic\\ufo",
		"data\\menu\\mc"
		})
	{
		if (PathFileExistsA(path))
		{
			for (const auto & entry : std::filesystem::directory_iterator(path))
			{
				if (entry.is_regular_file())
				{
					// Get size of file
					WIN32_FILE_ATTRIBUTE_DATA FileInformation = {};

					wchar_t Filename[MAX_PATH];
					if (GetFileAttributesEx(UpdateModPath(entry.path().c_str(), Filename), GetFileExInfoStandard, &FileInformation) && size < FileInformation.nFileSizeLow)
					{
						size = FileInformation.nFileSizeLow;
					}
				}
			}
		}
	}
	return size;
}

void PatchTexAddr()
{
	// Get addresses
	const DWORD StaticAddr = 0x00401CC1;		// Address is the same on all binaries
	BYTE SearchBytes1[]{ 0x68, 0x00, 0x00, 0x00, 0x00 };
	memcpy((SearchBytes1 + 1), (void*)StaticAddr, sizeof(DWORD));
	const DWORD Addr1 = SearchAndGetAddresses(0x0044B99D, 0x0044BB3D, 0x0044BB3D, SearchBytes1, sizeof(SearchBytes1), 0x01);
	constexpr BYTE SearchBytes2[]{ 0x05, 0x00, 0x00, 0x08, 0x00 };
	const DWORD Addr2 = SearchAndGetAddresses(0x00496F87, 0x00497231, 0x00497331, SearchBytes2, sizeof(SearchBytes2), 0x00);
	constexpr BYTE SearchBytes3[]{ 0x05, 0x00, 0x48, 0x10, 0x00 };
	const DWORD Addr3 = SearchAndGetAddresses(0x0049B40A, 0x0049B6BA, 0x0049AF7A, SearchBytes3, sizeof(SearchBytes3), 0x00);

	// Checking address pointer
	if (!Addr1 || !Addr2 || !Addr3)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get size of textures
	DWORD size = GetTexBufferSize();
	if (!size)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find texture buffer size!";
		return;
	}
	size += 2 * 1024 * 1024;	// Add 2 MB to buffer
	Logging::Log() << "Setting texture buffer size: " << size;

	// Allocate dynamic memory for loading textures
	BYTE *PtrBytes1 = new BYTE[size];
	ZeroMemory(PtrBytes1, size);		// Need to clear menory for debug mode, game expects the memory to be zeroed
	BYTE *PtrBytes2 = new BYTE[size];
	ZeroMemory(PtrBytes2, size);		// Need to clear menory for debug mode, game expects the memory to be zeroed
	if (!PtrBytes1 || !PtrBytes2)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to create texture buffer!";
		return;
	}

	// Logging update
	Logging::Log() << "Updating Texture memory address locations...";

	// Write new memory static address
	UpdateMemoryAddress((void*)StaticAddr, &PtrBytes1, sizeof(void*));

	// Write new memory address 1
	UpdateMemoryAddress((void*)Addr1, &PtrBytes1, sizeof(void*));

	// Write new memory address 2
	UpdateMemoryAddress((void*)Addr2, "\xB8", sizeof(BYTE));
	UpdateMemoryAddress((void*)(Addr2 + 1), &PtrBytes2, sizeof(void*));

	// Write new memory address 3
	UpdateMemoryAddress((void*)Addr3, "\xB8", sizeof(BYTE));
	UpdateMemoryAddress((void*)(Addr3 + 1), &PtrBytes2, sizeof(void*));
}
