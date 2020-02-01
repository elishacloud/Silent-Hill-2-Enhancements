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
#include <psapi.h>
#include <string>
#include <iostream>
#include <fstream>
#include <shellapi.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

// Forward declarations
void CheckForAdminAccess();

// Variables for ASM
DWORD GameAddressPointer;
void *jmpAdminAccess;

// ASM function to to check if admin access is needed
__declspec(naked) void __stdcall AdminAccessASM()
{
	__asm
	{
		push eax
		push ebx
		push ecx
		call CheckForAdminAccess
		pop ecx
		pop ebx
		pop eax
		push dword ptr ds : [GameAddressPointer]
		jmp jmpAdminAccess
	}
}

// Check arguments to see if a pid is sent as an argument
void CheckArgumentsForPID()
{
	// Retrieve command line arguments
	LPWSTR *szArgList;
	int argCount;
	szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);

	// If arguments
	if (szArgList && argCount)
	{
		wchar_t sh2path[MAX_PATH];
		GetModuleFileName(nullptr, sh2path, MAX_PATH);

		for (int i = 0; i < argCount; i++)
		{
			int pid = _wtoi(szArgList[i]);

			// If argument is a number
			if (pid > 0)
			{
				wchar_t filename[MAX_PATH];

				HANDLE ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

				if (ProcessHandle)
				{
					if (GetModuleFileNameEx(ProcessHandle, NULL, filename, MAX_PATH))
					{
						// Terminate pid
						if (wcscmp(sh2path, filename) == 0)
						{
							TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, false, pid), 1);
							break;
						}
					}
					CloseHandle(ProcessHandle);
				}
			}
		}
	}
	LocalFree(szArgList);
}

// Check if administrator access is required
void CheckForAdminAccess()
{
	// Get Silent Hill 2 file path
	wchar_t sh2path[MAX_PATH];
	if (GetModuleFileName(nullptr, sh2path, MAX_PATH) != 0)
	{
		// Get path for temp file
		wchar_t tmpfile[MAX_PATH];
		wcscpy_s(tmpfile, MAX_PATH, sh2path);
		wcscpy_s(wcsrchr(tmpfile, '\\'), MAX_PATH - wcslen(tmpfile), L"\0");
		wcscat_s(tmpfile, MAX_PATH, L"\\~sh2check.dll");		// Needs to be a dll or exe file to bypass Windows' VirtualStore

		// Log activity
		Logging::Log() << __FUNCTION__ << " Checking if administrator access is required.";

		// Check if file can be created in Silent Hill 2 folder
		std::ofstream outfile(tmpfile);

		if (outfile.is_open())
		{
			outfile.close();
			DeleteFile(tmpfile);
			return;		// Admin access is not needed
		}
	}

	// Get pid
	wchar_t buffer[10] = { '\0' };
	_itow_s(GetCurrentProcessId(), buffer, 10);

	// Administrator access required, re-launching with administrator access
	if ((int)ShellExecute(nullptr, L"runas", sh2path, buffer, nullptr, SW_NORMAL) > 32)
	{
		exit(0);
	}
}

// Set hook at beginning of Silent Hill 2 code to check for administrator access
void UpdateAdminAccess()
{
	// Get memory pointer
	constexpr BYTE SearchBytes[]{ 0xFF, 0xD7, 0x66, 0x81, 0x38, 0x4D, 0x5A, 0x75, 0x1F, 0x8B, 0x48, 0x3C, 0x03, 0xC8, 0x81, 0x39 };
	DWORD Address = SearchAndGetAddresses(0x0056FDEB, 0x0056EBBB, 0x0056E4DB, SearchBytes, sizeof(SearchBytes), -0x13);

	// Checking address pointer
	if (!Address)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
		return;
	}
	GameAddressPointer = *(DWORD*)(Address + 2);
	jmpAdminAccess = (void*)(Address + 5);

	// Update SH2 code
	WriteJMPtoMemory((BYTE*)Address, *AdminAccessASM, 5);
}
