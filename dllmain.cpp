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
#include <vector>
#include "Resources\sh2-enhce.h"
#include "Patches\Patches.h"
#include "External\Hooking\Hook.h"
#include "Common\FileSystemHooks.h"
#include "Wrappers\wrapper.h"
#include "Wrappers\d3d8to9.h"
#include "Common\LoadModules.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

// For Logging
std::ofstream LOG;

// Screen settings
HDC hDC;
std::string lpRamp((3 * 256 * 2), '\0');

// Variables
HMODULE wrapper_dll = nullptr;

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

		// Get config file path
		wchar_t configpath[MAX_PATH];
		GetModuleFileName(hModule, configpath, MAX_PATH);
		wcscpy_s(wcsrchr(configpath, '.'), MAX_PATH - wcslen(configpath), L".ini");

		// Read config file
		char* szCfg = Read(configpath);

		// Parce config file
		bool IsLoadConfig = false;
		if (szCfg)
		{
			IsLoadConfig = true;
			Parse(szCfg, ParseCallback);
			free(szCfg);
		}

		// Get log file path and open log file
		wchar_t pathname[MAX_PATH];
		GetModuleFileName(hModule, pathname, MAX_PATH);
		wcscpy_s(wcsrchr(pathname, '.'), MAX_PATH - wcslen(pathname), L".log");
		Logging::EnableLogging = !DisableLogging;
		Logging::Open(pathname);

		// Starting
		Logging::Log() << "Starting Silent Hill 2 Enhancements! v" << APP_VERSION;

		// Init Logs
		Logging::LogComputerManufacturer();
		Logging::LogVideoCard();
		Logging::LogOSVersion();

		// Get Silent Hill 2 file path
		GetModuleFileName(nullptr, pathname, MAX_PATH);
		Logging::Log() << "Running from: " << pathname;

		if (IsLoadConfig)
		{
			Logging::Log() << "Config file: " << configpath;
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Error: Config file not found, using defaults";
		}

		// Store screen settings
		if (ResetScreenRes)
		{
			// Reset screen settings
			hDC = GetDC(nullptr);
			GetDeviceGammaRamp(hDC, &lpRamp[0]);
		}

		// Create wrapper
		wrapper_dll = Wrapper::CreateWrapper();
		if (wrapper_dll)
		{
			Logging::Log() << "Wrapper created for " << dtypename[Wrapper::dtype];
		}
		else if (Wrapper::dtype != DTYPE_ASI)
		{
			Logging::Log() << __FUNCTION__ << " Error: could not create wrapper!";
		}

		// Hook d3d8.dll
		if (Wrapper::dtype != DTYPE_D3D8)
		{
			// Load d3d8 procs
			HMODULE d3d8_dll = LoadLibrary(L"d3d8.dll");
			AddHandleToVector(d3d8_dll);

			// Hook d3d8.dll -> d3d8wrapper
			Logging::Log() << "Hooking d3d8.dll APIs...";
			d3d8::Direct3DCreate8_var = (FARPROC)Hook::HotPatch(Hook::GetProcAddress(d3d8_dll, "Direct3DCreate8"), "Direct3DCreate8", p_Direct3DCreate8Wrapper, d3d8to9);
		}

		// Enable d3d8to9
		if (d3d8to9)
		{
			EnableD3d8to9();
		}

		// Hook CreateFile API, only needed for external modules and UseCustomModFolder
		if (Nemesis2000FogFix || WidescreenFix || UseCustomModFolder)
		{
			InstallFileSystemHooks(hModule, configpath);
		}

		// Set single core affinity
		if (SingleCoreAffinity)
		{
			SetSingleCoreAffinity();
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

		// Fix issue with saving the gome on a drive that is larger than 2TBs
		if (ImproveStorageSupport)
		{
			Update2TBHardDriveFix();
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

		// Catacombs Meat Room
		if (CatacombsMeatRoomFix)
		{
			UpdateCatacombsMeatRoom();
		}

		// Fog adjustment fixes
		if (FogParameterFix)
		{
			UpdateFogParameters();
		}

		// Piston room fix
		if (PistonRoomFix)
		{
			UpdatePistonRoom();
		}

		// Red Cross health indicator in cutscene
		if (DisableRedCrossInCutScenes)
		{
			UpdateRedCrossInCutscene();
		}

		// Adjusts flashlight brightness
		if (PS2FlashlightBrightness)
		{
			UpdatePS2Flashlight();
		}

		// Loads font texture form tga file
		if (UseCustomFonts)
		{
			UpdateCustomFonts();
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

		// Load modupdater
		if (AutoUpdateModule)
		{
			LoadModUpdater(hModule, IDR_SH2UPD);
		}

		// Load ASI pluggins
		if (LoadPlugins && Wrapper::dtype != DTYPE_ASI)
		{
			LoadASIPlugins(LoadFromScriptsOnly);
		}

		// Loaded
		Logging::Log() << "Silent Hill 2 Enhancements module loaded!";

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
#ifdef DEBUG
		// Unloading all modules
		Logging::Log() << "Unloading all loaded modules";

		// Unload memory modules
		UnloadResourceModules();

		// Unload standard modules
		UnloadAllModules();

		// Unload wrapped dll file
		if (wrapper_dll)
		{
			FreeModule(wrapper_dll);
		}
#endif // DEBUG

		// Unhook APIs
		Logging::Log() << "Unhooking library functions";
		Hook::UnhookAll();

		// Reset screen back to original Windows settings to fix some display errors on exit
		if (ResetScreenRes)
		{
			// Reset screen settings
			Logging::Log() << "Reseting screen resolution";
			SetDeviceGammaRamp(hDC, &lpRamp[0]);
			ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);
			ReleaseDC(nullptr, hDC);
		}

		// Quitting
		Logging::Log() << "Unloading Silent Hill 2 Enhancements!";
	}
	break;
	}

	return true;
}
