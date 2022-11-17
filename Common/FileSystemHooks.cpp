/**
* Copyright (C) 2022 Elisha Riedlinger
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
#include "Common\Utils.h"
#include "Patches\Patches.h"
#include "FileSystemHooks.h"
#include "External\Hooking\Hook.h"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Settings.h"
#include "Logging\Logging.h"
#include "Unicode.h"

// API typedef
typedef int(WINAPI* PFN_BinkOpen)(char* name, DWORD flags);
typedef FILE(WINAPIV *PFN_fopen)(char const* lpFileName, char const* lpMode);
typedef HANDLE(WINAPI *PFN_CreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef HANDLE(WINAPI *PFN_CreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef HANDLE(WINAPI *PFN_FindFirstFileA)(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
typedef BOOL(WINAPI *PFN_FindNextFileA)(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
typedef BOOL(WINAPI *PFN_CreateProcessA)(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
typedef BOOL(WINAPI *PFN_CreateProcessW)(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

// Proc addresses
FARPROC p_BinkOpen = nullptr;
FARPROC p_fopen = nullptr;
FARPROC p_CreateFileA = nullptr;
FARPROC p_CreateFileW = nullptr;
FARPROC p_FindFirstFileA = nullptr;
FARPROC p_FindNextFileA = nullptr;
FARPROC p_CreateProcessA = nullptr;
FARPROC p_CreateProcessW = nullptr;

// Variable used in hooked modules
bool IsFileSystemHooking = false;
char ModPathA[MAX_PATH];
wchar_t ModPathW[MAX_PATH];
char *ModPicPathA = "ps2";
wchar_t *ModPicPathW = L"ps2";
DWORD modLoc = 0;
DWORD picLen = 0;
DWORD MaxModFileLen = 0;

#define DEFINE_BGM_FILES(name, unused, unused2) \
	DWORD name ## SizeLow = 0;

VISIT_BGM_FILES(DEFINE_BGM_FILES);

LPCSTR GetModPath(LPCSTR) { return ModPathA; }
LPCWSTR GetModPath(LPCWSTR) { return ModPathW; }

inline LPCSTR ModPath(LPCSTR) { return ModPathA; }
inline LPCWSTR ModPath(LPCWSTR) { return ModPathW; }
inline LPCSTR ModPicPath(LPCSTR) { return ModPicPathA; }
inline LPCWSTR ModPicPath(LPCWSTR) { return ModPicPathW; }
inline LPCSTR LangPath(LPCSTR) { return "lang"; }
inline LPCWSTR LangPath(LPCWSTR) { return L"lang"; }
inline LPCSTR GetEnding1(LPCSTR) { return "\\movie\\end.bik"; }
inline LPCWSTR GetEnding1(LPCWSTR) { return L"\\movie\\end.bik"; }
inline LPCSTR GetEnding2(LPCSTR) { return "\\movie\\ending.bik"; }
inline LPCWSTR GetEnding2(LPCWSTR) { return L"\\movie\\ending.bik"; }

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
		if (tolower(*p1) == tolower(*p2))
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

			if (tolower(*p1) == tolower(*p2))
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

inline bool isInString(LPCSTR strCheck, LPCSTR strA, LPCWSTR, size_t size)
{
	return isInString(strCheck, strA, size);
}

inline bool isInString(LPCWSTR strCheck, LPCSTR, LPCWSTR strW, size_t size)
{
	return isInString(strCheck, strW, size);
}

template<typename T>
inline bool isDataPath(T sh2)
{
	if (sh2[0] != '\0' && (sh2[0] == 'd' || sh2[0] == 'D') &&
		sh2[1] != '\0' && (sh2[1] == 'a' || sh2[1] == 'A') &&
		sh2[2] != '\0' && (sh2[2] == 't' || sh2[2] == 'T') &&
		sh2[3] != '\0' && (sh2[3] == 'a' || sh2[3] == 'A'))
	{
		return true;
	}
	return false;
}

template<typename T>
inline bool isEndVideoPath(T sh2)
{
	if (sh2[0] != '\0' && (sh2[0] == 'm' || sh2[0] == 'M') &&
		sh2[1] != '\0' && (sh2[1] == 'o' || sh2[1] == 'O') &&
		sh2[2] != '\0' && (sh2[2] == 'v' || sh2[2] == 'V') &&
		sh2[3] != '\0' && (sh2[3] == 'i' || sh2[3] == 'I') &&
		sh2[4] != '\0' && (sh2[4] == 'e' || sh2[4] == 'E') &&
		sh2[5] != '\0' &&
		sh2[6] != '\0' && (sh2[6] == 'e' || sh2[6] == 'E') &&
		sh2[7] != '\0' && (sh2[7] == 'n' || sh2[7] == 'N') &&
		sh2[8] != '\0' && (sh2[8] == 'd' || sh2[8] == 'D') &&
		sh2[9] != '\0' && (sh2[9] == 'i' || sh2[9] == 'I') &&
		sh2[10] != '\0' && (sh2[10] == 'n' || sh2[10] == 'N') &&
		sh2[11] != '\0' && (sh2[11] == 'g' || sh2[11] == 'G') &&
		sh2[12] != '\0' &&
		sh2[13] != '\0' && (sh2[13] == 'b' || sh2[13] == 'B') &&
		sh2[14] != '\0' && (sh2[14] == 'i' || sh2[14] == 'I') &&
		sh2[15] != '\0' && (sh2[15] == 'k' || sh2[15] == 'K'))
	{
		return true;
	}
	if (sh2[0] != '\0' && (sh2[0] == 'm' || sh2[0] == 'M') &&
		sh2[1] != '\0' && (sh2[1] == 'o' || sh2[1] == 'O') &&
		sh2[2] != '\0' && (sh2[2] == 'v' || sh2[2] == 'V') &&
		sh2[3] != '\0' && (sh2[3] == 'i' || sh2[3] == 'I') &&
		sh2[4] != '\0' && (sh2[4] == 'e' || sh2[4] == 'E') &&
		sh2[5] != '\0' &&
		sh2[6] != '\0' && (sh2[6] == 'e' || sh2[6] == 'E') &&
		sh2[7] != '\0' && (sh2[7] == 'n' || sh2[7] == 'N') &&
		sh2[8] != '\0' && (sh2[8] == 'd' || sh2[8] == 'D') &&
		sh2[9] != '\0' &&
		sh2[10] != '\0' && (sh2[10] == 'b' || sh2[10] == 'B') &&
		sh2[11] != '\0' && (sh2[11] == 'i' || sh2[11] == 'I') &&
		sh2[12] != '\0' && (sh2[12] == 'k' || sh2[12] == 'K'))
	{
		return true;
	}
	return false;
}

template<typename T>
inline DWORD getPicPath(T sh2)
{
	if (sh2[0] != '\0' && (sh2[0] == 'p' || sh2[0] == 'P') &&
		sh2[1] != '\0' && (sh2[1] == 'i' || sh2[1] == 'I') &&
		sh2[2] != '\0' && (sh2[2] == 'c' || sh2[2] == 'C'))
	{
		return 3;
	}
	if (sh2[0] != '\0' && (sh2[0] == 'm' || sh2[0] == 'M') &&
		sh2[1] != '\0' && (sh2[1] == 'e' || sh2[1] == 'E') &&
		sh2[2] != '\0' && (sh2[2] == 'n' || sh2[2] == 'N') &&
		sh2[3] != '\0' && (sh2[3] == 'u' || sh2[3] == 'U') &&
		sh2[4] != '\0' &&
		sh2[5] != '\0' && (sh2[5] == 'm' || sh2[5] == 'M') &&
		sh2[6] != '\0' && (sh2[6] == 'c' || sh2[6] == 'C'))
	{
		return 4;
	}
	if (sh2[0] != '\0' && (sh2[0] == 'e' || sh2[0] == 'E') &&
		sh2[1] != '\0' && (sh2[1] == 't' || sh2[1] == 'T') &&
		sh2[2] != '\0' && (sh2[2] == 'c' || sh2[2] == 'C') &&
		sh2[3] != '\0' &&
		sh2[4] != '\0' && (sh2[4] == 'e' || sh2[4] == 'E') &&
		sh2[5] != '\0' && (sh2[5] == 'f' || sh2[5] == 'F') &&
		sh2[6] != '\0' && (sh2[6] == 'f' || sh2[6] == 'F') &&
		sh2[7] != '\0' && (sh2[7] == 'e' || sh2[7] == 'E') &&
		sh2[8] != '\0' && (sh2[8] == 'c' || sh2[8] == 'C') &&
		sh2[9] != '\0' && (sh2[9] == 't' || sh2[9] == 'T'))
	{
		return 3;
	}
	return 0;
}

template<typename T, typename D>
inline T UpdateModPath(T sh2, D str)
{
	if (!sh2 || !str || !IsFileSystemHooking || !UseCustomModFolder)
	{
		return sh2;
	}

	DWORD StrSize = strlen(sh2);
	DWORD padding = 0;
	bool isEnding = false;

	// Check if data path is found and store location
	if (isDataPath(sh2))
	{
		// Data path found at location '0', do nothing
	}
	else if (StrSize + strlen(ModPath(sh2)) + 4 > MAX_PATH)
	{
		// Game path is too long
		LOG_ONCE(__FUNCTION__ " Error: Game path is too long: '" << sh2 << "'");
		return sh2;
	}
	else if (StrSize > modLoc && isDataPath(sh2 + modLoc))
	{
		// Data path found at mod location, update padding and initialize
		padding = modLoc;
		strcpy_s(str, MAX_PATH, sh2);
	}
	else
	{
		// Could not find data path
		return sh2;
	}

	for (auto NewPath : { LangPath(sh2), ModPath(sh2) })
	{
		// Get len of NewPath
		size_t PathLen = strlen(NewPath);

		// Update path with new mod path
		strcpy_s(str + padding, MAX_PATH - padding, NewPath);
		strcpy_s(str + padding + PathLen, MAX_PATH - padding - PathLen, sh2 + padding + 4);

		// Handle end.bik/ending.bik (favor end.bik)
		if ((StrSize > padding + PathLen + 1) && isEndVideoPath(sh2 + padding + PathLen + 1))
		{
			Logging::Log() << __FUNCTION__ " " << sh2;
			isEnding = true;

			// Check mod path
			strcpy_s(str + padding + PathLen, MAX_PATH - padding - PathLen, GetEnding1(sh2));
			if (PathFileExists(str))
			{
				return str;
			}
			strcpy_s(str + padding + PathLen, MAX_PATH - padding - PathLen, GetEnding2(sh2));
			if (PathFileExists(str))
			{
				return str;
			}
		}

		// Handle PS2 low texture mod
		if (UsePS2LowResTextures)
		{
			T sh2_pic = sh2 + padding + 5;
			DWORD PicPath = getPicPath(sh2_pic);
			if (PicPath)
			{
				strcpy_s(str + padding + PathLen + 1, MAX_PATH - padding - PathLen, ModPicPath(sh2));
				strcpy_s(str + padding + PathLen + picLen + 1, MAX_PATH - padding - PathLen - picLen - 1, sh2_pic + PicPath);
			}
		}

		// If mod path exists then use it
		if (PathFileExists(str))
		{
			return str;
		}
	}

	// Handle end.bik/ending.bik (favor end.bik)
	if (isEnding)
	{
		// Check data path
		strcpy_s(str, MAX_PATH, sh2);
		strcpy_s(str + padding + 4, MAX_PATH - padding - 4, GetEnding1(sh2));
		if (PathFileExists(str))
		{
			return str;
		}
		strcpy_s(str + padding + 4, MAX_PATH - padding - 4, GetEnding2(sh2));
		if (PathFileExists(str))
		{
			return str;
		}
	}

	return sh2;
}

char* GetFileModPath(const char* sh2, const char* str)
{
	OnFileLoadTex(sh2);
	OnFileLoadVid(sh2);

	return UpdateModPath<char*, char*>((char*)sh2, (char*)str);
}
wchar_t* GetFileModPath(const wchar_t* sh2, const wchar_t* str)
{
	return UpdateModPath<wchar_t*, wchar_t*>((wchar_t*)sh2, (wchar_t*)str);
}

int WINAPI BinkOpenHandler(char* lpFileName, DWORD dwFlags)
{
	static PFN_BinkOpen org_BinkOpen = (PFN_BinkOpen)InterlockedCompareExchangePointer((PVOID*)&p_BinkOpen, nullptr, nullptr);

	if (!org_BinkOpen)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
		return NULL;
	}

	if (!IsFileSystemHooking)
	{
		return org_BinkOpen(lpFileName, dwFlags);
	}

	char Filename[MAX_PATH];
	return org_BinkOpen(GetFileModPath(lpFileName, Filename), dwFlags);
}

FILE WINAPIV fopenHandler(char const* lpFileName, char const* lpMode)
{
	static PFN_fopen org_fopen = (PFN_fopen)InterlockedCompareExchangePointer((PVOID*)&p_fopen, nullptr, nullptr);

	if (!org_fopen)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
		FILE file = {};
		return file;
	}

	if (!IsFileSystemHooking)
	{
		return org_fopen(lpFileName, lpMode);
	}

	char Filename[MAX_PATH];
	return org_fopen(GetFileModPath(lpFileName, Filename), lpMode);
}

// CreateFileA wrapper function
HANDLE WINAPI CreateFileAHandler(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	static PFN_CreateFileA org_CreateFile = (PFN_CreateFileA)InterlockedCompareExchangePointer((PVOID*)&p_CreateFileA, nullptr, nullptr);

	if (!org_CreateFile)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
		SetLastError(127);
		return INVALID_HANDLE_VALUE;
	}

	if (!IsFileSystemHooking)
	{
		return org_CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	char Filename[MAX_PATH];
	return org_CreateFile(GetFileModPath(lpFileName, Filename), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

// CreateFileW wrapper function
HANDLE WINAPI CreateFileWHandler(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	static PFN_CreateFileW org_CreateFile = (PFN_CreateFileW)InterlockedCompareExchangePointer((PVOID*)&p_CreateFileW, nullptr, nullptr);

	if (!org_CreateFile)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
		SetLastError(127);
		return INVALID_HANDLE_VALUE;
	}

	if (!IsFileSystemHooking)
	{
		return org_CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	wchar_t Filename[MAX_PATH];
	return org_CreateFile(GetFileModPath(lpFileName, Filename), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

void UpdateFindData(LPWIN32_FIND_DATAA lpFindFileData)
{
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
}

// FindFirstFileA wrapper function
HANDLE WINAPI FindFirstFileAHandler(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
	static PFN_FindFirstFileA org_FindFirstFile = (PFN_FindFirstFileA)InterlockedCompareExchangePointer((PVOID*)&p_FindFirstFileA, nullptr, nullptr);

	if (!org_FindFirstFile)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
		SetLastError(127);
		return FALSE;
	}

	if (!IsFileSystemHooking)
	{
		return org_FindFirstFile(lpFileName, lpFindFileData);
	}

	HANDLE ret = org_FindFirstFile(lpFileName, lpFindFileData);

	UpdateFindData(lpFindFileData);

	return ret;
}

// FindNextFileA wrapper function
BOOL WINAPI FindNextFileAHandler(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	static PFN_FindNextFileA org_FindNextFile = (PFN_FindNextFileA)InterlockedCompareExchangePointer((PVOID*)&p_FindNextFileA, nullptr, nullptr);

	if (!org_FindNextFile)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
		SetLastError(127);
		return FALSE;
	}

	if (!IsFileSystemHooking)
	{
		return org_FindNextFile(hFindFile, lpFindFileData);
	}

	BOOL ret = org_FindNextFile(hFindFile, lpFindFileData);

	UpdateFindData(lpFindFileData);

	return ret;
}

BOOL WINAPI CreateProcessAHandler(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	static PFN_CreateProcessA org_CreateProcess = (PFN_CreateProcessA)InterlockedCompareExchangePointer((PVOID*)&p_CreateProcessA, nullptr, nullptr);

	if (!org_CreateProcess)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";

		if (lpProcessInformation)
		{
			lpProcessInformation->dwProcessId = 0;
			lpProcessInformation->dwThreadId = 0;
			lpProcessInformation->hProcess = nullptr;
			lpProcessInformation->hThread = nullptr;
		}
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	if (isInString(lpCommandLine, "gameux.dll,GameUXShim", L"gameux.dll,GameUXShim", MAX_PATH))
	{
		Logging::Log() << __FUNCTION__ << " Disabling the GameUX CLI: " << lpCommandLine;

		char CommandLine[MAX_PATH] = { '\0' };

		for (int x = 0; x < MAX_PATH && lpCommandLine && lpCommandLine[x] != ',' && lpCommandLine[x] != '\0'; x++)
		{
			CommandLine[x] = lpCommandLine[x];
		}

		return org_CreateProcess(lpApplicationName, CommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
			lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	return org_CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
		lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

BOOL WINAPI CreateProcessWHandler(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	static PFN_CreateProcessW org_CreateProcess = (PFN_CreateProcessW)InterlockedCompareExchangePointer((PVOID*)&p_CreateProcessW, nullptr, nullptr);

	if (!org_CreateProcess)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";

		if (lpProcessInformation)
		{
			lpProcessInformation->dwProcessId = 0;
			lpProcessInformation->dwThreadId = 0;
			lpProcessInformation->hProcess = nullptr;
			lpProcessInformation->hThread = nullptr;
		}
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	if (isInString(lpCommandLine, "gameux.dll,GameUXShim", L"gameux.dll,GameUXShim", MAX_PATH))
	{
		Logging::Log() << __FUNCTION__ << " Disabling the GameUX CLI: " << lpCommandLine;

		wchar_t CommandLine[MAX_PATH] = { '\0' };

		for (int x = 0; x < MAX_PATH && lpCommandLine && lpCommandLine[x] != ',' && lpCommandLine[x] != '\0'; x++)
		{
			CommandLine[x] = lpCommandLine[x];
		}

		return org_CreateProcess(lpApplicationName, CommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
			lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	return org_CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
		lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

void InstallCreateProcessHooks()
{
	// Logging
	Logging::Log() << "Hooking the CreateProcess APIs...";

	// Hook CreateProcess APIs
	HMODULE h_kernel32 = GetModuleHandle(L"kernel32.dll");
	InterlockedExchangePointer((PVOID*)&p_CreateProcessA, Hook::HotPatch(Hook::GetProcAddress(h_kernel32, "CreateProcessA"), "CreateProcessA", *CreateProcessAHandler));
	InterlockedExchangePointer((PVOID*)&p_CreateProcessW, Hook::HotPatch(Hook::GetProcAddress(h_kernel32, "CreateProcessW"), "CreateProcessW", *CreateProcessWHandler));
}

void DisableFileSystemHooking()
{
	IsFileSystemHooking = false;
	strcpy_s(ModPathA, MAX_PATH, "data");
	wcscpy_s(ModPathW, MAX_PATH, L"data");
}

void InstallFileSystemHooks()
{
	// Logging
	Logging::Log() << "Hooking the FileSystem APIs...";

	// Hook FileSystem APIs
	HMODULE h_binkw32 = GetModuleHandle(L"binkw32.dll");
	p_BinkOpen = GetProcAddress(h_binkw32, "_BinkOpen@8");
	HMODULE h_msvcr70 = GetModuleHandle(L"msvcr70.dll");
	p_fopen = GetProcAddress(h_msvcr70, "fopen");
	HMODULE h_kernel32 = GetModuleHandle(L"kernel32.dll");
	p_CreateFileA = GetProcAddress(h_kernel32, "CreateFileA");
	p_CreateFileW = GetProcAddress(h_kernel32, "CreateFileW");
	p_FindFirstFileA = GetProcAddress(h_kernel32, "FindFirstFileA");
	p_FindNextFileA = GetProcAddress(h_kernel32, "FindNextFileA");

	// Check for failures
	if (!p_BinkOpen || !p_fopen ||
		!p_CreateFileA || !p_CreateFileW ||
		!p_FindFirstFileA || !p_FindNextFileA)
	{
		Logging::Log() << __FUNCTION__ << " Error: FAILED to hook the FileSystem APIs, disabling 'UseCustomModFolder'!";
		DisableFileSystemHooking();
		return;
	}

	if (GameVersion == SH2V_UNKNOWN)
	{
		Logging::Log() << __FUNCTION__ << " Error: unknown version of Silent Hill 2!";
		DisableFileSystemHooking();
		return;
	}

	char* BinkOpen_bytes = "E8 ? ? 13 00 85 C0 89 45 00";
	char* fopen_bytes =
		(GameVersion == SH2V_10) ? "FF 15 60 6A 4A 02" :
		(GameVersion == SH2V_11) ? "FF 15 30 AA 4A 02" :
		(GameVersion == SH2V_DC) ? "FF 15 38 9A 4A 02" : "";
	char* CreateFileA_bytes =
		(GameVersion == SH2V_10) ? "FF 15 A0 67 4A 02" :
		(GameVersion == SH2V_11) ? "FF 15 88 A7 4A 02" :
		(GameVersion == SH2V_DC) ? "FF 15 8C 97 4A 02" : "";
	char* CreateFileW_bytes =
		(GameVersion == SH2V_10) ? "FF 15 A4 67 4A 02" :
		(GameVersion == SH2V_11) ? "FF 15 8C A7 4A 02" :
		(GameVersion == SH2V_DC) ? "FF 15 90 97 4A 02" : "";
	char* FindFirstFileA_bytes =
		(GameVersion == SH2V_10) ? "FF 15 7C 68 4A 02" :
		(GameVersion == SH2V_11) ? "FF 15 10 A8 4A 02" :
		(GameVersion == SH2V_DC) ? "FF 15 18 98 4A 02" : "";
	char* FindNextFileA_bytes =
		(GameVersion == SH2V_10) ? "FF 15 80 68 4A 02" :
		(GameVersion == SH2V_11) ? "FF 15 14 A8 4A 02" :
		(GameVersion == SH2V_DC) ? "FF 15 1C 98 4A 02" : "";

	struct HOOKSTRUCT
	{
		char* Bytes;
		void* ProcAddr;
		int AddrSize;
	};

	HOOKSTRUCT HookList[] = {
		{ BinkOpen_bytes, BinkOpenHandler, 5 },
		{ fopen_bytes, fopenHandler, 6 },
		{ CreateFileA_bytes, CreateFileAHandler, 6 },
		{ CreateFileW_bytes, CreateFileWHandler, 6 },
		{ FindFirstFileA_bytes, FindFirstFileAHandler, 6 },
		{ FindNextFileA_bytes, FindNextFileAHandler, 6 },
	};

	for (auto item : HookList)
	{
		auto pattern = hook::pattern(item.Bytes);
		if (!pattern.size())
		{
			Logging::Log() << __FUNCTION__ << " Error: could not find a hook! '" << item.Bytes << "'";
		}
		for (DWORD x = 0; x < pattern.size(); x++)
		{
			WriteCalltoMemory((byte*)pattern.get(x).get<uint32_t*>(0), item.ProcAddr, item.AddrSize);
		}
	}

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

	// Get data path
	wchar_t tmpPath[MAX_PATH];
	GetModulePath(tmpPath, MAX_PATH);
	wchar_t* pdest = wcsrchr(tmpPath, '\\');
	if (pdest)
	{
		*pdest = '\0';
	}
	modLoc = wcslen(tmpPath) + 1;
	size_t modLen = strlen(ModPathA);
	picLen = strlen(ModPicPathA);
	if (modLoc + modLen + 42 > MAX_PATH)	// Check max length of a file in the game
	{
		Logging::Log() << __FUNCTION__ " Error: Game path is too long: " << modLoc + modLen;
		DisableFileSystemHooking();
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

	// Enable file system hooking flag
	IsFileSystemHooking = true;
}
