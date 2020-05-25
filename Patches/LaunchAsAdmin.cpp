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
void CheckAdminAccess()
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
