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
*
* ASI plugin loader taken from source code found in Ultimate ASI Loader
* https://github.com/ThirteenAG/Ultimate-ASI-Loader
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <vector>
#include "Resource.h"
#include "LoadModules.h"
#include "Patches\Patches.h"
#include "Settings.h"
#include "Utils.h"
#include "Logging\Logging.h"

typedef void(WINAPI *PFN_InitializeASI)(void);

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
	bool ret = GetModulePath(selfPath, MAX_PATH);
	wchar_t* pdest = wcsrchr(selfPath, '\\');
	if (ret && pdest)
	{
		*pdest = '\0';
	}
	SetCurrentDirectory(selfPath);

	WIN32_FIND_DATA fd;
	if (!LoadFromScriptsOnlyFlag)
	{
		FindFiles(&fd);
	}

	SetCurrentDirectory(selfPath);

	if (SetCurrentDirectory(L"\\scripts"))
	{
		FindFiles(&fd);
	}

	SetCurrentDirectory(selfPath);

	if (SetCurrentDirectory(L"\\plugins"))
	{
		FindFiles(&fd);
	}

	SetCurrentDirectory(oldDir); // Reset the current directory
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
