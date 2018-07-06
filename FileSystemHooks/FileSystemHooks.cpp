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

char ConfigPathA[MAX_PATH];
wchar_t ConfigPathW[MAX_PATH];
char ModulePathA[MAX_PATH];
wchar_t ModulePathW[MAX_PATH];
wchar_t ProcNameW[MAX_PATH];
DWORD nPathSize = 0;

wchar_t *ConfigFileList[] =
{
	L"sh2fog.ini",
	ProcNameW
};

LPCSTR GetFileName(LPCSTR lpFileName)
{
	for (wchar_t *it : ConfigFileList)
	{
		std::wstring ws(it);
		if (strstr(lpFileName, std::string(ws.begin(), ws.end()).c_str()))
		{
			return ConfigPathA;
		}
	}
	return lpFileName;
}

LPCWSTR GetFileName(LPCWSTR lpFileName)
{
	for (wchar_t *it : ConfigFileList)
	{
		if (wcsstr(lpFileName, it))
		{
			return ConfigPathW;
		}
	}
	return lpFileName;
}

// API typedef
typedef DWORD(WINAPI *PFN_GetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef DWORD(WINAPI *PFN_GetModuleFileNameW)(HMODULE, LPWSTR, DWORD);
typedef HANDLE(WINAPI *PFN_CreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef HANDLE(WINAPI *PFN_CreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef DWORD(WINAPI *PFN_GetPrivateProfileStringA)(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName);
typedef DWORD(WINAPI *PFN_GetPrivateProfileStringW)(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName);

// Proc addresses
FARPROC p_GetModuleFileNameA = nullptr;
FARPROC p_GetModuleFileNameW = nullptr;
FARPROC p_CreateFileA = nullptr;
FARPROC p_CreateFileW = nullptr;
FARPROC p_GetPrivateProfileStringA = nullptr;
FARPROC p_GetPrivateProfileStringW = nullptr;

// Update GetModuleFileNameA to fix module name
DWORD WINAPI GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	PFN_GetModuleFileNameA org_GetModuleFileName = (PFN_GetModuleFileNameA)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleFileNameA, nullptr, nullptr);

	if (org_GetModuleFileName)
	{
		DWORD ret = org_GetModuleFileName(hModule, lpFilename, nSize);

		if (lpFilename[0] != '\\' && lpFilename[1] != '\\' && lpFilename[2] != '\\' && lpFilename[3] != '\\' &&
			lpFilename[0] != ':' && lpFilename[1] != ':' && lpFilename[2] != ':' && lpFilename[3] != ':')
		{
			ZeroMemory(lpFilename, nSize * sizeof(char));
			memcpy(lpFilename, ModulePathA, (nSize - 1) * sizeof(char));
			ret = min(nPathSize, nSize);
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

		if (lpFilename[0] != '\\' && lpFilename[1] != '\\' && lpFilename[2] != '\\' && lpFilename[3] != '\\' &&
			lpFilename[0] != ':' && lpFilename[1] != ':' && lpFilename[2] != ':' && lpFilename[3] != ':')
		{
			ZeroMemory(lpFilename, nSize * sizeof(wchar_t));
			memcpy(lpFilename, ModulePathW, (nSize - 1) * sizeof(wchar_t));
			ret = (nPathSize, nSize);
		}

		return ret;
	}

	Log() << __FUNCTION__ << " Error: invalid proc address!";
	SetLastError(5);
	return 0;
}

// CreateFileA wrapper function
HANDLE WINAPI CreateFileAHooked(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
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
HANDLE WINAPI CreateFileWHooked(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
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
DWORD WINAPI GetPrivateProfileStringAHooked(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
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
DWORD WINAPI GetPrivateProfileStringWHooked(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
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
	// Get module path
	GetModuleFileName(hModule, ModulePathW, MAX_PATH);
	std::wstring ws(ModulePathW);
	strcpy_s(ModulePathA, MAX_PATH, std::string(ws.begin(), ws.end()).c_str());
	nPathSize = strlen(ModulePathA);

	// Get proc name as ini
	wchar_t PathW[MAX_PATH];
	GetModuleFileName(nullptr, PathW, MAX_PATH);
	wcscpy_s(wcsrchr(PathW, '.'), MAX_PATH, L".ini");
	wcscpy_s(ProcNameW, MAX_PATH, wcsrchr(PathW, '\\') + 1);

	// Store config path
	wcscpy_s(ConfigPathW, MAX_PATH , ConfigPath);
	ws.assign(ConfigPath);
	strcpy_s(ConfigPathA, MAX_PATH, std::string(ws.begin(), ws.end()).c_str());

	// Logging
	Log() << "Hooking the FileSystem APIs...";

	// Hook GetModuleFileName to fix module name in modules loaded from memory
	InterlockedExchangePointer((PVOID*)&p_GetModuleFileNameA, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleFileNameA"), "GetModuleFileNameA", GetModuleFileNameAHandler));
	InterlockedExchangePointer((PVOID*)&p_GetModuleFileNameW, Hook::HookAPI(GetModuleHandle(L"kernel32"), "kernel32.dll", Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetModuleFileNameW"), "GetModuleFileNameW", GetModuleFileNameWHandler));

	// Hook FileSystem APIs
	InterlockedExchangePointer((PVOID*)&p_CreateFileA, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileA"), "CreateFileA", *CreateFileAHooked));
	InterlockedExchangePointer((PVOID*)&p_CreateFileW, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW"), "CreateFileW", *CreateFileWHooked));
	InterlockedExchangePointer((PVOID*)&p_GetPrivateProfileStringA, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetPrivateProfileStringA"), "GetPrivateProfileStringA", *GetPrivateProfileStringAHooked));
	InterlockedExchangePointer((PVOID*)&p_GetPrivateProfileStringW, Hook::HotPatch(Hook::GetProcAddress(GetModuleHandle(L"kernel32"), "GetPrivateProfileStringW"), "GetPrivateProfileStringW", *GetPrivateProfileStringWHooked));
}
