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
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include "FileSystemHooks.h"
#include "External\Hooking\Hook.h"
#include "Settings.h"
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
char ModPathA[MAX_PATH];
wchar_t ModPathW[MAX_PATH];
DWORD modLoc = 0;
DWORD modLen = 0;
DWORD MaxModFileLen = 0;
std::string strModulePathA;
std::wstring strModulePathW;
DWORD nPathSize = 0;
char ConfigNameA[MAX_PATH];
wchar_t ConfigNameW[MAX_PATH];
bool FileEnabled = true;

#define DEFINE_BGM_FILES(name, unused, unused2) \
	DWORD name ## SizeLow = 0;

VISIT_BGM_FILES(DEFINE_BGM_FILES);

template<typename T>
bool isInString(T strCheck, T str, size_t size);

inline LPCSTR ModPath(LPCSTR)
{
	return ModPathA;
}

inline LPCWSTR ModPath(LPCWSTR)
{
	return ModPathW;
}

inline LPCSTR ConfigPath(LPCSTR)
{
	return ConfigPathA;
}

inline LPCWSTR ConfigPath(LPCWSTR)
{
	return ConfigPathW;
}

inline void strcpy_s(wchar_t *dest, size_t size, LPCWSTR src)
{
	wcscpy_s(dest, size, src);
}

inline void strcat_s(wchar_t *dest, size_t size, LPCWSTR src)
{
	wcscat_s(dest, size, src);
}

inline bool PathExists(LPCSTR str)
{
	return PathFileExistsA(str);
}

inline bool PathExists(LPCWSTR str)
{
	return PathFileExistsW(str);
}

inline bool isInString(LPCSTR strCheck, LPCSTR strA, LPCWSTR, size_t size)
{
	return isInString(strCheck, strA, size);
}

inline bool isInString(LPCWSTR strCheck, LPCSTR, LPCWSTR strW, size_t size)
{
	return isInString(strCheck, strW, size);
}

inline int mytolower(const char chr)
{
	return tolower((unsigned char)chr);
}

inline wint_t mytolower(const wchar_t chr)
{
	return towlower(chr);
}

template<typename T>
bool isInString(T strCheck, T str, size_t size)
{
	if (!strCheck || !str)
	{
		return false;
	}

	T p1 = strCheck;
	T p2 = str;
	T r = *p2 == 0 ? strCheck : 0;

	while (*p1 != 0 && *p2 != 0 && (size_t)(p1 - strCheck) < size)
	{		
		if (mytolower(*p1) == mytolower(*p2))
		{
			if (r == 0)
			{
				r = p1;
			}

			p2++;
		}
		else
		{
			p2 = str;
			if (r != 0)
			{
				p1 = r + 1;
			}

			if (mytolower(*p1) == mytolower(*p2))
			{
				r = p1;
				p2++;
			}
			else
			{
				r = 0;
			}
		}

		p1++;
	}

	return (*p2 == 0) ? true : false;
}

template<typename T>
inline bool isDataPath(T sh2)
{
	if ((sh2[0] == 'd' || sh2[0] == 'D') &&
		(sh2[1] == 'a' || sh2[1] == 'A') &&
		(sh2[2] == 't' || sh2[2] == 'T') &&
		(sh2[3] == 'a' || sh2[3] == 'A') &&
		sh2[4] == '\\')
	{
		return true;
	}
	return false;
}

template<typename T>
bool CheckConfigPath(T str)
{
	for (MODULECONFIG it : ConfigList)
	{
		if (*(it.Enabled) && isInString(str, it.ConfigFileListA, it.ConfigFileListW, MAX_PATH))
		{
			return true;
		}
	}
	return false;
}

template<typename T, typename D>
T UpdateModPath(T sh2, D str)
{
	if (!sh2 || !str)
	{
		return sh2;
	}

	// Check if this is a config file
	if (CheckConfigPath(sh2))
	{
		return ConfigPath(sh2);
	}

	T sh2_data = sh2 + modLoc;

	// Check relative path
	if (isDataPath(sh2))
	{
		strcpy_s(str, MAX_PATH, ModPath(sh2));
		strcpy_s(str + modLen, MAX_PATH - modLen, sh2 + 4);
		if (PathExists(str))
		{
			return str;
		}

		return sh2;
	}

	// Check full path
	if (isDataPath(sh2_data))
	{
		strcpy_s(str, MAX_PATH, sh2);
		strcpy_s(str + modLoc, MAX_PATH - modLoc, ModPath(sh2));
		strcpy_s(str + modLoc + modLen, MAX_PATH - modLoc - modLen, sh2_data + 4);
		if (PathExists(str))
		{
			return str;
		}

		return sh2;
	}

	return sh2;
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
DWORD WINAPI GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFileName, DWORD nSize)
{
	PFN_GetModuleFileNameA org_GetModuleFileName = (PFN_GetModuleFileNameA)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleFileNameA, nullptr, nullptr);

	if (org_GetModuleFileName)
	{
		DWORD ret = org_GetModuleFileName(hModule, lpFileName, nSize);
		if (lpFileName && nSize && ((moduleHandle && hModule == moduleHandle) || !PathExists(lpFileName)))
		{
			ret = min(nPathSize, nSize) - 1;
			memcpy(lpFileName, strModulePathA.c_str(), ret * sizeof(char));
			lpFileName[ret] = '\0';
		}
		return ret;
	}

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return NULL;
}

// Update GetModuleFileNameW to fix module name
DWORD WINAPI GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFileName, DWORD nSize)
{
	PFN_GetModuleFileNameW org_GetModuleFileName = (PFN_GetModuleFileNameW)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleFileNameW, nullptr, nullptr);

	if (org_GetModuleFileName)
	{
		DWORD ret = org_GetModuleFileName(hModule, lpFileName, nSize);
		if (lpFileName && nSize && ((moduleHandle && hModule == moduleHandle) || !PathExists(lpFileName)))
		{
			ret = min(nPathSize, nSize) - 1;
			memcpy(lpFileName, strModulePathW.c_str(), ret * sizeof(wchar_t));
			lpFileName[ret] = '\0';
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
		return org_CreateFile(UpdateModPath(lpFileName, Filename), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(127);
	return INVALID_HANDLE_VALUE;
}

// FindNextFileA wrapper function
BOOL WINAPI FindNextFileAHandler(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	PFN_FindNextFileA org_FindNextFile = (PFN_FindNextFileA)InterlockedCompareExchangePointer((PVOID*)&p_FindNextFileA, nullptr, nullptr);

	if (org_FindNextFile)
	{
		BOOL ret = org_FindNextFile(hFindFile, lpFindFileData);
		if (UseCustomModFolder && lpFindFileData)
		{

#define CHECK_BGM_FILES(name, ext, unused) \
			if (isInString(lpFindFileData->cFileName, ## #name ## "." ## # ext, L## #name ## "." ## # ext, MaxModFileLen)) \
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
		return org_GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, UpdateModPath(lpFileName, Filename));
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
		return org_GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, UpdateModPath(lpFileName, Filename));
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
	if (!PathExists(ConfigPathA) || !PathExists(ConfigPathW))
	{
		Logging::Log() << __FUNCTION__ " Error: 'ConfigPath' incorrect! " << ConfigPathA;
		return;
	}

	// Store config name
	strcpy_s(ConfigNameA, MAX_PATH, strrchr(ConfigPathA, '\\'));
	wcscpy_s(ConfigNameW, MAX_PATH, wcsrchr(ConfigPathW, '\\'));

	// Set module name
	if (CustomModFolder.size())
	{
		strcpy_s(ModPathA, MAX_PATH, CustomModFolder.c_str());
		wcscpy_s(ModPathW, MAX_PATH, std::wstring(CustomModFolder.begin(), CustomModFolder.end()).c_str());
		Logging::Log() << "Using mod path: " << ModPathW;
	}
	else
	{
		strcpy_s(ModPathA, MAX_PATH, "sh2e");
		wcscpy_s(ModPathW, MAX_PATH, L"sh2e");
	}

	// Get module path
	wchar_t tmpPath[MAX_PATH];
	GetModuleFileName(hModule, tmpPath, MAX_PATH);
	strModulePathW.assign(tmpPath);
	strModulePathA.assign(strModulePathW.begin(), strModulePathW.end());
	nPathSize = strModulePathA.size() + 1; // Include a null terminator
	if (!PathExists(strModulePathA.c_str()) || !PathExists(strModulePathW.c_str()))
	{
		Logging::Log() << __FUNCTION__ " Error: 'strModulePath' incorrect! " << strModulePathA.c_str();
		return;
	}

	// Get data path
	GetModuleFileName(nullptr, tmpPath, MAX_PATH);
	wcscpy_s(wcsrchr(tmpPath, '\\'), MAX_PATH - wcslen(tmpPath), L"\0");
	modLoc = wcslen(tmpPath) + 1;
	modLen = strlen(ModPathA);
	if (modLoc + modLen > MAX_PATH)
	{
		Logging::Log() << __FUNCTION__ " Error: custom mod path length is too long! " << modLoc + modLen;
		return;
	}

	// Get size of files from mod path
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;

#define GET_BGM_FILES(name, ext, path) \
	if (MaxModFileLen < strlen(#name ## "." ## #ext) + 1) \
	{ \
		MaxModFileLen = strlen(#name ## "." ## #ext) + 1; \
	} \
	if (GetFileAttributesEx(std::wstring(std::wstring(ModPathW) + path ## "\\" ## #name ## "." ## #ext).c_str(), GetFileExInfoStandard, &FileInformation)) \
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
