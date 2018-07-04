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
#include "NoCDPatch\nocd.h"
#include "SFX\sfx.h"
#include "d3d8to9\d3d8to9.h"
#include "Hooking\Hook.h"
#include "Wrappers\wrapper.h"
#include "Common\LoadASI.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Common\Logging.h"

// Basic logging
std::ofstream LOG;
char LogPath[MAX_PATH];

// Configurable settings
bool EnableSFXAddrHack = true;
bool d3d8to9 = true;
bool ResetScreenRes = true;
bool NoCDPatch = true;
bool LoadPlugins = true;
bool LoadFromScriptsOnly = true;

// Get config settings from string (file)
void __stdcall ParseCallback(char* name, char* value)
{
	// Check for valid entries
	if (!IsValidSettings(name, value)) return;

	// Check settings
	if (!_strcmpi(name, "EnableSFXAddrHack")) EnableSFXAddrHack = SetValue(value);
	if (!_strcmpi(name, "d3d8to9")) ResetScreenRes = SetValue(value);
	if (!_strcmpi(name, "ResetScreenRes")) ResetScreenRes = SetValue(value);
	if (!_strcmpi(name, "NoCDPatch")) NoCDPatch = SetValue(value);
	if (!_strcmpi(name, "LoadPlugins")) NoCDPatch = SetValue(value);
	if (!_strcmpi(name, "LoadFromScriptsOnly")) NoCDPatch = SetValue(value);
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Set thread priority a trick to reduce concurrency problems at program startup
		HANDLE hCurrentThread = GetCurrentThread();
		int dwPriorityClass = GetThreadPriority(hCurrentThread);
		dwPriorityClass = (GetLastError() == THREAD_PRIORITY_ERROR_RETURN) ? THREAD_PRIORITY_NORMAL : dwPriorityClass;
		SetThreadPriority(hCurrentThread, THREAD_PRIORITY_HIGHEST);

		// Get log file path and open log file
		char pathname[MAX_PATH];
		GetModuleFileNameA(hModule, pathname, MAX_PATH);
		strcpy_s(strrchr(pathname, '.'), MAX_PATH - strlen(pathname), ".log");
		LOG.open(pathname);

		// Starting
		Log() << "Starting Silent Hill 2 Enhancements!";

		// Get Silent Hill 2 file path
		GetModuleFileNameA(nullptr, pathname, MAX_PATH);
		Log() << "Running from:  " << pathname;

		// Get config file path
		GetModuleFileNameA(hModule, pathname, MAX_PATH);
		strcpy_s(strrchr(pathname, '.'), MAX_PATH - strlen(pathname), ".ini");

		// Read config file
		Log() << "Reading config file:  " << pathname;
		char* szCfg = Read(pathname);

		// Parce config file
		if (szCfg)
		{
			Parse(szCfg, ParseCallback);
			free(szCfg);
		}
		else
		{
			Log() << "Config file not found, using defaults";
		}

		// Create wrapper
		HMODULE dll = Wrapper::CreateWrapper();
		if (dll)
		{
			AddHandleToVector(dll);
			Log() << "Wrapper created for " << dtypename[Wrapper::dtype];
		}
		else if (Wrapper::dtype != DTYPE_ASI)
		{
			Log() << "Error: could not create wrapper!";
		}

		// Enable d3d8to9
		if (d3d8to9)
		{
			HMODULE d3d9_dll = LoadLibrary(L"d3d9.dll");
			if (d3d9_dll)
			{
				AddHandleToVector(d3d9_dll);
				p_Direct3DCreate9 = GetProcAddress(d3d9_dll, "Direct3DCreate9");
				if (Wrapper::dtype == DTYPE_D3D8)
				{
					d3d8::Direct3DCreate8_var = p_Direct3DCreate8to9;
				}
				else
				{
					// Load d3d8 procs
					HMODULE d3d8_dll = LoadLibrary(L"d3d8.dll");
					AddHandleToVector(d3d8_dll);

					// Hook d3d8.dll -> D3d8to9
					Log() << "Hooking d3d8.dll APIs...";
					Hook::HookAPI(dll, "d3d8.dll", Hook::GetProcAddress(d3d8_dll, "Direct3DCreate8"), "Direct3DCreate8", p_Direct3DCreate8to9);
				}
			}
			else
			{
				Log() << "Error: could not load d3d9.dll!";
			}
		}

		// Enable No-CD Patch
		if (NoCDPatch)
		{
			DisableCDCheck();
		}

		// Update SFX addresses
		if (EnableSFXAddrHack)
		{
			UpdateSFXAddr();
		}

		// Load ASI pluggins
		if (LoadPlugins)
		{
			LoadASIPlugins(LoadFromScriptsOnly);
		}

		// Resetting thread priority
		SetThreadPriority(hCurrentThread, dwPriorityClass);

		// Closing handle
		CloseHandle(hCurrentThread);
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_PROCESS_DETACH:

		// Reset screen back to original Windows settings to fix some display errors on exit
		if (ResetScreenRes)
		{
			// Reset screen settings
			Log() << "Reseting screen resolution...";
			std::string lpRamp((3 * 256 * 2), '\0');
			HDC hDC = GetDC(nullptr);
			GetDeviceGammaRamp(hDC, &lpRamp[0]);
			Sleep(0);
			SetDeviceGammaRamp(hDC, &lpRamp[0]);
			ReleaseDC(nullptr, hDC);
			Sleep(0);
			ChangeDisplaySettings(nullptr, 0);
		}

		// Unloading all modules
		if (custom_dll.size())
		{
			Log() << "Unloading all loaded modules";
			for (HMODULE it : custom_dll)
			{
				if (it)
				{
					FreeLibrary(it);
				}
			}
		}

		// Quiting
		Log() << "Unloading Silent Hill 2 Enhancements!";

		break;
	}

	return true;
}
