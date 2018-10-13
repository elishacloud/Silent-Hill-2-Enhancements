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
#include "FileSystemHooks.h"
#include "External\Hooking\Hook.h"
#include "MyStrings.h"
#include "Logging\Logging.h"

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
char ModPathA[5] = "sh2e";
wchar_t ModPathW[5] = L"sh2e";
std::wstring strDataPathW;
DWORD modLoc = 0;
std::string strModulePathA;
std::wstring strModulePathW;
DWORD nPathSize = 0;
char ConfigNameA[MAX_PATH];
wchar_t ConfigNameW[MAX_PATH];
bool FileEnabled = true;

#define DEFINE_BGM_FILES(name, unused, unused2) \
	DWORD name ## SizeLow = 0;

VISIT_BGM_FILES(DEFINE_BGM_FILES);

bool PathExists(LPCSTR str)
{
	return PathFileExistsA(str);
}

bool PathExists(LPCWSTR str)
{
	return PathFileExistsW(str);
}

template<typename T>
bool CheckConfigPath(T str)
{
	for (MODULECONFIG it : ConfigList)
	{
		if (*(it.Enabled) && IsInString(str, it.ConfigFileListA, it.ConfigFileListW))
		{
			return true;
		}
	}
	return false;
}

template<typename T>
T *ReplaceWithModPath(T *lpFileName, DWORD start)
{
	size_t Size = length(lpFileName);
	if (Size > start + 3 &&
		(lpFileName[start] == 'd' || lpFileName[start] == 'D') &&
		(lpFileName[start + 1] == 'a' || lpFileName[start + 1] == 'A') &&
		(lpFileName[start + 2] == 't' || lpFileName[start + 2] == 'T') &&
		(lpFileName[start + 3] == 'a' || lpFileName[start + 3] == 'A'))
	{
		T tmpPath[MAX_PATH];
		CopyString(tmpPath, MAX_PATH, lpFileName);
		for (int x = 0; x < 4; x++)
		{
			tmpPath[start + x] = GetStringType(lpFileName, ModPathA, ModPathW)[x];
		}

		if (tmpPath[Size - 1] == '*')
		{
			tmpPath[Size - 1] = '\0';
		}

		if (PathExists(tmpPath))
		{
			for (int x = 0; x < 4; x++)
			{
				lpFileName[start + x] = GetStringType(lpFileName, ModPathA, ModPathW)[x];
			}
		}
	}
	return lpFileName;
}

template<typename T>
T CheckForModPath(T lpFileName)
{
	// Verify mod location and data path
	if (!UseCustomModFolder ||															// Verify UseCustomModFolder is enabled
		!modLoc || modLoc + 3 > strDataPathW.size() ||									// Verify modLoc is correct and string is large enough
		IsInString(lpFileName, "data\\save", L"data\\save") ||							// Ignore files in the 'data\save' folder
		(IsInString(lpFileName, "sddata.bin", L"sddata.bin") && !EnableSFXAddrHack))	// Ignore 'sddata.bin' if EnableSFXAddrHack is disabled
	{
		return lpFileName;
	}

	// Check if string is in the data folder
	return ReplaceWithModPath(ReplaceWithModPath(lpFileName, 0), modLoc);
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

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
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

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
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
			memcpy(lpFilename, strModulePathA.c_str(), ret * sizeof(char));
			lpFilename[ret] = '\0';
		}
		return ret;
	}

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
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
			memcpy(lpFilename, strModulePathW.c_str(), ret * sizeof(wchar_t));
			lpFilename[ret] = '\0';
		}
		return ret;
	}

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return NULL;
}

// CreateFileW wrapper function
HANDLE WINAPI CreateFileWHandler(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	PFN_CreateFileW org_CreateFile = (PFN_CreateFileW)InterlockedCompareExchangePointer((PVOID*)&p_CreateFileW, nullptr, nullptr);

	if (org_CreateFile)
	{
		wchar_t Filename[MAX_PATH];
		wcscpy_s(Filename, MAX_PATH, lpFileName);
		return org_CreateFile(GetFileName(Filename), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
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

#define CHECK_BGM_FILES(name, ext, unused) \
			if (IsInString(lpFindFileData->cFileName, ## #name ## "." ## # ext, L## #name ## "." ## # ext)) \
			{ \
				if (name ## SizeLow) \
				{ \
					lpFindFileData->nFileSizeLow = name ## SizeLow; \
					lpFindFileData->nFileSizeHigh = 0; \
				} \
			}

			VISIT_BGM_FILES(CHECK_BGM_FILES);
		}
		return ret;
	}

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(127);
	return FALSE;
}

// GetPrivateProfileStringA wrapper function
DWORD WINAPI GetPrivateProfileStringAHandler(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
	PFN_GetPrivateProfileStringA org_GetPrivateProfileString = (PFN_GetPrivateProfileStringA)InterlockedCompareExchangePointer((PVOID*)&p_GetPrivateProfileStringA, nullptr, nullptr);

	if (org_GetPrivateProfileString)
	{
		char Filename[MAX_PATH];
		strcpy_s(Filename, MAX_PATH, lpFileName);
		return org_GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, GetFileName(Filename));
	}

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(2);
	return NULL;
}

// GetPrivateProfileStringW wrapper function
DWORD WINAPI GetPrivateProfileStringWHandler(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
	PFN_GetPrivateProfileStringW org_GetPrivateProfileString = (PFN_GetPrivateProfileStringW)InterlockedCompareExchangePointer((PVOID*)&p_GetPrivateProfileStringW, nullptr, nullptr);

	if (org_GetPrivateProfileString)
	{
		wchar_t Filename[MAX_PATH];
		wcscpy_s(Filename, MAX_PATH, lpFileName);
		return org_GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, GetFileName(Filename));
	}

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(2);
	return NULL;
}

void InstallFileSystemHooks(HMODULE hModule, wchar_t *ConfigPath)
{
	// Store handle
	moduleHandle = hModule;

	// Store config path
	std::wstring ws(ConfigPath);
	strcpy_s(ConfigPathA, MAX_PATH, std::string(ws.begin(), ws.end()).c_str());
	wcscpy_s(ConfigPathW, MAX_PATH, ConfigPath);

	// Store config name
	strcpy_s(ConfigNameA, MAX_PATH, strrchr(ConfigPathA, '\\'));
	wcscpy_s(ConfigNameW, MAX_PATH, wcsrchr(ConfigPathW, '\\'));

	// Get module path
	wchar_t tmpPath[MAX_PATH];
	GetModuleFileName(hModule, tmpPath, MAX_PATH);
	strModulePathW.assign(tmpPath);
	strModulePathA.assign(strModulePathW.begin(), strModulePathW.end());
	nPathSize = strModulePathA.size() + 1; // Include a null terminator

	// Get data path
	GetModuleFileName(nullptr, tmpPath, MAX_PATH);
	wcscpy_s(wcsrchr(tmpPath, '\\'), MAX_PATH - wcslen(tmpPath), L"\0");
	modLoc = wcslen(tmpPath) + 1;
	wcscat_s(tmpPath, MAX_PATH, L"\\data\\");
	strDataPathW.assign(tmpPath);

	// Get size of files from mod path
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;

#define GET_BGM_FILES(name, ext, path) \
	if (GetFileAttributesEx(std::wstring(std::wstring(ModPathW) + path ## "\\" ## #name ## "." ## # ext).c_str(), GetFileExInfoStandard, &FileInformation)) \
	{ \
		name ## SizeLow = FileInformation.nFileSizeLow; \
	}

	VISIT_BGM_FILES(GET_BGM_FILES);

	// Logging
	Logging::Log() << "Hooking the FileSystem APIs...";

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
