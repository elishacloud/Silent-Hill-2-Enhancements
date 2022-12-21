/**
* Copyright (C) 2022 Elisha Riedlinger
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

BYTE *PtrBytes1 = nullptr;
BYTE *PtrBytes2 = nullptr;
BYTE *PtrBytes3 = nullptr;
DWORD BufferSize = 0;
DWORD BufferSize3 = 0;

void *LoadAddress = nullptr;
void *callBufferAddr = nullptr;
void *jmpBufferAddr = nullptr;

void ClearMemoryBuffer()
{
	if (LoadAddress == PtrBytes1)
	{
		ZeroMemory(PtrBytes1, BufferSize);
	}
	else if (LoadAddress == PtrBytes2)
	{
		ZeroMemory(PtrBytes2, BufferSize);
	}
	else if (LoadAddress == PtrBytes3)
	{
		ZeroMemory(PtrBytes3, BufferSize3);
	}
}

// ASM function to clear texture buffer
__declspec(naked) void __stdcall TexBufferASM()
{
	__asm
	{
		call callBufferAddr
		push eax
		push ecx
		push edx
		mov dword ptr ds : [LoadAddress], ebp
		call ClearMemoryBuffer
		pop edx
		pop ecx
		pop eax
		jmp jmpBufferAddr
	}
}

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
					if (GetFileAttributesEx(GetFileModPath(entry.path().c_str(), Filename), GetFileExInfoStandard, &FileInformation) && size < FileInformation.nFileSizeLow)
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
	// Get call address
	constexpr BYTE BufferSearchBytes[]{ 0x83, 0xC4, 0x10, 0x85, 0xC0, 0x74, 0x1C, 0x33, 0xC0, 0x5D, 0x8B, 0x8C, 0x24 };
	const DWORD ClearBuffAddr = SearchAndGetAddresses(0x00449FDC, 0x0044A17C, 0x0044A17C, BufferSearchBytes, sizeof(BufferSearchBytes), 0x31);

	// Checking address pointer
	if (!ClearBuffAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	callBufferAddr = (void*)(*(DWORD*)(ClearBuffAddr + 1) + ClearBuffAddr + 5);
	jmpBufferAddr = (void*)(ClearBuffAddr + 5);

	// Get addresses
	const DWORD StaticAddr = 0x00401CC1;		// Address is the same on all binaries
	BYTE SearchBytes1[]{ 0x68, 0x00, 0x00, 0x00, 0x00 };
	memcpy((SearchBytes1 + 1), (void*)StaticAddr, sizeof(DWORD));
	const DWORD Addr1 = SearchAndGetAddresses(0x0044B99D, 0x0044BB3D, 0x0044BB3D, SearchBytes1, sizeof(SearchBytes1), 0x01);
	constexpr BYTE SearchBytes2[]{ 0x05, 0x00, 0x00, 0x08, 0x00 };
	const DWORD Addr2 = SearchAndGetAddresses(0x00496F87, 0x00497231, 0x00497331, SearchBytes2, sizeof(SearchBytes2), 0x00);
	constexpr BYTE SearchBytes3[]{ 0x05, 0x00, 0x48, 0x10, 0x00 };
	const DWORD Addr3 = SearchAndGetAddresses(0x0049B40A, 0x0049B6BA, 0x0049AF7A, SearchBytes3, sizeof(SearchBytes3), 0x00);

	// Find UFO addresses
	constexpr BYTE UFOSearchBytes[]{ 0x05, 0xFF, 0x07, 0x00, 0x00, 0x25, 0x00, 0xF8, 0xFF, 0xFF, 0x03, 0xC2, 0x50, 0x68 };
	const DWORD UfoAddr1 = SearchAndGetAddresses(0x0057E84E, 0x0057F0FE, 0x0057EA1E, UFOSearchBytes, sizeof(UFOSearchBytes), -0x28);
	const DWORD UfoAddr2 = UfoAddr1 + 0x0F;
	const DWORD UfoAddr3 = SearchAndGetAddresses(0x0058C31E, 0x0058CBCE, 0x0058C4EE, UFOSearchBytes, sizeof(UFOSearchBytes), -0x28);
	const DWORD UfoAddr4 = UfoAddr3 + 0x0F;

	// Checking address pointer
	if (!Addr1 || !Addr2 || !Addr3 || !UfoAddr1 || !UfoAddr3)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get size of textures
	DWORD Size = GetTexBufferSize();
	if (!Size)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find texture buffer size!";
		return;
	}
	BufferSize = Size + max(Size, 2 * 1024 * 1024);
	BufferSize3 = Size * 4;
	Logging::Log() << "Setting texture buffer size: " << BufferSize;

	// Allocate dynamic memory for loading textures
	PtrBytes1 = new BYTE[BufferSize];
	PtrBytes2 = new BYTE[BufferSize];
	PtrBytes3 = new BYTE[BufferSize3];
	if (!PtrBytes1 || !PtrBytes2 || !PtrBytes3)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to create texture buffer!";
		return;
	}

	// Clear texture buffers
	ZeroMemory(PtrBytes1, BufferSize);
	ZeroMemory(PtrBytes2, BufferSize);
	ZeroMemory(PtrBytes3, BufferSize3);

	// Logging update
	Logging::Log() << "Updating Texture memory address locations...";

	// Write new memory static address
	UpdateMemoryAddress((void*)StaticAddr, &PtrBytes1, sizeof(void*));

	// Write new memory address 1
	UpdateMemoryAddress((void*)Addr1, &PtrBytes1, sizeof(void*));

	// Write new memory address 2
	UpdateMemoryAddress((void*)Addr2, "\xB8", sizeof(BYTE));		// Change from 'add' to 'mov'
	UpdateMemoryAddress((void*)(Addr2 + 1), &PtrBytes2, sizeof(void*));

	// Write new memory address 3
	UpdateMemoryAddress((void*)Addr3, "\xB8", sizeof(BYTE));		// Change from 'add' to 'mov'
	UpdateMemoryAddress((void*)(Addr3 + 1), &PtrBytes2, sizeof(void*));

	// Write addresses for lake UFO sighting
	UpdateMemoryAddress((void*)UfoAddr1, &PtrBytes3, sizeof(void*));
	UpdateMemoryAddress((void*)UfoAddr2, &PtrBytes3, sizeof(void*));

	// Write addresses for hospital UFO sighting
	UpdateMemoryAddress((void*)UfoAddr3, &PtrBytes3, sizeof(void*));
	UpdateMemoryAddress((void*)UfoAddr4, &PtrBytes3, sizeof(void*));

	// Write jmp to memory
	WriteJMPtoMemory((BYTE*)ClearBuffAddr, TexBufferASM);
}
