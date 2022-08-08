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
#include <fstream>
#include <iterator>
#include <vector>
#include "External\MemoryModule\MemoryModule.h"
#include "Logging\Logging.h"

// For Logging
std::ofstream LOG;

HMEMORYMODULE d3d8dll = nullptr;

FARPROC m_pDirect3DCreate8 = nullptr;

std::vector<byte> buffer;

wchar_t modpathw[MAX_PATH] = { '\0' };

bool WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Get dll name
		bool ret = (GetModuleFileNameW(hModule, modpathw, MAX_PATH) != 0);

		wchar_t* pdest = wcsrchr(modpathw, '\\');
		if (ret && pdest)
		{
			wcscpy_s(pdest + 1, MAX_PATH - wcslen(modpathw), L"d3d8.dll");
		}

		// open dll file
		std::ifstream input(modpathw, std::ios::binary);

		// copy all data into buffer
		buffer.assign(std::istreambuf_iterator<char>(input), {});

		input.close();

		if (buffer.size())
		{
			// Get log file path and open log file
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

			d3d8dll = MemoryLoadLibrary(&buffer[0], buffer.size());

			Logging::Log() << "Loaded add: " << (void*)d3d8dll;

			// Get function address
			m_pDirect3DCreate8 = MemoryGetProcAddress(d3d8dll, "Direct3DCreate8");
		}
	}
		break;

	case DLL_PROCESS_DETACH:
		MemoryFreeLibrary(d3d8dll);
		break;
	}

	return true;
}

__declspec(naked) void Direct3DCreate8()
{
	__asm
	{
		jmp m_pDirect3DCreate8
	}
}

HMEMORYMODULE WINAPI GetMemoryModuleHandle()
{
	Logging::Log() << __FUNCTION__;

	return d3d8dll;
}
