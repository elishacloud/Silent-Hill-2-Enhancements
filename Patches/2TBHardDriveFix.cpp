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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Forward declarations
DWORD GetDiskSpace();

// Variables for ASM
void *jmpSkipDisk;
void *jmpNewSaveReturnAddr;
void *jmpHardDriveReturnAddr;
void *jmpSkipDisplay;
void *jmpDisplayReturnAddr;

// ASM function to update 2TB disk limit
__declspec(naked) void __stdcall HardDriveASM()
{
	__asm
	{
		push eax
		push ebx
		push ecx
		call GetDiskSpace
		cmp eax, 0x08		// Require at least 8KBs of disk space
		pop ecx
		pop ebx
		pop eax
		ja near EnoughDiskSpace
		jmp jmpSkipDisk

	EnoughDiskSpace:
		jmp jmpHardDriveReturnAddr
	}
}

// ASM function for new save
__declspec(naked) void __stdcall NewSaveASM()
{
	__asm
	{
		push eax
		push ebx
		push ecx
		call GetDiskSpace
		cmp eax, 0x20		// Require at least 32KBs of disk space for new save
		pop ecx
		pop ebx
		pop eax
		ja near EnoughDiskSpace
		jmp HardDriveASM

	EnoughDiskSpace:
		jmp jmpNewSaveReturnAddr
	}
}

// ASM function to update hard disk display
__declspec(naked) void __stdcall DisplayASM()
{
	__asm
	{
		push eax
		push ebx
		push ecx
		call GetDiskSpace
		cmp eax, 0x3FFFFFFF
		pop ecx
		pop ebx
		pop eax
		jb near SmallDiskSpace
		jmp jmpSkipDisplay

	SmallDiskSpace:
		cmp esi, 0x3B9AC9FF
		jmp jmpDisplayReturnAddr
	}
}

// Get amount of free disk space in KBs, greater than 0x7FFFFFFF will simply return 0x7FFFFFFF
DWORD GetDiskSpace()
{
	static wchar_t DirectoryName[MAX_PATH] = { '\0' };
	static bool GetFolder = true;

	if (GetFolder)
	{
		GetModuleFileName(nullptr, DirectoryName, MAX_PATH);
		wcscpy_s(wcsrchr(DirectoryName, '\\'), MAX_PATH - wcslen(DirectoryName), L"\0");

		GetFolder = false;
	}

	ULARGE_INTEGER TotalNumberOfFreeBytes = { NULL };
	if (!GetDiskFreeSpaceEx(DirectoryName, nullptr, nullptr, &TotalNumberOfFreeBytes))
	{
		RUNCODEONCE(Logging::Log() << __FUNCTION__ << " Error: failed to get available disk space!");

		return NULL;
	}

	ULONGLONG FreeSpace = TotalNumberOfFreeBytes.QuadPart / 1024;
	if (FreeSpace > 0x7FFFFFFF)
	{
		RUNCODEONCE(Logging::Log() << __FUNCTION__ << " Available disk space larger than 2TBs: " << FreeSpace);
		return 0x7FFFFFFF;	// Largest unsigned number
	}

	RUNCODEONCE(Logging::Log() << __FUNCTION__ << " Available disk space smaller than 2TBs: " << FreeSpace);
	return (DWORD)(FreeSpace);
}

// Patch SH2 code to Fix 2TB disk limit
void Patch2TBHardDrive()
{
	// 2TB disk check fix
	constexpr BYTE HardDriveSearchBytes[]{ 0x75, 0x08, 0x5F, 0xB8, 0x02, 0x00, 0x00, 0x00, 0x5E, 0xC3, 0x84, 0xDB, 0x75, 0x14, 0x83, 0xFE, 0x20, 0x7C, 0x0F };
	DWORD HardDriveAddr = SearchAndGetAddresses(0x0044B86E, 0x0044BA0E, 0x0044BA0E, HardDriveSearchBytes, sizeof(HardDriveSearchBytes), 0x22);

	// Checking address pointer
	if (!HardDriveAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpSkipDisk = (void*)(HardDriveAddr + 0x1A);
	jmpHardDriveReturnAddr = (void*)(HardDriveAddr + 0x05);
	jmpNewSaveReturnAddr = (void*)(HardDriveAddr - 0x0F);

	// Disk display fix
	constexpr BYTE DisplaySearchBytes[]{ 0x8B, 0xF0, 0x83, 0xC4, 0x04, 0x85, 0xF6, 0x7D, 0x02, 0x33, 0xF6, 0x6A, 0x00 };
	DWORD DisplayFix = SearchAndGetAddresses(0x0044FB54, 0x0044FDB4, 0x0044FDB4, DisplaySearchBytes, sizeof(DisplaySearchBytes), 0x1A);

	// Checking address pointer
	if (!DisplayFix)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpSkipDisplay = (void*)(DisplayFix + 0x08);
	jmpDisplayReturnAddr = (void*)(DisplayFix + 0x06);

	// Update SH2 code
	Logging::Log() << "Setting 2TB hard disk Fix...";
	WriteJMPtoMemory((BYTE*)(HardDriveAddr - 0x14), *NewSaveASM, 5);
	WriteJMPtoMemory((BYTE*)HardDriveAddr, *HardDriveASM, 5);
	WriteJMPtoMemory((BYTE*)DisplayFix, *DisplayASM, 6);
}
