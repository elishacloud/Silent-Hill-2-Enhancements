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
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include "FileSystemHooks.h"
#include "Hooking\Hook.h"
#include "Common\MyStrings.h"
#include "Common\Logging.h"

#pragma comment(lib, "Shlwapi.lib")

// API typedef
typedef BOOL(WINAPI *PFN_GetModuleHandleExA)(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule);
typedef BOOL(WINAPI *PFN_GetModuleHandleExW)(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule);
typedef DWORD(WINAPI *PFN_GetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef DWORD(WINAPI *PFN_GetModuleFileNameW)(HMODULE, LPWSTR, DWORD);
typedef HANDLE(WINAPI *PFN_CreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef BOOL(WINAPI *PFN_FindNextFileA)(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
typedef DWORD(WINAPI *PFN_GetPrivateProfileStringA)(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName);
typedef DWORD(WINAPI *PFN_GetPrivateProfileStringW)(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName);

// Proc addresses
FARPROC p_GetModuleHandleExA = nullptr;
FARPROC p_GetModuleHandleExW = nullptr;
FARPROC p_GetModuleFileNameA = nullptr;
FARPROC p_GetModuleFileNameW = nullptr;
FARPROC p_CreateFileW = nullptr;
FARPROC p_FindNextFileA = nullptr;
FARPROC p_GetPrivateProfileStringA = nullptr;
FARPROC p_GetPrivateProfileStringW = nullptr;

// Variable used in hooked modules
HMODULE moduleHandle = nullptr;
char ConfigPathA[MAX_PATH];
wchar_t ConfigPathW[MAX_PATH];
char ModPathA[MAX_PATH] = "sh2e";
wchar_t ModPathW[MAX_PATH] = L"sh2e";
std::wstring wstrDataPath;
DWORD modLoc = 0;
std::wstring wstrModulePath;
DWORD nPathSize = 0;
wchar_t ConfigName[MAX_PATH] = { '\0' };
bool FileEnabled = true;
DWORD VoiceSizeLow = 0;

template<typename T>
bool CheckConfigPath(T str)
{
	std::wstring ws(toLower(toWString(str)));
	for (MODULECONFIG it : ConfigList)
	{
		if (*(it.Enabled) && wcsstr(ws.c_str(), it.ConfigFileList))
		{
			return true;
		}
	}
	return false;
}

template<typename T>
bool ReplaceWithModPath(T lpFileName, DWORD start)
{
	if (length(lpFileName) > start + 3 &&
		(lpFileName[start] == 'd' || lpFileName[start] == 'D') &&
		(lpFileName[start + 1] == 'a' || lpFileName[start + 1] == 'A') &&
		(lpFileName[start + 2] == 't' || lpFileName[start + 2] == 'T') &&
		(lpFileName[start + 3] == 'a' || lpFileName[start + 3] == 'A'))
	{
		wchar_t tmpPath[MAX_PATH];
		wcscpy_s(tmpPath, MAX_PATH, toWString(lpFileName).c_str());
		for (int x = 0; x < 4; x++)
		{
			tmpPath[start + x] = ModPathW[x];
		}

		wchar_t tmpPath2[MAX_PATH];
		wcscpy_s(tmpPath2, MAX_PATH, tmpPath);
		size_t Size = wcslen(tmpPath2);
		if (Size > 4)
		{
			tmpPath2[Size - 1] = '\0';
		}

		if (PathFileExists(tmpPath) || PathFileExists(tmpPath2))
		{
			for (int x = 0; x < 4; x++)
			{
				ConstStr(lpFileName)[start + x] = GetStringType(lpFileName, ModPathA, ModPathW)[x];
			}
			return true;
		}
	}
	return false;
}

template<typename T>
T CheckForModPath(T lpFileName)
{
	// Verify mod location and data path
	if (!UseCustomModFolder || !modLoc || modLoc + 3 > wstrDataPath.size() || toLower(toWString(lpFileName)).find(L"data\\save") != std::string::npos)
	{
		return lpFileName;
	}

	// Check if string is in the data folder
	ReplaceWithModPath(lpFileName, 0);
	ReplaceWithModPath(lpFileName, modLoc);

	return lpFileName;
}

template<typename T>
T GetFileName(T lpFileName)
{
	return (CheckConfigPath(lpFileName)) ? GetStringType(lpFileName, ConfigPathA, ConfigPathW) : CheckForModPath(lpFileName);
}

template<typename T>
bool NotValidFileNamePath(T& lpFilename, DWORD nSize)
{
	return ((nSize < 3 && lpFilename[0] != '\\' &&
		!(lpFilename[0] >= 'A' && lpFilename[0] <= 'Z' &&
			lpFilename[0] >= 'a' && lpFilename[0] <= 'z')) ||
		(nSize < 4 && lpFilename[0] != '\\' &&
			!(lpFilename[0] >= 'A' && lpFilename[0] <= 'Z' &&
				lpFilename[0] >= 'a' && lpFilename[0] <= 'z') &&
			lpFilename[1] != '\\' && lpFilename[1] != ':') ||
		(nSize < 5 && lpFilename[0] != '\\' &&
			!(lpFilename[0] >= 'A' && lpFilename[0] <= 'Z' &&
				lpFilename[0] >= 'a' && lpFilename[0] <= 'z') &&
			lpFilename[1] != '\\' && lpFilename[1] != ':' &&
			lpFilename[2] != '\\' && lpFilename[2] != ':') ||
		(nSize >= 5 && lpFilename[0] != '\\' &&
			!(lpFilename[0] >= 'A' && lpFilename[0] <= 'Z' &&
				lpFilename[0] >= 'a' && lpFilename[0] <= 'z') &&
			lpFilename[1] != '\\' && lpFilename[1] != ':' &&
			lpFilename[2] != '\\' && lpFilename[2] != ':' &&
			lpFilename[3] != '\\' && lpFilename[3] != ':'));
}

// Update GetModuleHandleExA to fix module handle
BOOL WINAPI GetModuleHandleExAHandler(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule)
{
	PFN_GetModuleHandleExA org_GetModuleHandleEx = (PFN_GetModuleHandleExA)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleHandleExA, nullptr, nullptr);

	if (org_GetModuleHandleEx)
	{
		BOOL ret = org_GetModuleHandleEx(dwFlags, lpModuleName, phModule);
		if ((dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) && !*phModule && moduleHandle)
		{
			*phModule = moduleHandle;
			ret = TRUE;
		}
		return ret;
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return FALSE;
}

// Update GetModuleHandleExW to fix module handle
BOOL WINAPI GetModuleHandleExWHandler(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule)
{
	PFN_GetModuleHandleExW org_GetModuleHandleEx = (PFN_GetModuleHandleExW)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleHandleExW, nullptr, nullptr);

	if (org_GetModuleHandleEx)
	{
		BOOL ret = org_GetModuleHandleEx(dwFlags, lpModuleName, phModule);
		if ((dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) && !*phModule && moduleHandle)
		{
			*phModule = moduleHandle;
			ret = TRUE;
		}
		return ret;
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return FALSE;
}

// Update GetModuleFileNameA to fix module name
DWORD WINAPI GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	PFN_GetModuleFileNameA org_GetModuleFileName = (PFN_GetModuleFileNameA)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleFileNameA, nullptr, nullptr);

	if (org_GetModuleFileName)
	{
		DWORD ret = org_GetModuleFileName(hModule, lpFilename, nSize);
		if ((NotValidFileNamePath(lpFilename, nSize) && nSize > 1) || (moduleHandle && hModule == moduleHandle))
		{
			ret = min(nPathSize, nSize) - 1;
			memcpy(lpFilename, std::string(wstrModulePath.begin(), wstrModulePath.end()).c_str(), ret * sizeof(char));
			lpFilename[ret] = '\0';
		}
		return ret;
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return NULL;
}

// Update GetModuleFileNameW to fix module name
DWORD WINAPI GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	PFN_GetModuleFileNameW org_GetModuleFileName = (PFN_GetModuleFileNameW)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleFileNameW, nullptr, nullptr);

	if (org_GetModuleFileName)
	{
		DWORD ret = org_GetModuleFileName(hModule, lpFilename, nSize);
		if ((NotValidFileNamePath(lpFilename, nSize) && nSize > 1) || (moduleHandle && hModule == moduleHandle))
		{
			ret = min(nPathSize, nSize) - 1;
			memcpy(lpFilename, wstrModulePath.c_str(), ret * sizeof(wchar_t));
			lpFilename[ret] = '\0';
		}
		return ret;
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return NULL;
}

// CreateFileW wrapper function
HANDLE WINAPI CreateFileWHandler(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	PFN_CreateFileW org_CreateFile = (PFN_CreateFileW)InterlockedCompareExchangePointer((PVOID*)&p_CreateFileW, nullptr, nullptr);

	if (org_CreateFile)
	{
		return org_CreateFile(GetFileName(std::wstring(lpFileName).c_str()), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(127);
	return INVALID_HANDLE_VALUE;
}

BOOL WINAPI FindNextFileAHandler(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	PFN_FindNextFileA org_FindNextFile = (PFN_FindNextFileA)InterlockedCompareExchangePointer((PVOID*)&p_FindNextFileA, nullptr, nullptr);

	if (org_FindNextFile)
	{
		BOOL ret = org_FindNextFile(hFindFile, lpFindFileData);
		if (UseCustomModFolder)
		{
			std::wstring ws = toLower(toWString(lpFindFileData->cFileName));
			if (ws.find(L"voice.afs") != std::string::npos)
			{
				if (VoiceSizeLow)
				{
					lpFindFileData->nFileSizeLow = VoiceSizeLow;
					lpFindFileData->nFileSizeHigh = 0;
				}
			}
			else if (ws.find(L".adx") != std::string::npos || ws.find(L".aix") != std::string::npos)
			{
				lpFindFileData->nFileSizeLow = 0;
				lpFindFileData->nFileSizeHigh = 0;
			}
		}
		return ret;
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(127);
	return FALSE;
}

// GetPrivateProfileStringA wrapper function
DWORD WINAPI GetPrivateProfileStringAHandler(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
	PFN_GetPrivateProfileStringA org_GetPrivateProfileString = (PFN_GetPrivateProfileStringA)InterlockedCompareExchangePointer((PVOID*)&p_GetPrivateProfileStringA, nullptr, nullptr);

	if (org_GetPrivateProfileString)
	{
		return org_GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, GetFileName(std::string(lpFileName).c_str()));
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(2);
	return NULL;
}

// GetPrivateProfileStringW wrapper function
DWORD WINAPI GetPrivateProfileStringWHandler(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
	PFN_GetPrivateProfileStringW org_GetPrivateProfileString = (PFN_GetPrivateProfileStringW)InterlockedCompareExchangePointer((PVOID*)&p_GetPrivateProfileStringW, nullptr, nullptr);

	if (org_GetPrivateProfileString)
	{
		return org_GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, GetFileName(std::wstring(lpFileName).c_str()));
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(2);
	return NULL;
}

void InstallFileSystemHooks(HMODULE hModule, wchar_t *ConfigPath)
{
	// Store handle
	moduleHandle = hModule;

	// Store config path
	wcscpy_s(ConfigPathW, MAX_PATH, ConfigPath);
	std::wstring ws(ConfigPath);
	strcpy_s(ConfigPathA, MAX_PATH, std::string(ws.begin(), ws.end()).c_str());

	// Store config name
	wcscpy_s(ConfigName, MAX_PATH, wcsrchr(ConfigPath, '\\'));

	// Get module path
	wchar_t Path[MAX_PATH];
	GetModuleFileName(hModule, Path, MAX_PATH);
	wstrModulePath.assign(Path);
	nPathSize = wstrModulePath.size() + 1; // Include a null terminator

	// Get data path
	GetModuleFileName(hModule, Path, MAX_PATH);
	wcscpy_s(wcsrchr(Path, '\\'), MAX_PATH - wcslen(Path), L"\0");
	modLoc = wcslen(Path) + 1;
	wcscat_s(Path, MAX_PATH, L"\\data\\");
	wstrDataPath.assign(Path);
	wstrDataPath.assign(toLower(wstrDataPath));

	// Get voice.afs size from mod path
	wcscpy_s(Path, MAX_PATH, ModPathW);
	wcscat_s(Path, MAX_PATH, L"\\sound\\adx\\voice\\voice.afs");
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;
	if (GetFileAttributesEx(Path, GetFileExInfoStandard, &FileInformation))
	{
		VoiceSizeLow = FileInformation.nFileSizeLow;
	}

	// Logging
	Log() << "Hooking the FileSystem APIs...";

	// Hook GetModuleFileName and GetModuleHandleEx to fix module name in modules loaded from memory
	InterlockedExchangePointer((PVOID*)&p_GetModuleHandleExA, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleHandleExA"), "GetModuleHandleExA", GetModuleHandleExAHandler));
	InterlockedExchangePointer((PVOID*)&p_GetModuleHandleExW, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleHandleExW"), "GetModuleHandleExW", GetModuleHandleExWHandler));
	InterlockedExchangePointer((PVOID*)&p_GetModuleFileNameA, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleFileNameA"), "GetModuleFileNameA", GetModuleFileNameAHandler));
	InterlockedExchangePointer((PVOID*)&p_GetModuleFileNameW, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleFileNameW"), "GetModuleFileNameW", GetModuleFileNameWHandler));

	// Hook FileSystem APIs
	InterlockedExchangePointer((PVOID*)&p_CreateFileW, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW"), "CreateFileW", *CreateFileWHandler));
	InterlockedExchangePointer((PVOID*)&p_FindNextFileA, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "FindNextFileA"), "FindNextFileA", *FindNextFileAHandler));
	InterlockedExchangePointer((PVOID*)&p_GetPrivateProfileStringA, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetPrivateProfileStringA"), "GetPrivateProfileStringA", *GetPrivateProfileStringAHandler));
	InterlockedExchangePointer((PVOID*)&p_GetPrivateProfileStringW, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetPrivateProfileStringW"), "GetPrivateProfileStringW", *GetPrivateProfileStringWHandler));
}
