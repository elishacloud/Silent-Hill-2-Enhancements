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
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef void (WINAPI* DelayedStartProc)();

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	static HMODULE d3d8_dll = nullptr;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get process path
		wchar_t Path[MAX_PATH] = {};
		DWORD ret = GetModuleFileName(hModule, Path, MAX_PATH);

		// Get d3d8 local path
		wchar_t* pdest = wcsrchr(Path, '\\');
		if (ret && pdest)
		{
			*pdest = '\0';
		}
		wcscat_s(Path, MAX_PATH, L"\\d3d8.dll");

		// Load d3d8 module locally if not already loaded
		if (!GetModuleHandle(Path))
		{
			d3d8_dll = LoadLibrary(Path);
		}
		break;
	}
	case DLL_PROCESS_DETACH:
		if (d3d8_dll)
		{
			FreeLibrary(d3d8_dll);
		}
		break;
	}

	return TRUE;
}
