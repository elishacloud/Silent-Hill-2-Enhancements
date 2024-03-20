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
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

typedef int(__cdecl *oTextToGameAsmProc)(unsigned __int16* a1, unsigned __int16 a2);

// Forward declarations
DWORD GetDiskSpace();

// Variables
const int StringSize = 12;
bool DiskSizeSet = false;
char szNewFreeSpaceString[StringSize] = { '\0' };
char* szNewFreeSpaceStringUnits = nullptr;

// Variables for ASM
void *jmpSkipDisk;
void *jmpNewSaveReturnAddr;
void *jmpHardDriveReturnAddr;
void *jmpSkipDisplay;
void *jmpDisplayReturnAddr;
void *jmpRemoveKBAddr;

// ASM function to update 2TB disk limit
__declspec(naked) void __stdcall HardDriveASM()
{
	__asm
	{
		push eax
		push ecx
		push edx
		call GetDiskSpace
		cmp eax, 0x08		// Require at least 8KBs of disk space
		pop edx
		pop ecx
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
		push ecx
		push edx
		call GetDiskSpace
		cmp eax, 0x20		// Require at least 32KBs of disk space for new save
		pop edx
		pop ecx
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
		push ecx
		push edx
		call GetDiskSpace
		cmp eax, 0x3FFFFFFF
		pop edx
		pop ecx
		pop eax
		jb near SmallDiskSpace
		jmp jmpSkipDisplay

	SmallDiskSpace:
		cmp esi, 0x3B9AC9FF
		jmp jmpDisplayReturnAddr
	}
}

// ASM function to update 2TB disk limit
__declspec(naked) void __stdcall oTextToGameAsm()
{
	__asm
	{
		mov eax, dword ptr ss : [esp + 4]
		test eax, eax
		jmp jmpRemoveKBAddr
	}
}
oTextToGameAsmProc oTextToGame = (oTextToGameAsmProc)oTextToGameAsm;

// Remove "KB" text
int __cdecl TextToGame(unsigned __int16* a1, unsigned __int16 a2)
{
	int TTG = oTextToGame(a1, a2);

	if (a2 == 147)
	{
		for (DWORD x = 0; x < 50; x++)
		{
			// ASCII chars
			if (((char*)TTG)[x + 0] == '\x2B' &&
				((char*)TTG)[x + 1] == '\x22' &&
				((char*)TTG)[x + 2] == '\xFF' &&
				((char*)TTG)[x + 3] == '\xFF')
			{
				constexpr byte RemoveKBPatch[] = { 0x00, 0x00 };
				UpdateMemoryAddress((void*)(TTG + x), RemoveKBPatch, sizeof(RemoveKBPatch));
				break;
			}
			// Shift-JIS chars
			if (((char*)TTG)[x + 0] == '\x86' &&
				((char*)TTG)[x + 1] == '\x01' &&
				((char*)TTG)[x + 2] == '\x7D' &&
				((char*)TTG)[x + 3] == '\x01' &&
				((char*)TTG)[x + 4] == '\xFF' &&
				((char*)TTG)[x + 5] == '\xFF')
			{
				constexpr byte RemoveKBPatch[] = { 0x00, 0x00, 0x00, 0x00 };
				UpdateMemoryAddress((void*)(TTG + x), RemoveKBPatch, sizeof(RemoveKBPatch));
				break;
			}
			// End of string
			if (((char*)TTG)[x + 0] == '\xFF' &&
				((char*)TTG)[x + 1] == '\xFF')
			{
				break;
			}
		}
	}
	return TTG;
}

// Get amount of free disk space in KBs, greater than 0x7FFFFFFF will simply return 0x7FFFFFFF
DWORD GetDiskSpace()
{
	static wchar_t DirectoryName[MAX_PATH] = { '\0' };
	static bool GetFolder = true;

	if (GetFolder)
	{
		bool ret = GetSH2FolderPath(DirectoryName, MAX_PATH);
		wchar_t* pdest = wcsrchr(DirectoryName, '\\');
		if (ret && pdest)
		{
			*pdest = '\0';
		}

		GetFolder = false;
	}

	ULARGE_INTEGER FreeBytesAvailableToCaller = { NULL };
	if (!GetDiskFreeSpaceEx(DirectoryName, &FreeBytesAvailableToCaller, nullptr, nullptr))
	{
		RUNCODEONCE(Logging::Log() << __FUNCTION__ << " Error: failed to get available disk space!");

		DiskSizeSet = false;
		return NULL;
	}

	DiskSizeSet = true;
	if (FreeBytesAvailableToCaller.QuadPart < 0xF4240)
	{
		szNewFreeSpaceStringUnits = "KB";
		_snprintf_s(szNewFreeSpaceString, StringSize - 6, _TRUNCATE, "%u", (UINT)FreeBytesAvailableToCaller.QuadPart);
	}
	else if (FreeBytesAvailableToCaller.QuadPart / 1024 < 0xF4240)
	{
		szNewFreeSpaceStringUnits = "MB";
		_snprintf_s(szNewFreeSpaceString, StringSize - 6, _TRUNCATE, "%u", (UINT)((double)FreeBytesAvailableToCaller.QuadPart / 1024.0f / 1024.0f));
	}
	else if (FreeBytesAvailableToCaller.QuadPart / 1024 < 0x3B9ACA00)
	{
		szNewFreeSpaceStringUnits = "GB";
		_snprintf_s(szNewFreeSpaceString, StringSize - 6, _TRUNCATE, "%.1f", (double)FreeBytesAvailableToCaller.QuadPart / 1024.0f / 1024.0f / 1024.0f);
	}
	else if (FreeBytesAvailableToCaller.QuadPart / 1024 >= 0x3B9ACA00)
	{
		szNewFreeSpaceStringUnits = "TB";
		_snprintf_s(szNewFreeSpaceString, StringSize - 6, _TRUNCATE, "%.2f", (double)FreeBytesAvailableToCaller.QuadPart / 1024.0f / 1024.0f / 1024.0f / 1024.0f);
	}

	ULONGLONG FreeSpace = FreeBytesAvailableToCaller.QuadPart / 1024;
	if (FreeSpace > 0x7FFFFFFF)
	{
		RUNCODEONCE(Logging::Log() << __FUNCTION__ << " Available disk space larger than 2TBs: " << FreeSpace);
		return 0x7FFFFFFF;	// Largest unsigned number
	}

	RUNCODEONCE(Logging::Log() << __FUNCTION__ << " Available disk space smaller than 2TBs: " << FreeSpace);
	return (DWORD)(FreeSpace);
}

// sprintf replacement for printing diskspace
int PrintFreeDiskSpace(char* Buffer, const char* a1, ...)
{
	if (Buffer == nullptr || a1 == nullptr)
	{
		return sprintf(Buffer, a1);
	}

	char FullMessageBufferReturn[StringSize] = { 0 };

	va_list vaReturn;
	va_start(vaReturn, a1);

	_vsnprintf_s(FullMessageBufferReturn, StringSize, _TRUNCATE, a1, vaReturn);
	va_end(vaReturn);

	bool UsingHEX = (FullMessageBufferReturn[0] == '\\' && FullMessageBufferReturn[1] == 'h');

	if (DiskSizeSet)
	{
		return _snprintf_s(Buffer, StringSize, _TRUNCATE, UsingHEX ? "\\h%s %s" : "%s%s", szNewFreeSpaceString, szNewFreeSpaceStringUnits);
	}
	else
	{
		return _snprintf_s(Buffer, StringSize, _TRUNCATE, UsingHEX ? "%s %s" : "%s%s", FullMessageBufferReturn, "KB");
	}
}

// Patch SH2 code to Fix 2TB disk limit
void Patch2TBHardDrive()
{
	// 2TB disk check fix
	constexpr BYTE HardDriveSearchBytes[]{ 0x75, 0x08, 0x5F, 0xB8, 0x02, 0x00, 0x00, 0x00, 0x5E, 0xC3, 0x84, 0xDB, 0x75, 0x14, 0x83, 0xFE, 0x20, 0x7C, 0x0F };
	DWORD HardDriveAddr = SearchAndGetAddresses(0x0044B86E, 0x0044BA0E, 0x0044BA0E, HardDriveSearchBytes, sizeof(HardDriveSearchBytes), 0x22, __FUNCTION__);

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
	DWORD DisplayFix = SearchAndGetAddresses(0x0044FB54, 0x0044FDB4, 0x0044FDB4, DisplaySearchBytes, sizeof(DisplaySearchBytes), 0x1A, __FUNCTION__);
	constexpr BYTE RemoveKBSearchBytes[]{ 0x8B, 0x44, 0x24, 0x04, 0x85, 0xC0, 0x74, 0x16, 0x66, 0x8B, 0x4C, 0x24, 0x08 };
	DWORD RemoveKBAddr = SearchAndGetAddresses(0x0047EC60, 0x0047EF00, 0x0047F110, RemoveKBSearchBytes, sizeof(RemoveKBSearchBytes), 0x0, __FUNCTION__);

	// Checking address pointer
	if (!DisplayFix || !RemoveKBAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	jmpSkipDisplay = (void*)(DisplayFix + 0x08);
	jmpDisplayReturnAddr = (void*)(DisplayFix + 0x06);
	jmpRemoveKBAddr = (void*)(RemoveKBAddr + 6);
	DWORD sprintfAddr = DisplayFix + 0x42;

	// Update SH2 code
	Logging::Log() << "Setting 2TB hard disk Fix...";
	WriteJMPtoMemory((BYTE*)(HardDriveAddr - 0x14), *NewSaveASM, 5);
	WriteJMPtoMemory((BYTE*)HardDriveAddr, *HardDriveASM, 5);
	WriteJMPtoMemory((BYTE*)DisplayFix, *DisplayASM, 6);
	WriteCalltoMemory((BYTE*)sprintfAddr, *PrintFreeDiskSpace, 6);
	WriteJMPtoMemory((BYTE*)RemoveKBAddr, *TextToGame, 6);
}
