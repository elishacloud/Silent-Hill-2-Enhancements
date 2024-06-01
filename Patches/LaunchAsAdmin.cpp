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
#include <psapi.h>
#include <string>
#include <regex>
#include <shellapi.h>
#include <shlwapi.h>
#pragma warning( suppress : 4091 )
#include <Shlobj.h>
#include <iostream>
#include <fstream>
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\LoadModules.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

const wchar_t *ArgString = L"SH2-PID=";
const DWORD ArgSize = wcslen(ArgString);

bool ProcessRelaunched = false;
bool NeedsRestart = false;

// Check arguments to see if a pid is sent as an argument
void CheckArgumentsForPID()
{
	wchar_t sh2path[MAX_PATH];

	// Retrieve command line arguments
	LPWSTR *szArgList;
	int argCount;
	szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);

	// If arguments
	if (szArgList && argCount && GetSH2FolderPath(sh2path, MAX_PATH))
	{
		for (int i = 0; i < argCount; i++)
		{
			DWORD pid = 0;

			if (wcslen(szArgList[i]) >= ArgSize && wcsstr(szArgList[i], ArgString))
			{
				ProcessRelaunched = true;
				pid = _wtoi(szArgList[i] + ArgSize);
			}

			// If argument is a number
			if (pid)
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

// Check if running elevated
BOOL IsElevated()
{
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
		{
			fRet = Elevation.TokenIsElevated;
		}
	}
	if (hToken)
	{
		CloseHandle(hToken);
	}
	return fRet;
}

// Relaunch Silent Hill 2 with administrator rights
void RelaunchSilentHill2()
{
	// Only relaunch once
	static bool RunOnce = false;
	if (ProcessRelaunched || RunOnce)
	{
		return;
	}
	RunOnce = true;

	// Check if already running elevated
	if (IsElevated())
	{
		return;
	}

	// Check Silent Hill 2 file path
	wchar_t sh2path[MAX_PATH];
	if (GetSH2FolderPath(sh2path, MAX_PATH))
	{
		// Get pid
		wchar_t parameter[40] = { '\0' };
		wcscpy_s(parameter, ArgString);
		_itow_s(GetCurrentProcessId(), parameter + ArgSize, 40 - ArgSize, 10);

		// Administrator access required, re-launching with administrator access
		if (ShellExecute(nullptr, L"runas", sh2path, parameter, nullptr, SW_SHOWDEFAULT) > (HINSTANCE)32)
		{
			exit(0);
		}
	}
}

// Check if administrator access is required
void CheckAdminAccess()
{
	// Log activity
	Logging::Log() << "Checking if administrator access is required.";

	if (ProcessRelaunched)
	{
		Logging::Log() << "Detected relaunch with administrator rights!";
		return;
	}

	// Get Silent Hill 2 file path
	wchar_t sh2path[MAX_PATH];
	if (GetSH2FolderPath(sh2path, MAX_PATH))
	{
		// Check if restart is needed
		if (NeedsRestart)
		{
			RelaunchSilentHill2();
			return;
		}

		// Check compatibility options
		if (CheckCompatibilityMode && GetEnvironmentVariableA("__COMPAT_LAYER", nullptr, 0))
		{
			// Get compatibility layer variables
			char str[MAX_PATH] = { '\0' };
			GetEnvironmentVariableA("__COMPAT_LAYER", str, MAX_PATH);

			// Remove non-applicable compatibility variables
			std::string string(str);
			for (auto& remove : {" ", "ElevateCreateProcess"})
			{
				string = std::regex_replace(string, std::regex(remove), "");
			}

			// Restart in admin mode if needed
			if (string.size())
			{
				RelaunchSilentHill2();
				return;
			}
		}

		// Get path for temp file
		wchar_t tmpfile[MAX_PATH];
		wcscpy_s(tmpfile, MAX_PATH, sh2path);
		wchar_t* pdest = wcsrchr(tmpfile, '\\');
		if (pdest)
		{
			*pdest = '\0';
		}
		wcscat_s(tmpfile, MAX_PATH, L"\\~sh2check.dll");		// Needs to be a dll or exe file to bypass Windows' VirtualStore

		// Check if file can be created in Silent Hill 2 folder
		std::ofstream outfile(tmpfile);

		if (outfile.is_open())
		{
			outfile.close();
			DeleteFile(tmpfile);
		}
		else
		{
			RelaunchSilentHill2();
			return;
		}
	}
}

void RemoveVirtualStoreFiles()
{
	// Get Silent Hill 2 file path
	wchar_t sh2path[MAX_PATH];
	if (!GetSH2FolderPath(sh2path, MAX_PATH))
	{
		return;
	}

	// Remove Silent Hill 2 process name from path
	wchar_t *pdest = wcsrchr(sh2path, '\\');
	if (pdest)
	{
		wcscpy_s(pdest, MAX_PATH - wcslen(sh2path), L"\\");
	}

	// Remove drive root from the path
	pdest = wcschr(sh2path, '\\');
	if (!pdest || !wcslen(pdest))
	{
		return;
	}

	// Get local appdata path
	wchar_t vspath[MAX_PATH];
	if (FAILED(SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, vspath)))
	{
		return;
	}

	// Append virtual store and Silent Hill 2 to local appdata path
	if (!PathAppend(vspath, L"VirtualStore\\") || !PathAppend(vspath, pdest))
	{
		return;
	}

	// Check if path exists and delete virtual store files in the root
	if (PathFileExists(vspath))
	{
		DeleteAllfiles(vspath);
		NeedsRestart = true;
	}
}

void RemoveCompatibilityRegistry(HKEY hKeyRoot, const wchar_t *sh2path)
{
	// Check Silent Hill 2 file path
	if (PathFileExists(sh2path))
	{
		HKEY hKey;
		if (RegOpenKeyEx(hKeyRoot, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 0, KEY_READ | KEY_SET_VALUE | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
		{
			DWORD dwType;
			LONG nResult = RegQueryValueEx(hKey, sh2path, NULL, &dwType, NULL, NULL);

			if (nResult == ERROR_SUCCESS)
			{
				NeedsRestart = true;
				RegDeleteValue(hKey, sh2path);
			}

			RegCloseKey(hKey);
		}

	}
}

// Remove compatibility settings
void RemoveCompatibilityMode()
{
	// Get Silent Hill 2 file path
	wchar_t sh2path[MAX_PATH];
	if (GetSH2FolderPath(sh2path, MAX_PATH))
	{
		Logging::Log() << "Checking for Compatibility Settings!";
		RemoveCompatibilityRegistry(HKEY_CURRENT_USER, sh2path);
		RemoveCompatibilityRegistry(HKEY_LOCAL_MACHINE, sh2path);
	}
}
