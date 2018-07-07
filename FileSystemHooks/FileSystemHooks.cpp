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
#include <algorithm>
#include "Hooking\Hook.h"
#include "Common\Logging.h"
#include "FileSystemHooks.h"

// Varables used in hooked modules
bool LoadingMemoryModule = false;
HMODULE moduleHandle = nullptr;
char ConfigPathA[MAX_PATH];
wchar_t ConfigPathW[MAX_PATH];
std::wstring wstrModulePath;
DWORD nPathSize = 0;

// List of hardcoded config file names from memory modules
wchar_t *ConfigFileList[] =
{
	L"sh2fog.ini"
};

template<typename T>
bool CheckConfigPath(T& str)
{
	if (LoadingMemoryModule)
	{
		for (wchar_t *it : ConfigFileList)
		{
			if (wcsstr(std::wstring(str.begin(), str.end()).c_str(), it))
			{
				return true;
			}
		}
	}
	return false;
}

LPCSTR GetFileName(LPCSTR lpFileName)
{
	return (CheckConfigPath(std::string(lpFileName))) ? ConfigPathA : lpFileName;
}

LPCWSTR GetFileName(LPCWSTR lpFileName)
{
	return (CheckConfigPath(std::wstring(lpFileName))) ? ConfigPathW : lpFileName;
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

// API typedef
typedef BOOL(WINAPI *PFN_GetModuleHandleExA)(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule);
typedef BOOL(WINAPI *PFN_GetModuleHandleExW)(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule);
typedef DWORD(WINAPI *PFN_GetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef DWORD(WINAPI *PFN_GetModuleFileNameW)(HMODULE, LPWSTR, DWORD);
typedef HANDLE(WINAPI *PFN_CreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef HANDLE(WINAPI *PFN_CreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef DWORD(WINAPI *PFN_GetPrivateProfileStringA)(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName);
typedef DWORD(WINAPI *PFN_GetPrivateProfileStringW)(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName);

// Proc addresses
FARPROC p_GetModuleHandleExA = nullptr;
FARPROC p_GetModuleHandleExW = nullptr;
FARPROC p_GetModuleFileNameA = nullptr;
FARPROC p_GetModuleFileNameW = nullptr;
FARPROC p_CreateFileA = nullptr;
FARPROC p_CreateFileW = nullptr;
FARPROC p_GetPrivateProfileStringA = nullptr;
FARPROC p_GetPrivateProfileStringW = nullptr;

// Update GetModuleHandleExA to fix module handle
BOOL WINAPI GetModuleHandleExAHandler(DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule)
{
	PFN_GetModuleHandleExA org_GetModuleHandleEx = (PFN_GetModuleHandleExA)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleHandleExA, nullptr, nullptr);

	if (org_GetModuleHandleEx)
	{
		BOOL ret = org_GetModuleHandleEx(dwFlags, lpModuleName, phModule);
		if (LoadingMemoryModule && (dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) && !*phModule && moduleHandle)
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
		if (LoadingMemoryModule && (dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) && !*phModule && moduleHandle)
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
		if (NotValidFileNamePath(lpFilename, nSize) && nSize > 1)
		{
			ret = min(nPathSize, nSize);
			lpFilename[ret] = (char)'/0';
			ret--;	// for null terminator
			memcpy(lpFilename, std::string(wstrModulePath.begin(), wstrModulePath.end()).c_str(), ret * sizeof(char));
		}
		return ret;
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return 0;
}

// Update GetModuleFileNameW to fix module name
DWORD WINAPI GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	PFN_GetModuleFileNameW org_GetModuleFileName = (PFN_GetModuleFileNameW)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleFileNameW, nullptr, nullptr);

	if (org_GetModuleFileName)
	{
		DWORD ret = org_GetModuleFileName(hModule, lpFilename, nSize);
		if (NotValidFileNamePath(lpFilename, nSize))
		{
			ret = min(nPathSize, nSize);
			lpFilename[ret] = '/0';
			ret--;	// for null terminator
			memcpy(lpFilename, wstrModulePath.c_str(), ret * sizeof(wchar_t));
		}
		return ret;
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return 0;
}

// CreateFileA wrapper function
HANDLE WINAPI CreateFileAHandler(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	PFN_CreateFileA org_CreateFile = (PFN_CreateFileA)InterlockedCompareExchangePointer((PVOID*)&p_CreateFileA, nullptr, nullptr);

	if (org_CreateFile)
	{
		return org_CreateFile(GetFileName(lpFileName), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(127);
	return nullptr;
}

// CreateFileW wrapper function
HANDLE WINAPI CreateFileWHandler(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	PFN_CreateFileW org_CreateFile = (PFN_CreateFileW)InterlockedCompareExchangePointer((PVOID*)&p_CreateFileW, nullptr, nullptr);

	if (org_CreateFile)
	{
		return org_CreateFile(GetFileName(lpFileName), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(127);
	return nullptr;
}

// GetPrivateProfileStringA wrapper function
DWORD WINAPI GetPrivateProfileStringAHandler(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
	PFN_GetPrivateProfileStringA org_GetPrivateProfileString = (PFN_GetPrivateProfileStringA)InterlockedCompareExchangePointer((PVOID*)&p_GetPrivateProfileStringA, nullptr, nullptr);

	if (org_GetPrivateProfileString)
	{
		return org_GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, GetFileName(lpFileName));
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(2);
	return 0;
}

// GetPrivateProfileStringW wrapper function
DWORD WINAPI GetPrivateProfileStringWHandler(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
	PFN_GetPrivateProfileStringW org_GetPrivateProfileString = (PFN_GetPrivateProfileStringW)InterlockedCompareExchangePointer((PVOID*)&p_GetPrivateProfileStringW, nullptr, nullptr);

	if (org_GetPrivateProfileString)
	{
		return org_GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, GetFileName(lpFileName));
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(2);
	return 0;
}

void InstallFileSystemHooks(HMODULE hModule, wchar_t *ConfigPath)
{
	// Store handle
	moduleHandle = hModule;

	// Store config path
	wcscpy_s(ConfigPathW, MAX_PATH, ConfigPath);
	std::wstring ws(ConfigPath);
	strcpy_s(ConfigPathA, MAX_PATH, std::string(ws.begin(), ws.end()).c_str());

	// Get module path
	wchar_t Path[MAX_PATH];
	GetModuleFileName(hModule, Path, MAX_PATH);
	wstrModulePath.assign(Path);
	nPathSize = wstrModulePath.size() + 1; // Include a null terminator

	// Logging
	Log() << "Hooking the FileSystem APIs...";

	// Hook GetModuleFileName and GetModuleHandleEx to fix module name in modules loaded from memory
	InterlockedExchangePointer((PVOID*)&p_GetModuleHandleExA, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleHandleExA"), "GetModuleHandleExA", GetModuleHandleExAHandler));
	InterlockedExchangePointer((PVOID*)&p_GetModuleHandleExW, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleHandleExW"), "GetModuleHandleExW", GetModuleHandleExWHandler));
	InterlockedExchangePointer((PVOID*)&p_GetModuleFileNameA, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleFileNameA"), "GetModuleFileNameA", GetModuleFileNameAHandler));
	InterlockedExchangePointer((PVOID*)&p_GetModuleFileNameW, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleFileNameW"), "GetModuleFileNameW", GetModuleFileNameWHandler));

	// Hook FileSystem APIs
	InterlockedExchangePointer((PVOID*)&p_CreateFileA, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileA"), "CreateFileA", *CreateFileAHandler));
	InterlockedExchangePointer((PVOID*)&p_CreateFileW, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW"), "CreateFileW", *CreateFileWHandler));
	InterlockedExchangePointer((PVOID*)&p_GetPrivateProfileStringA, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetPrivateProfileStringA"), "GetPrivateProfileStringA", *GetPrivateProfileStringAHandler));
	InterlockedExchangePointer((PVOID*)&p_GetPrivateProfileStringW, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetPrivateProfileStringW"), "GetPrivateProfileStringW", *GetPrivateProfileStringWHandler));
}
