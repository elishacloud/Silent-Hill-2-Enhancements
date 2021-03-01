/**
* Copyright (C) 2021 Elisha Riedlinger
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
#include "Resources\sh2-enhce.h"
#include "Patches\Patches.h"
#include "WidescreenFixesPack\WidescreenFixesPack.h"
#include "External\Hooking\Hook.h"
#include "Common\FileSystemHooks.h"
#include "Wrappers\wrapper.h"
#include "Wrappers\d3d8to9.h"
#include "Common\LoadModules.h"
#include "Common\Utils.h"
#include "Common\AutoUpdate.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

// For Logging
std::ofstream LOG;

// Variables
HMODULE m_hModule = nullptr;
SH2VERSION GameVersion = SH2V_UNKNOWN;
HMODULE wrapper_dll = nullptr;
EXECUTION_STATE esFlags = 0;
bool IsWindows10 = false;
bool CustomExeStrSet = false;
bool EnableCustomShaders = false;
bool ds_threadExit = false;
bool m_StopThreadFlag = false;			// Used for thread functions

void DelayedStart()
{
	// Get config file path
	wchar_t configpath[MAX_PATH] = {};
	if (GetModulePath(configpath, MAX_PATH) && wcsrchr(configpath, '\\'))
	{
		wchar_t t_name[MAX_PATH] = {};
		wcscpy_s(t_name, MAX_PATH - wcslen(configpath) - 1, wcsrchr(configpath, '\\') + 1);
		if (wcsrchr(configpath, '.'))
		{
			wcscpy_s(wcsrchr(t_name, '.'), MAX_PATH - wcslen(t_name), L"\0");
		}
		wcscpy_s(wcsrchr(configpath, '\\'), MAX_PATH - wcslen(configpath), L"\0");
		std::wstring name(t_name);
		std::transform(name.begin(), name.end(), name.begin(), [](wchar_t c) { return towlower(c); });
		wcscpy_s(configpath, MAX_PATH, std::wstring(std::wstring(configpath) + L"\\" + name + L".ini").c_str());
	}

	// Read config file
	char* szCfg = Read(configpath);

	// Parce config file
	bool IsLoadConfig = false;
	if (szCfg)
	{
		IsLoadConfig = true;
		Parse(szCfg, ParseCallback);
		free(szCfg);

		UpdateConfigDefaults();
	}

	// Check arguments for PID
	CheckArgumentsForPID();

	// Get log file path and open log file
	wchar_t logpath[MAX_PATH];
	wcscpy_s(logpath, MAX_PATH, configpath);
	wcscpy_s(wcsrchr(logpath, '.'), MAX_PATH - wcslen(logpath), L".log");
	Logging::EnableLogging = !DisableLogging;
	Logging::Open(logpath);

	// Starting
	Logging::Log() << "Starting Silent Hill 2 Enhancements! v" << APP_VERSION;

	// Init Logs
	Logging::LogComputerManufacturer();
	Logging::LogOSVersion();
	Logging::LogProcessNameAndPID();

	// Check if system is running Windows 10 equivalent
	if (!IsWindows10)
	{
		// Define registry keys
		HKEY RegKey = nullptr;
		DWORD dwDataMajor = 0;
		unsigned long iSize = sizeof(DWORD);

		// Get version
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &RegKey) == ERROR_SUCCESS)
		{
			if (RegQueryValueExA(RegKey, "CurrentMajorVersionNumber", nullptr, nullptr, (LPBYTE)&dwDataMajor, &iSize) == ERROR_SUCCESS)
			{
				if (dwDataMajor >= 10)
				{
					IsWindows10 = true;
					Logging::Log() << "Windows 10 equivalent found!";
				}
			}
			RegCloseKey(RegKey);
		}
	}

	// Game version
	if (memcmp((void*)0x00401005, "\xE9\x56\x25\x00\x00\xE9\x71\x25\x00\x00\xE9\xFC\x69\x00\x00\xE9\x77\x06\x00\x00", 0x14) == 0)
	{
		GameVersion = SH2V_10;
		Logging::Log() << "Game binary version: v1.0";
	}
	else if (memcmp((void*)0x00401005, "\xE9\x56\x25\x00\x00\xE9\x71\x25\x00\x00\xE9\x9C\x6A\x00\x00\xE9\x77\x06\x00\x00", 0x14) == 0)
	{
		GameVersion = SH2V_11;
		Logging::Log() << "Game binary version: v1.1";
	}
	else if (memcmp((void*)0x00401005, "\xE9\x66\x25\x00\x00\xE9\x81\x25\x00\x00\xE9\xAC\x6A\x00\x00\xE9\x77\x06\x00\x00", 0x14) == 0)
	{
		GameVersion = SH2V_DC;
		Logging::Log() << "Game binary version: Director's Cut";
	}
	else
	{
		Logging::Log() << "Warning: Unknown game binary version!";
	}

	// Remove unsupported compatibility settings
	// Needs to be before CheckAdminAccess()
	if (CheckCompatibilityMode)
	{
		RemoveCompatibilityMode();
	}
	Logging::LogCompatLayer();

	// Remove unneeded virtual store files and check for admin access
	if (CheckForAdminAccess)
	{
		RemoveVirtualStoreFiles();
		CheckAdminAccess();
	}

	// Validate binary version
	ValidateBinary();

	// Get Silent Hill 2 file path
	wchar_t sh2path[MAX_PATH];
	GetSH2FolderPath(sh2path, MAX_PATH);
	Logging::Log() << "Running from: " << sh2path;

	// Log settings in ini file
	if (IsLoadConfig)
	{
		Logging::Log() << "Config file: " << configpath;
		Logging::Log() << "|----------- SETTINGS -----------";

		// Log config settings
		szCfg = Read(configpath);
		if (szCfg)
		{
			Parse(szCfg, LogCallback);
			free(szCfg);
		}

		Logging::Log() << "|--------------------------------";
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: Config file not found, using defaults";
	}

	// Log files in folder
	LogDirectory();

	// Get wrapper mode
	Wrapper::GetWrapperMode();

	// Create wrapper
	if (!(d3d8to9 && Wrapper::dtype == DTYPE_D3D8))
	{
		// Get defined d3d8 script wrapper
		HMODULE ScriptDll = nullptr;
		if (LoadD3d8FromScriptsFolder)
		{
			ScriptDll = GetD3d8ScriptDll();
		}

		wrapper_dll = Wrapper::CreateWrapper(Wrapper::dtype == DTYPE_D3D8 ? ScriptDll : nullptr);
		if (wrapper_dll)
		{
			Logging::Log() << "Wrapper created for " << dtypename[Wrapper::dtype];
		}
		else if (Wrapper::dtype != DTYPE_ASI)
		{
			Logging::Log() << __FUNCTION__ << " Error: could not create wrapper!";
		}
	}

	// Hook Direct3D8
	if (HookDirect3D)
	{
		HookDirect3DCreate8();
	}

	// Enable d3d8to9
	if (d3d8to9)
	{
		EnableD3d8to9();
	}

	// Hook DirectSound8
	if (AudioClipDetection)
	{
		HookDirectSoundCreate8();
	}

	// Hook DirectInput8
	if (RestoreVibration)
	{
		HookDirectInput8Create();
	}

	// Hook CreateFile API when using UseCustomModFolder
	if (UseCustomModFolder)
	{
		InstallFileSystemHooks(m_hModule);
	}

	// Fix Windows Game Explorer issue
	if (DisableGameUX)
	{
		InstallCreateProcessHooks();
	}

	// Enable No-CD Patch
	if (NoCDPatch)
	{
		PatchCDCheck();
	}

	// Patch binray
	PatchBinary();

	// Update SFX addresses
	if (EnableSFXAddrHack)
	{
		PatchSFXAddr();
	}

	// Update Texture addresses
	if (EnableTexAddrHack)
	{
		PatchTexAddr();
	}

	// Sets application DPI aware which disables DPI virtulization/High DPI scaling for this process
	if (DisableHighDPIScaling)
	{
		SetDPIAware();
	}

	// Disable screensaver
	if (DisableScreenSaver)
	{
		esFlags = SetThreadExecutionState(ES_USER_PRESENT | ES_CONTINUOUS);
		if (!esFlags)
		{
			esFlags = SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
			if (esFlags)
			{
				Logging::Log() << "Disabling Screensaver...";
			}
		}
		else
		{
			Logging::Log() << "Disabling Screensaver for Windows XP...";
		}
	}

	// Fix issue with saving the gome on a drive that is larger than 2TBs
	if (ImproveStorageSupport)
	{
		Patch2TBHardDrive();
	}

	// PS2 Noise Filter
	if (PS2StyleNoiseFilter)
	{
		PatchPS2NoiseFilter();
	}

	// Draw Distance
	if (IncreaseDrawDistance)
	{
		PatchDrawDistance();
	}

	// Room Lighting Fix
	if (RoomLightingFix)
	{
		PatchRoomLighting();
	}

	// Tree Lighting fix
	if (LightingFix)
	{
		PatchTreeLighting();
	}

	// Rowboat Animation Fix
	if (RowboatAnimationFix)
	{
		PatchRowboatAnimation();
	}

	// Catacombs Meat Room
	if (CatacombsMeatRoomFix)
	{
		PatchCatacombsMeatRoom();
	}

	// Fog adjustment fixes
	if (FogParameterFix)
	{
		PatchFogParameters();
	}

	// Piston room fix
	if (PistonRoomFix)
	{
		PatchPistonRoom();
	}

	// Hotel Room 312 Shadow Flicker Fix
	if (Room312ShadowFix)
	{
		PatchRoom312ShadowFix();
	}

	// Red Cross health indicator in cutscene
	if (DisableRedCrossInCutScenes)
	{
		PatchRedCrossInCutscene();
	}

	// Adjusts flashlight brightness
	if (PS2FlashlightBrightness)
	{
		PatchPS2Flashlight();
	}

	// Fixes Lying Figure's behavior
	if (FixCreatureVehicleSpawn)
	{
		PatchCreatureVehicleSpawn();
	}

	// Prevent chainsaw spawn on first playthrough
	if (FixChainsawSpawn)
	{
		PatchPreventChainsawSpawn();
	}

	// Fix flashlight flicker
	if (FlashlightFlickerFix)
	{
		PatchFlashlightFlicker();
	}

	// Causes the Options menu to exit directly to game play
	if (PauseScreenFix)
	{
		PatchPauseScreen();
	}

	// XInput based vibration
	if (RestoreVibration)
	{
		PatchXInputVibration();
	}

	// DPad movement
	if (DPadMovementFix || RestoreSearchCamMovement != 0)
	{
		PatchControllerTweaks();
	}

	// Loads font texture form tga file
	if (UseCustomFonts)
	{
		PatchCustomFonts();
	}

	// Load exe's strings from txt file
	if (UseCustomExeStr)
	{
		CustomExeStrSet = SUCCEEDED(PatchCustomExeStr());
	}

	// Fixes mouse hitboxes in Main Menu (for 1.1 version)
	if (MainMenuFix)
	{
		PatchMainMenu();
	}

	// Loads 'start01.tex' (graphic Main Menu) according to the selected language
	if (MainMenuTitlePerLang)
	{
		PatchMainMenuTitlePerLang();
	}

	// Reenable game's special FX
	if (RestoreSpecialFX)
	{
		PatchSpecialFX();
	}

	// Changes the event at the gate near Heaven's Night to that when trying to re-enter the door to Blue Creek Apartments
	if (FixTownWestGateEvent)
	{
		PatchTownWestGateEvent();
	}

	// Disables the screen position feature in the game's options menu, which is no longer needed for modern displays
	if (LockScreenPosition)
	{
		PatchLockScreenPosition();
	}

	// Fix flashlight at end of failed clock push cutscene
	if (FixAptClockFlashlight)
	{
		PatchFlashlightClockPush();
	}

	// FixSaveBGImage
	if (FixSaveBGImage)
	{
		PatchSaveBGImage();
	}

	// Enables all advanced graphics settings from the game's options menu on game launch
	if (UseBestGraphics)
	{
		PatchBestGraphics();
	}

	// Disables changing the speaker configuration in the game's options menu
	if (LockSpeakerConfig && CustomExeStrSet)
	{
		PatchSpeakerConfigLock();
	}

	// Fog Fix
	if (FogFix)
	{
		PatchCustomFog();
	}

	// Widescreen Fix
	if (WidescreenFix)
	{
		Logging::Log() << "Loading the \"WidescreenFixesPack\" module...";
		WSFInit();
	}

	// Update full screen images
	if (FullscreenImages)
	{
		PatchFullscreenImages();
	}

	// Patch resolution list in the Options menu
	if (((DynamicResolution && WidescreenFix) || LockResolution) && CustomExeStrSet)
	{
		SetResolutionPatch();
	}

	// Check for update
	if (AutoUpdateModule)
	{
		CreateThread(nullptr, 0, CheckForUpdate, nullptr, 0, nullptr);
	}

	// Load ASI pluggins
	if (LoadPlugins && Wrapper::dtype != DTYPE_ASI)
	{
		LoadASIPlugins(LoadFromScriptsOnly);
	}

	// Set single core affinity
	if (SingleCoreAffinity && !SingleCoreAffinityTimer)
	{
		SetSingleCoreAffinity();
	}

	// Find GetModelID when a dependent fix is enabled
	if (EnableSoftShadows || SpecularFix)
	{
		FindGetModelID();
	}

	// Specular Fix
	if (SpecularFix)
	{
		PatchSpecular();
	}

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

	// Loaded
	Logging::Log() << "Silent Hill 2 Enhancements module loaded!";
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Store Module handle
		m_hModule = hModule;

		// Allows application to use Windows themes
		if (ScreenMode != 3 && WndModeBorder)
		{
			SetGUITheme(nullptr);
		}

		// Set things to load on delayed access
		SetDelayedStart();

		break;
	}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		// Stop thread
		m_StopThreadFlag = true;

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

		// Unload wrapped dll file
		if (wrapper_dll)
		{
			FreeModule(wrapper_dll);
		}
#endif // DEBUG

		// Reenabling screensaver
		if (esFlags)
		{
			Logging::Log() << "Reenabling Screensaver...";
			SetThreadExecutionState(esFlags);
		}

		// Reset screen settings
		ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, CDS_RESET, nullptr);

		// Quitting
		Logging::Log() << "Unloading Silent Hill 2 Enhancements!";

		break;
	}
	}

	return true;
}
