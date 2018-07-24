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
*
* ASI plugin loader taken from source code found in Ultimate ASI Loader
* https://github.com/ThirteenAG/Ultimate-ASI-Loader
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include "External\MemoryModule\MemoryModule.h"
#include "Settings.h"
#include "Utils.h"
#include "Logging.h"

// Memory modules
struct MMODULE
{
	HMEMORYMODULE handle;		// Module handle
	DWORD ResID;				// Resource ID
};
std::vector<MMODULE> HMModules;

// Find asi plugins to load
void FindFiles(WIN32_FIND_DATA* fd)
{
	wchar_t dir[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, dir);

	HANDLE asiFile = FindFirstFile(L"*.asi", fd);
	if (asiFile != INVALID_HANDLE_VALUE)
	{
		do {
			if (!(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				auto pos = wcslen(fd->cFileName);

				if (fd->cFileName[pos - 4] == '.' &&
					(fd->cFileName[pos - 3] == 'a' || fd->cFileName[pos - 3] == 'A') &&
					(fd->cFileName[pos - 2] == 's' || fd->cFileName[pos - 2] == 'S') &&
					(fd->cFileName[pos - 1] == 'i' || fd->cFileName[pos - 1] == 'I'))
				{
					wchar_t path[MAX_PATH] = { 0 };
					swprintf_s(path, L"%s\\%s", dir, fd->cFileName);

					auto h = LoadLibrary(path);
					SetCurrentDirectory(dir); //in case asi switched it

					if (h)
					{
						AddHandleToVector(h);
						Log() << "Loaded '" << fd->cFileName << "'";
					}
					else
					{
						Log() << __FUNCTION__ << " Error: Unable to load '" << fd->cFileName << "'. Error: " << GetLastError();
					}
				}
			}
		} while (FindNextFile(asiFile, fd));
		FindClose(asiFile);
	}
}

// Load asi plugins
void LoadASIPlugins(bool LoadFromScriptsOnlyFlag)
{
	Log() << "Loading ASI Plugins";

	wchar_t oldDir[MAX_PATH] = { 0 }; // store the current directory
	GetCurrentDirectory(MAX_PATH, oldDir);

	wchar_t selfPath[MAX_PATH] = { 0 };
	HMODULE hModule = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)LoadASIPlugins, &hModule);
	GetModuleFileName(hModule, selfPath, MAX_PATH);
	*wcsrchr(selfPath, '\\') = '\0';
	SetCurrentDirectory(selfPath);

	WIN32_FIND_DATA fd;
	if (!LoadFromScriptsOnlyFlag)
	{
		FindFiles(&fd);
	}

	SetCurrentDirectory(selfPath);

	if (SetCurrentDirectory(L"scripts\\"))
	{
		FindFiles(&fd);
	}

	SetCurrentDirectory(selfPath);

	if (SetCurrentDirectory(L"plugins\\"))
	{
		FindFiles(&fd);
	}

	SetCurrentDirectory(oldDir); // Reset the current directory
}

// Load memory module from resource
void LoadModuleFromResource(HMODULE hModule, DWORD ResID, LPCWSTR lpName)
{
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(ResID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource(hLoadedResource);
			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource(hModule, hResource);
				if (dwResourceSize != 0)
				{
					Log() << "Loading the " << lpName << " module...";
					HMEMORYMODULE hMModule = MemoryLoadLibrary((const void*)pLockedResource, dwResourceSize);
					if (hMModule)
					{
						MMODULE MMItem = { hMModule , ResID };
						HMModules.push_back(MMItem);
						return;
					}
					else
					{
						Log() << __FUNCTION__ << " Error: " << lpName << " module could not be loaded!";
					}
				}
			}
		}
	}
	Log() << __FUNCTION__ << " Error: failed to load " << lpName << " module!";
}

// Load memory module from resource
void LoadModuleFromResourceToFile(HMODULE hModule, DWORD ResID, LPCWSTR lpName, LPCWSTR lpFilepath)
{
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(ResID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource(hLoadedResource);
			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource(hModule, hResource);
				if (dwResourceSize != 0)
				{
					Log() << "Loading the " << lpName << " module...";
					std::fstream fsModule;
					fsModule.open(lpFilepath, std::ios_base::out | std::ios_base::binary);
					if (fsModule.is_open())
					{
						fsModule.write((char*)pLockedResource, dwResourceSize);
						fsModule.close();
						HMODULE h_Module = LoadLibrary(lpFilepath);
						if (h_Module)
						{
							AddHandleToVector(h_Module);
							return;
						}
						else
						{
							Log() << __FUNCTION__ << " Error: " << lpName << " module could not be loaded!";
						}
					}
					else
					{
						Log() << __FUNCTION__ << " Error: " << lpName << " module could not be written!";
					}
				}
			}
		}
	}
	Log() << __FUNCTION__ << " Error: failed to load " << lpName << " module!";
}

// Load modupdater
void LoadModUpdater(HMODULE hModule, DWORD ResID)
{
	// Get module name
	wchar_t Path[MAX_PATH], Name[MAX_PATH];
	GetModuleFileName(hModule, Path, MAX_PATH);
	wcscpy_s(Name, MAX_PATH, wcsrchr(Path, '\\'));

	// Get 'temp' path
	GetTempPath(MAX_PATH, Path);
	wcscat_s(Path, MAX_PATH, L"~tmp_sh2_enhce");
	CreateDirectory(Path, nullptr);

	// Update path with module name
	wcscat_s(Path, MAX_PATH, Name);
	wcscpy_s(wcsrchr(Path, '.'), MAX_PATH, L".tmp");

	// Load module
	LoadModuleFromResourceToFile(hModule, ResID, L"modupdater", Path);
}

// Get config settings from string (file)
void __stdcall ParseWndCallback(char* lpName, char* lpValue)
{
	// Check settings
#define GET_WNDMODE_VALUES(name) \
	if (!_strcmpi(lpName, #name)) WndModeConfig.name = atoi(lpValue);

	VISIT_WNDMODE_SETTINGS(GET_WNDMODE_VALUES);
}

// Load WndMode
void LoadWndMode(HMODULE hModule, DWORD ResID)
{
	// Get 'temp' path
	wchar_t wndPath[MAX_PATH];
	GetTempPath(MAX_PATH, wndPath);
	wcscat_s(wndPath, MAX_PATH, L"~tmp_sh2_enhce");
	CreateDirectory(wndPath, nullptr);
	wcscat_s(wndPath, MAX_PATH, L"\\wndmode.ini");

	// Get wndmode config settings
	wchar_t configPath[MAX_PATH];
	GetModuleFileName(hModule, configPath, MAX_PATH);
	wcscpy_s(wcsrchr(configPath, '.'), MAX_PATH - wcslen(configPath), L".ini");
	char* szCfg = Read(configPath);

	// Parce config file
	if (szCfg)
	{
		// Store settings
		Parse(szCfg, ParseWndCallback);
		free(szCfg);

		// Open ini file
		std::ofstream WndMode_ini;
		WndMode_ini.open(wndPath, std::ios::trunc);

		// Write to ini file
		if (WndMode_ini.is_open())
		{
			WndMode_ini << "[WINDOWMODE]\n";

			// Write settings
#define SET_WNDMODE_VALUES(name) \
	WndMode_ini << #name << "=" << WndModeConfig.name << "\n";

			VISIT_WNDMODE_SETTINGS(SET_WNDMODE_VALUES);

			// Close file
			WndMode_ini.close();
		}
		else
		{
			Log() << __FUNCTION__ << " Error: Could not write WndMode settings!";
		}
	}
	else
	{
		Log() << __FUNCTION__ << " Error: Could not read WndMode settings!";
	}

	// Set WndMode path
	wcscpy_s(wcsrchr(wndPath, '.'), MAX_PATH - wcslen(wndPath), L".tmp");

	// Load module
	LoadModuleFromResourceToFile(hModule, ResID, L"WndMode", wndPath);
}

// Unload resource memory modules
void UnloadResourceModules()
{
	for (MMODULE it : HMModules)
	{
		MemoryFreeLibrary(it.handle);
	}
}
