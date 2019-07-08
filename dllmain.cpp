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
#include <Shlwapi.h>
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

#pragma comment(lib, "Shlwapi.lib")

// For Logging
std::ofstream LOG;

// Variables
HMODULE wrapper_dll = nullptr;

// Forces Nvidia and AMD high performance graphics
extern "C" { _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
extern "C" { _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; }

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
		Logging::LogProcessNameAndPID();

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

		// Get wrapper mode
		Wrapper::GetWrapperMode(hModule);

		// Load wrapper d3d8 from scripts or plugins folder
		HMODULE script_d3d8_dll = nullptr;
		if (LoadD3d8FromScriptsFolder && Wrapper::dtype == DTYPE_D3D8)
		{
			// Get script paths
			wchar_t path[MAX_PATH];
			wcscpy_s(path, MAX_PATH, pathname);
			wcscpy_s(wcsrchr(path, '\\'), MAX_PATH - wcslen(path), L"\0");
			std::wstring separator = L"\\";
			std::wstring script_path(path + separator + L"scripts");
			std::wstring script_path_dll(script_path + L"\\d3d8.dll");
			std::wstring plugin_path(path + separator + L"plugins");
			std::wstring plugin_path_dll(plugin_path + L"\\d3d8.dll");

			// Store the current folder
			wchar_t oldDir[MAX_PATH] = { 0 };
			GetCurrentDirectory(MAX_PATH, oldDir);

			// Load d3d8.dll from 'scripts' folder
			SetCurrentDirectory(script_path.c_str());
			script_d3d8_dll = LoadLibrary(script_path_dll.c_str());
			if (script_d3d8_dll)
			{
				Logging::Log() << "Loaded d3d8.dll from: " << script_path_dll.c_str();
			}

			// Load d3d8.dll from 'plugins' folder
			if (!script_d3d8_dll && Wrapper::dtype == DTYPE_D3D8)
			{
				SetCurrentDirectory(plugin_path.c_str());
				script_d3d8_dll = LoadLibrary(plugin_path_dll.c_str());
				if (script_d3d8_dll)
				{
					Logging::Log() << "Loaded d3d8.dll from: " << plugin_path_dll.c_str();
				}
			}

			// Set current folder back
			SetCurrentDirectory(oldDir);
		}

		// Create wrapper
		wrapper_dll = Wrapper::CreateWrapper((Wrapper::dtype == DTYPE_D3D8) ? script_d3d8_dll : nullptr);
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

		// Check for required DirectX 9 runtime files
		wchar_t d3dx9_path[MAX_PATH], xaudio2_path[MAX_PATH];
		GetSystemDirectory(d3dx9_path, MAX_PATH);
		wcscat_s(d3dx9_path, L"\\d3dx9_43.dll");
		GetSystemDirectory(xaudio2_path, MAX_PATH);
		wcscat_s(xaudio2_path, L"\\xaudio2_7.dll");
		if (!PathFileExists(d3dx9_path) || !PathFileExists(xaudio2_path))
		{
			Logging::Log() << "Warning: Could not find expected DirectX 9.0c End-User Runtime files.  Try installing from: https://www.microsoft.com/download/details.aspx?id=35";
		}

		// Hook CreateFile API, only needed for external modules and UseCustomModFolder
		if (Nemesis2000FogFix || WidescreenFix || UseCustomModFolder)
		{
			InstallFileSystemHooks(hModule, configpath);
		}

		// Fix Windows Game Explorer issue
		if (DisableGameUX)
		{
			InstallCreateProcessHooks();
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

		// Lighting Transition fix
		if (LightingTransitionFix)
		{
			UpdateLightingTransition();
		}

		// XInput based vibration
		if (XInputVibration)
		{
			UpdateXInputVibration();
		}

		// DPad movement
		if (DPadMovementFix)
		{
			UpdateDPadMovement();
		}

		// Loads font texture form tga file
		if (UseCustomFonts)
		{
			UpdateCustomFonts();
		}

		// Load exe's strings from txt file
		if (UseCustomExeStr)
		{
			UpdateCustomExeStr();
		}

		// Disables the ability to change resolution, displays currently used
		if (LockResolution)
		{
			UpdateResolutionLock();
		}

		// Fixes mouse hitboxes in Main Menu (for 1.1 version)
		if (MainMenuFix)
		{
			UpdateMainMenuFix();
		}

		// Loads 'start01.tex' (graphic Main Menu) according to the selected language
		if (MainMenuTitlePerLang)
		{
			UpdateMainMenuTitlePerLang();
		}

		// Load Nemesis2000's Fog Fix
		if (Nemesis2000FogFix)
		{
			if (LoadModulesFromMemory)
			{
				LoadModuleFromResource(hModule, IDR_SH2FOG, L"Nemesis2000 Fog Fix");
			}
			else
			{
				LoadModuleFromFile(hModule, IDR_SH2FOG, L"sh2fog.ini", configpath, L"Nemesis2000 Fog Fix", false);
			}
		}

		// Widescreen Fix
		if (WidescreenFix)
		{
			DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
			HANDLE hCurrentProcess = GetCurrentProcess();
			bool Flag = GetProcessAffinityMask(hCurrentProcess, &ProcessAffinityMask, &SystemAffinityMask);
			if (LoadModulesFromMemory)
			{
				LoadModuleFromResource(hModule, IDR_SH2WID, L"WidescreenFixesPack and sh2proxy");
			}
			else
			{
				LoadModuleFromFile(hModule, IDR_SH2WID, nullptr, configpath, L"WidescreenFixesPack and sh2proxy", true);
			}
			if (Flag)
			{
				SetProcessAffinityMask(hCurrentProcess, ProcessAffinityMask);
			}
			CloseHandle(hCurrentProcess);
		}

		// Load modupdater
		if (AutoUpdateModule)
		{
			if (LoadModulesFromMemory)
			{
				LoadModuleFromResource(hModule, IDR_SH2UPD, L"modupdater");
			}
			else
			{
				LoadModuleFromFile(hModule, IDR_SH2UPD, nullptr, configpath, L"modupdater", false);
			}
		}

		// Load ASI pluggins
		if (LoadPlugins && Wrapper::dtype != DTYPE_ASI)
		{
			LoadASIPlugins(LoadFromScriptsOnly);
		}

		// Set single core affinity
		if (SingleCoreAffinity)
		{
			if (SingleCoreAffinityTimer)
			{
				DWORD ThreadID = 0;
				static DWORD SleepTime = SingleCoreAffinityTimer;
				CreateThread(nullptr, 0, SetSingleCoreAffinity, &SleepTime, 0, &ThreadID);
			}
			else
			{
				SetSingleCoreAffinity();
			}
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
		// Unhook window handle
		void UnhookWindowHandle();

		// Unhook APIs
		Logging::Log() << "Unhooking library functions";
		Hook::UnhookAll();

#ifdef DEBUG
		// Unloading all modules
		Logging::Log() << "Unloading all loaded modules";

		// Unload standard modules
		UnloadAllModules();

		// Unload memory modules
		UnloadResourceModules();

		// Unload wrapped dll file
		if (wrapper_dll)
		{
			FreeModule(wrapper_dll);
		}
#endif // DEBUG

		// Delete temp directories
		if (!LoadModulesFromMemory)
		{
			Logging::Log() << "Removing temp folders";
			RemoveTempFolders();
		}

		// Quitting
		Logging::Log() << "Unloading Silent Hill 2 Enhancements!";
	}
	break;
	}

	return true;
}
