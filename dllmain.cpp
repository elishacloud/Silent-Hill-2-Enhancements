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
#include <vector>
#include "Resources\sh2-enhce.h"
#include "Patches\Patches.h"
#include "Hooking\Hook.h"
#include "Hooking\FileSystemHooks.h"
#include "Wrappers\wrapper.h"
#include "Wrappers\d3d8to9.h"
#include "Common\LoadModules.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Common\Logging.h"

// Screen settings
HDC hDC;
std::string lpRamp((3 * 256 * 2), '\0');

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
		wchar_t pathname[MAX_PATH];
		GetModuleFileName(hModule, pathname, MAX_PATH);
		wcscpy_s(wcsrchr(pathname, '.'), MAX_PATH - wcslen(pathname), L".log");
		LOG.open(pathname);

		// Starting
		Log() << "Starting Silent Hill 2 Enhancements! v" << APP_VERSION;

		// Init Logs
		LogComputerManufacturer();
		LogVideoCard();
		LogOSVersion();

		// Get Silent Hill 2 file path
		GetModuleFileName(nullptr, pathname, MAX_PATH);
		Log() << "Running from:  " << pathname;

		// Get config file path
		GetModuleFileName(hModule, pathname, MAX_PATH);
		wcscpy_s(wcsrchr(pathname, '.'), MAX_PATH - wcslen(pathname), L".ini");

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
			Log() << __FUNCTION__ << " Error: Config file not found, using defaults";
		}

		// Hook CreateFile API, only needed for external modules
		if (Nemesis2000FogFix || WidescreenFix)
		{
			InstallFileSystemHooks(hModule, pathname);
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
			Log() << __FUNCTION__ << " Error: could not create wrapper!";
		}

		// Store screen settings
		if (ResetScreenRes)
		{
			// Reset screen settings
			hDC = GetDC(nullptr);
			GetDeviceGammaRamp(hDC, &lpRamp[0]);
		}

		// Load modupdater
		if (AutoUpdateModule)
		{
			// Get module name
			wchar_t Path[MAX_PATH], Name[MAX_PATH];
			GetModuleFileName(hModule, Path, MAX_PATH);
			wcscpy_s(Name, MAX_PATH, wcsrchr(Path, '\\'));

			// Get 'temp' path
			GetTempPath(MAX_PATH, Path);
			wcscat_s(Path, MAX_PATH, L"~tmp_sh2_enhce");
			CreateDirectory(Path, nullptr);

			// Update path with module name
			wcscat_s(Path, MAX_PATH, Name);
			wcscpy_s(wcsrchr(Path, '.'), MAX_PATH, L".tmp");

			// Load module
			LoadModuleFromResourceToFile(hModule, IDR_SH2UPD, L"modupdater", Path);
		}

		// Enable d3d8to9
		if (d3d8to9)
		{
			EnableD3d8to9();
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

		// PS2 Noise Filter
		if (PS2StyleNoiseFilter)
		{
			UpdatePS2NoiseFilter();
		}

		// Draw Distance
		if (IncreaseDrawDistance)
		{
			UpdateDrawDistance();
		}

		// Cemetery Lighting Fix
		if (CemeteryLightingFix)
		{
			UpdateCemeteryLighting();
		}

		// Rowboat Animation Fix
		if (RowboatAnimationFix)
		{
			UpdateRowboatAnimation();
		}

		// Load Nemesis2000's Fog Fix
		if (Nemesis2000FogFix)
		{
			LoadModuleFromResource(hModule, IDR_SH2FOG, L"Nemesis2000 Fog Fix");
		}

		// Widescreen Fix
		if (WidescreenFix)
		{
			LoadModuleFromResource(hModule, IDR_SH2WID, L"WidescreenFixesPack and sh2proxy");
		}

		// Load ASI pluggins
		if (LoadPlugins && Wrapper::dtype != DTYPE_ASI)
		{
			LoadASIPlugins(LoadFromScriptsOnly);
		}

		// Load forced windowed mode
		if (EnableWndMode)
		{
			// Get 'temp' path
			wchar_t Path[MAX_PATH];
			GetTempPath(MAX_PATH, Path);
			wcscat_s(Path, MAX_PATH, L"~tmp_sh2_enhce");
			CreateDirectory(Path, nullptr);
			wcscat_s(Path, MAX_PATH, L"\\wndmode.tmp");

			// Load module
			LoadModuleFromResourceToFile(hModule, IDR_SH2WND, L"WndMode", Path);
		}

		// Resetting thread priority
		SetThreadPriority(hCurrentThread, dwPriorityClass);

		// Closing handle
		CloseHandle(hCurrentThread);
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		// Set thread priority a trick to reduce concurrency problems
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

		// Unloading all modules
		Log() << "Unloading all loaded modules";

		// Unload memory modules
		UnloadResourceModules();

		// Unload standard modules
		UnloadAllModules();

		// Unhook APIs
		Log() << "Unhooking library functions";
		Hook::UnhookAll();

		// Reset screen back to original Windows settings to fix some display errors on exit
		if (ResetScreenRes)
		{
			// Reset screen settings
			Log() << "Reseting screen resolution";
			SetDeviceGammaRamp(hDC, &lpRamp[0]);
			ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);
		}

		// Quitting
		Log() << "Unloading Silent Hill 2 Enhancements!";
	}
	break;
	}

	return true;
}
