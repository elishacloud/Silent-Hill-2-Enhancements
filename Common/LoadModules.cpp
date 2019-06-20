/**
* Copyright (C) 2019 Elisha Riedlinger
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
#include <Shlwapi.h>
#include <vector>
#include "LoadModules.h"
#include "External\Hooking\Hook.h"
#include "FileSystemHooks.h"
#include "Settings.h"
#include "Utils.h"
#include "Logging\Logging.h"

int n = 0;
LPCWSTR exTempPath = L"~tmp_sh2_enhce";

typedef void(WINAPI *PFN_InitializeASI)(void);

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
						InitializeASI(h);
						Logging::Log() << "Loaded '" << fd->cFileName << "'";
					}
					else
					{
						Logging::Log() << __FUNCTION__ << " Error: Unable to load '" << fd->cFileName << "'. Error: " << GetLastError();
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
	Logging::Log() << "Loading ASI Plugins";

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
HMEMORYMODULE LoadModuleFromResource(HMODULE hModule, DWORD ResID, LPCWSTR lpName)
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
					Logging::Log() << "Loading the " << lpName << " module...";
					HMEMORYMODULE hMModule = MemoryLoadLibrary((const void*)pLockedResource, dwResourceSize);
					if (hMModule)
					{
						MMODULE MMItem = { hMModule , ResID };
						HMModules.push_back(MMItem);
						InitializeASI(hMModule);
						return (HMODULE)hMModule;
					}
					else
					{
						Logging::Log() << __FUNCTION__ << " Error: " << lpName << " module could not be loaded!";
					}
				}
			}
		}
	}
	Logging::Log() << __FUNCTION__ << " Error: failed to load " << lpName << " module!";

	return nullptr;
}

// Load memory module from resource
HMODULE LoadModuleFromResourceToFile(HMODULE hModule, DWORD ResID, LPCWSTR lpName, LPCWSTR lpFilepath)
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
					Logging::Log() << "Loading the " << lpName << " module...";
					std::fstream fsModule;
					fsModule.open(lpFilepath, std::ios_base::out | std::ios_base::binary);
					if (fsModule.is_open())
					{
						fsModule.write((char*)pLockedResource, dwResourceSize);
						fsModule.close();
						SetFileAttributes(lpFilepath, FILE_ATTRIBUTE_TEMPORARY);
						HMODULE h_Module = LoadLibrary(lpFilepath);
						if (h_Module)
						{
							AddHandleToVector(h_Module);
							return h_Module;
						}
						else
						{
							Logging::Log() << __FUNCTION__ << " Error: " << lpName << " module could not be loaded!";
						}
					}
					else
					{
						Logging::Log() << __FUNCTION__ << " Error: " << lpName << " module could not be written!";
					}
				}
			}
		}
	}
	Logging::Log() << __FUNCTION__ << " Error: failed to load " << lpName << " module!";

	return nullptr;
}

// Initialize ASI module
void InitializeASI(HMODULE hModule)
{
	if (!hModule)
	{
		return;
	}

	PFN_InitializeASI p_InitializeASI = (PFN_InitializeASI)GetProcAddress(hModule, "InitializeASI");

	if (!p_InitializeASI)
	{
		return;
	}

	p_InitializeASI();
}

// Initialize ASI module
void InitializeASI(HMEMORYMODULE hModule)
{
	if (!hModule)
	{
		return;
	}

	PFN_InitializeASI p_InitializeASI = (PFN_InitializeASI)MemoryGetProcAddress(hModule, "InitializeASI");

	if (!p_InitializeASI)
	{
		return;
	}

	p_InitializeASI();
}

HRESULT DeleteAllfiles(LPCWSTR lpFolder)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError;

	std::wstring FilePath(lpFolder);
	FilePath.append(L"\\");

	hFind = FindFirstFile(std::wstring(FilePath + L"*.*").c_str(), &FindFileData);

	do {
		DeleteFile(std::wstring(FilePath + FindFileData.cFileName).c_str());
	} while (FindNextFile(hFind, &FindFileData) != 0);

	dwError = GetLastError();
	FindClose(hFind);
	if (dwError != ERROR_NO_MORE_FILES)
	{
		return dwError;
	}

	return S_OK;
}

// Load mod from file
void LoadModuleFromFile(HMODULE hModule, DWORD ResID, LPCWSTR lpConfigName, LPCWSTR lpConfigPath, LPCWSTR lpName)
{
	// Get module name
	wchar_t Name[MAX_PATH], Path[MAX_PATH], Config[MAX_PATH];
	GetModuleFileName(hModule, Path, MAX_PATH);
	wcscpy_s(Name, MAX_PATH, wcsrchr(Path, '\\'));

	// Get 'temp' path
	if (!GetTempPath(MAX_PATH, Path))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get temp path!";
		return;
	}
	wcscat_s(Path, MAX_PATH, exTempPath);

	// Use unique folder path
	wcscat_s(Path, MAX_PATH, std::to_wstring(++n).c_str());

	// Remove all files from directory
	DeleteAllfiles(Path);

	// Create folder if does not exist
	CreateDirectory(Path, nullptr);
	if (!PathFileExists(Path))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to create temp folder!";
		return;
	}

	// Get config path name
	wcscpy_s(Config, MAX_PATH, Path);
	if (lpConfigName)
	{
		wcscat_s(Config, MAX_PATH, L"\\");
		wcscat_s(Config, MAX_PATH, lpConfigName);
	}
	else
	{
		wcscat_s(Config, MAX_PATH, Name);
		wcscpy_s(wcsrchr(Config, '.'), MAX_PATH - wcslen(Config), L".ini");
	}
	CopyFile(lpConfigPath, Config, FALSE);

	// Update path with module name
	wcscat_s(Path, MAX_PATH, Name);
	wcscpy_s(wcsrchr(Path, '.'), MAX_PATH - wcslen(Path), L".asi");

	// Load module
	LoadModuleFromResourceToFile(hModule, ResID, lpName, Path);
}

// Delete temp folders
void RemoveTempFolders()
{
	// Get 'temp' path
	wchar_t Path[MAX_PATH], TempPath[MAX_PATH];
	if (!GetTempPath(MAX_PATH, TempPath))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get temp path!";
		return;
	}
	wcscat_s(TempPath, MAX_PATH, exTempPath);

	// Delete folder path
	for (int x = 0; x <= n; x++)
	{
		wcscpy_s(Path, MAX_PATH, TempPath);
		if (x)
		{
			wcscat_s(Path, MAX_PATH, std::to_wstring(x).c_str());
		}
		DeleteAllfiles(Path);
		RemoveDirectory(Path);
	}
}

// Unload resource memory modules
void UnloadResourceModules()
{
	for (MMODULE it : HMModules)
	{
		MemoryFreeLibrary(it.handle);
	}
}
