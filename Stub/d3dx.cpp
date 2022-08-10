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

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <shlwapi.h>
#include "External\Hooking\Hook.h"
#include "Logging\Logging.h"

typedef DWORD(WINAPI* PFN_GetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef DWORD(WINAPI* PFN_GetModuleFileNameW)(HMODULE, LPWSTR, DWORD);

DWORD WINAPI GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFileName, DWORD nSize);
DWORD WINAPI GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFileName, DWORD nSize);

FARPROC p_GetModuleFileNameA = nullptr;
FARPROC p_GetModuleFileNameW = nullptr;

// For Logging
std::ofstream LOG;

HMODULE d3d8dll = nullptr;

FARPROC m_pDirect3DCreate8 = nullptr;

char modpatha[MAX_PATH] = { '\0' };
wchar_t modpathw[MAX_PATH] = { '\0' };

bool WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get dll name
		{
			bool ret = (GetModuleFileNameA(hModule, modpatha, MAX_PATH) != 0);

			char* pdest = strrchr(modpatha, '\\');
			if (ret && pdest)
			{
				strcpy_s(pdest + 1, MAX_PATH - strlen(modpatha), "d3d8.dll");
			}
		}

		bool ret = (GetModuleFileNameW(hModule, modpathw, MAX_PATH) != 0);

		wchar_t* pdest = wcsrchr(modpathw, '\\');
		if (ret && pdest)
		{
			wcscpy_s(pdest + 1, MAX_PATH - wcslen(modpathw), L"d3d8.dll");
		}

		// Get log path and open log file
		wchar_t logpath[MAX_PATH];
		ret = (GetModuleFileNameW(hModule, logpath, MAX_PATH) != 0);
		pdest = wcsrchr(logpath, '.');
		if (ret && pdest)
		{
			wcscpy_s(pdest + 1, MAX_PATH - wcslen(logpath), L"log");
			Logging::EnableLogging = true;
			Logging::Open(logpath);
		}

		// Starting
		Logging::Log() << "Starting d3dx Stub";
		Logging::Log() << "Loading " << modpathw;

		if (PathFileExistsW(modpathw))
		{

			// Hook GetModuleFileName and GetModuleHandleEx to fix module name in modules loaded from memory
			Logging::Log() << "Hooking the FileSystem APIs...";
			HMODULE h_kernel32 = GetModuleHandle(L"kernel32.dll");
			InterlockedExchangePointer((PVOID)&p_GetModuleFileNameA, Hook::HotPatch(Hook::GetProcAddress(h_kernel32, "GetModuleFileNameA"), "GetModuleFileNameA", *GetModuleFileNameAHandler));
			InterlockedExchangePointer((PVOID)&p_GetModuleFileNameW, Hook::HotPatch(Hook::GetProcAddress(h_kernel32, "GetModuleFileNameW"), "GetModuleFileNameW", *GetModuleFileNameWHandler));

			// Get temporary folder name
			wchar_t temppath[MAX_PATH];
			wcscpy_s(temppath, MAX_PATH, std::filesystem::temp_directory_path().c_str());
			wcscat_s(temppath, MAX_PATH, L"~d3d8.dll");

			// Copy module into temp folder
			Logging::Log() << "Copying module to: " << temppath;
			if (!CopyFile(modpathw, temppath, FALSE))
			{
				Logging::Log() << "Error: Failed to copy dll to temp folder!";
				return false;
			}

			// Load dll from temp folder
			d3d8dll = LoadLibrary(temppath);
			if (!d3d8dll)
			{
				Logging::Log() << "Error: Failed to load dll from temp folder!";
				return false;
			}

			Logging::Log() << "Loaded add: " << (void*)d3d8dll;

			// Get function address
			m_pDirect3DCreate8 = GetProcAddress(d3d8dll, "Direct3DCreate8");
		}
	}
		break;

	case DLL_PROCESS_DETACH:
		FreeLibrary(d3d8dll);
		break;
	}

	return true;
}

DWORD WINAPI GetModuleFileNameAHandler(HMODULE hModule, LPSTR lpFileName, DWORD nSize)
{
	static PFN_GetModuleFileNameA org_GetModuleFileName = (PFN_GetModuleFileNameA)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleFileNameA, nullptr, nullptr);

	if (!org_GetModuleFileName)
	{
		if (!org_GetModuleFileName)
		{
			Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
		}
		SetLastError(5);
		return NULL;
	}

	DWORD hr = org_GetModuleFileName(hModule, lpFileName, nSize);

	if (hr)
	{
		char* pdest = strrchr(lpFileName, '\\');
		if (pdest && _stricmp(pdest + 1, "~d3d8.dll") == 0)
		{
			strcpy_s(lpFileName, nSize, modpatha);

			hr = strlen(lpFileName);
		}
	}

	return hr;
}

DWORD WINAPI GetModuleFileNameWHandler(HMODULE hModule, LPWSTR lpFileName, DWORD nSize)
{
	static PFN_GetModuleFileNameW org_GetModuleFileName = (PFN_GetModuleFileNameW)InterlockedCompareExchangePointer((PVOID*)&p_GetModuleFileNameW, nullptr, nullptr);

	if (!org_GetModuleFileName)
	{
		if (!org_GetModuleFileName)
		{
			Logging::Log() << __FUNCTION__ << " Error: invalid proc address!";
		}
		SetLastError(5);
		return NULL;
	}

	DWORD hr = org_GetModuleFileName(hModule, lpFileName, nSize);

	if (hr)
	{
		wchar_t* pdest = wcsrchr(lpFileName, '\\');
		if (pdest && _wcsicmp(pdest + 1, L"~d3d8.dll") == 0)
		{
			wcscpy_s(lpFileName, nSize, modpathw);

			hr = wcslen(lpFileName);
		}
	}

	return hr;
}

__declspec(naked) void Direct3DCreate8()
{
	__asm
	{
		jmp m_pDirect3DCreate8
	};
}
