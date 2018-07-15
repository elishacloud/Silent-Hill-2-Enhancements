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
#include "External\MemoryModule\MemoryModule.h"
#include "Wrappers\wrapper.h"
#include "Wrappers\d3d8to9.h"
#include "Common\LoadASI.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Common\Logging.h"

// Basic logging
std::ofstream LOG;
wchar_t LogPath[MAX_PATH];

// Memory modules
struct MMODULE
{
	HMEMORYMODULE handle;		// Module handle
	DWORD ResID;				// Resource ID
};
std::vector<MMODULE> HMModules;

// Screen settings
std::string lpRamp((3 * 256 * 2), '\0');

// Configurable setting defaults
bool d3d8to9 = true;
bool CemeteryLightingFix = true;
bool EnableSFXAddrHack = true;
bool EnableWndMode = false;
bool IncreaseDrawDistance = true;
bool LoadFromScriptsOnly = false;
bool LoadPlugins = false;
bool Nemesis2000FogFix = true;
bool NoCDPatch = true;
bool PS2StyleNoiseFilter = true;
bool ResetScreenRes = true;
bool RowboatAnimationFix = true;
bool WidescreenFix = true;

// Get config settings from string (file)
void __stdcall ParseCallback(char* name, char* value)
{
	// Check for valid entries
	if (!IsValidSettings(name, value)) return;

	// Check settings
	if (!_strcmpi(name, "d3d8to9")) d3d8to9 = SetValue(value);
	if (!_strcmpi(name, "CemeteryLightingFix")) CemeteryLightingFix = SetValue(value);
	if (!_strcmpi(name, "EnableSFXAddrHack")) EnableSFXAddrHack = SetValue(value);
	if (!_strcmpi(name, "EnableWndMode")) EnableWndMode = SetValue(value);
	if (!_strcmpi(name, "IncreaseDrawDistance")) IncreaseDrawDistance = SetValue(value);
	if (!_strcmpi(name, "LoadFromScriptsOnly")) LoadFromScriptsOnly = SetValue(value);
	if (!_strcmpi(name, "LoadPlugins")) LoadPlugins = SetValue(value);
	if (!_strcmpi(name, "Nemesis2000FogFix")) Nemesis2000FogFix = SetValue(value);
	if (!_strcmpi(name, "NoCDPatch")) NoCDPatch = SetValue(value);
	if (!_strcmpi(name, "PS2StyleNoiseFilter")) PS2StyleNoiseFilter = SetValue(value);
	if (!_strcmpi(name, "ResetScreenRes")) ResetScreenRes = SetValue(value);
	if (!_strcmpi(name, "RowboatAnimationFix")) RowboatAnimationFix = SetValue(value);
	if (!_strcmpi(name, "WidescreenFix")) WidescreenFix = SetValue(value);
}

// Load memory module from resource
void LoadModuleFromResource(HMODULE hModule, DWORD ResID, LPCSTR lpName)
{
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(ResID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource(hLoadedResource);
			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource(hModule, hResource);
				if (dwResourceSize != 0)
				{
					Log() << "Loading the " << lpName << " module...";
					LoadingMemoryModule = true;
					HMEMORYMODULE hMModule = MemoryLoadLibrary((const void*)pLockedResource, dwResourceSize);
					LoadingMemoryModule = false;
					if (hMModule)
					{
						MMODULE MMItem = { hMModule , ResID };
						HMModules.push_back(MMItem);
					}
					else
					{
						Log() << __FUNCTION__ << " Error: " << lpName << " module could not be loaded!";
					}
				}
			}
		}
	}
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
			HDC hDC = GetDC(nullptr);
			GetDeviceGammaRamp(hDC, &lpRamp[0]);
			ReleaseDC(nullptr, hDC);
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
			LoadModuleFromResource(hModule, IDR_SH2FOG, "Nemesis2000 Fog Fix");
		}

		// Widescreen Fix
		if (WidescreenFix)
		{
			LoadModuleFromResource(hModule, IDR_SH2WID, "WidescreenFixesPack and sh2proxy");
		}

		// Load forced windowed mode
		if (EnableWndMode)
		{
			LoadModuleFromResource(hModule, IDR_SH2WND, "WndMode");
		}

		// Load ASI pluggins
		if (LoadPlugins && Wrapper::dtype != DTYPE_ASI)
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
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
#ifdef _DEBUG
		// Unloading all modules
		Log() << "Unloading all loaded modules";

		// Unload standard modules
		for (HMODULE it : custom_dll)
		{
			if (it)
			{
				FreeLibrary(it);
			}
		}

		// Unload memory modules
		for (MMODULE it : HMModules)
		{
			MemoryFreeLibrary(it.handle);
		}
#endif

		// Unhook APIs
		Log() << "Unhooking library functions";
		Hook::UnhookAll();

		// Reset screen back to original Windows settings to fix some display errors on exit
		if (ResetScreenRes)
		{
			// Reset screen settings
			Log() << "Reseting screen resolution";
			HDC hDC = GetDC(nullptr);
			SetDeviceGammaRamp(hDC, &lpRamp[0]);
			ReleaseDC(nullptr, hDC);
			ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);
		}

		// Quitting
		Log() << "Unloading Silent Hill 2 Enhancements!";
	}
	break;
	}

	return true;
}
