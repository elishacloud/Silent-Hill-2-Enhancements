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

#define DLLMAIN_CPP
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sstream>
#include <shlwapi.h>
#include "winmm.h"
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
#include "Resource.h"

// For Logging
std::ofstream LOG;

// Variables
HMODULE m_hModule = nullptr;
SH2VERSION GameVersion = SH2V_UNKNOWN;
HMODULE wrapper_dll = nullptr;
HANDLE g_hMutex = nullptr;
EXECUTION_STATE esFlags = 0;
bool CustomExeStrSet = false;
bool EnableCustomShaders = false;
bool IsUpdating = false;
bool m_StopThreadFlag = false;			// Used for thread functions
bool IsLoadConfig = false;

// Paths
wchar_t configpath[MAX_PATH] = {};

extern "C" BOOL WINAPI DSOAL_DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);

void VerifySH2EE(){}

std::wstring CreateUniqueMutexName()
{
	std::wstringstream ss;
	ss << L"SH2EECoreModule_" << GetCurrentProcessId();
	return ss.str();
}

void GetConfig()
{
	// Get config file path
	GetConfigName(configpath, MAX_PATH, L".ini");

	// Check if config file does not exist
	if (!PathFileExists(configpath))
	{
		ExtractFileFromResource(IDR_SETTINGS_INI, configpath);
	}

	// Read config file
	char* szCfg = Read(configpath);

	// Parce config file
	if (szCfg)
	{
		IsLoadConfig = true;
		Parse(szCfg, ParseCallback);
		free(szCfg);
	}

	LockConfigs();

	UpdateConfigDefaults();
}

void StartLogging()
{
	// Get log file path
	wchar_t logpath[MAX_PATH];
	GetConfigName(logpath, MAX_PATH, L".log");

	// Open log file
	Logging::EnableLogging = !DisableLogging;
	Logging::Open(logpath);

	// Starting
	Logging::Log() << "Starting Silent Hill 2 Enhancements! v" << APP_VERSION;
}

void GetGameVersion()
{
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
}

void DelayedStart()
{
	// Only allow function to run once
	static bool AlreadyRun = false;
	if (AlreadyRun)
	{
		return;
	}
	AlreadyRun = true;

	// Init Logs
	Logging::LogComputerManufacturer();
	Logging::LogOSVersion();
	Logging::LogProcessNameAndPID();

#ifndef SPEEDRUN_VERSION
	if (SpeedrunMode != 0)
	{
		Logging::Log() << __FUNCTION__ << " Error: SpeedrunMode is enabled, but this SH2:EE version doesn't support it. Please download a Speedrun Version of the SH2:EE.";
	}
#endif // !SPEEDRUN_VERSION

	// Check arguments for PID
	CheckArgumentsForPID();

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

	// Get config data settings
	GetConfigData();

	// Log settings in ini file
	if (IsLoadConfig)
	{
		Logging::Log() << "Config file: " << configpath;
		Logging::Log() << "|----------- SETTINGS -----------";

		LogSettings();

		Logging::Log() << "|--------------------------------";
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: Config file not found, using defaults";
	}

	// Log files in folder
	LogDirectory();

	// Replace window title
	PatchWindowTitle();

	// Fix window icon
	PatchWindowIcon();

	// Get wrapper mode
	Wrapper::GetWrapperMode();

	// Get script dll
	HMODULE ScriptDll = nullptr;
	if (LoadD3d8FromScriptsFolder && !d3d8to9)
	{
		ScriptDll = GetD3d8ScriptDll();
	}

	// Create wrapper
	if (Wrapper::dtype != DTYPE_ASI && (Wrapper::dtype != DTYPE_D3D8 || !d3d8to9))
	{
		wrapper_dll = Wrapper::CreateWrapper((Wrapper::dtype == DTYPE_D3D8) ? ScriptDll : nullptr);
		if (wrapper_dll)
		{
			Logging::Log() << "Wrapper created for " << dtypename[Wrapper::dtype];
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Error: could not create wrapper!";
		}
	}

	// Hook Direct3D8
	if (HookDirect3D)
	{
		HookDirect3DCreate8(ScriptDll);
	}

	// Enable d3d8to9
	if (d3d8to9)
	{
		EnableD3d8to9();
	}

	// Hook DirectSound8
	if (HookDirectSound)
	{
		HookDirectSoundCreate8();

		// Check if DSOAL alsoft.ini file does not exist and add it
		if (UseDSOAL)
		{
			wchar_t alsoftpath[MAX_PATH] = {};
			if (GetModuleFileNameW(nullptr, alsoftpath, MAX_PATH) != NULL)
			{
				wchar_t* pdest = wcsrchr(alsoftpath, '\\');
				if (pdest)
				{
					*(pdest + 1) = '\0';
					if (wcscat_s(alsoftpath, MAX_PATH, L"alsoft.ini") == 0)
					{
						if (!PathFileExists(alsoftpath))
						{
							ExtractFileFromResource(IDR_ALSOFT_INI, alsoftpath);
						}
					}
				}
			}
		}
	}

	// Hook DirectInput8
	if (HookDirectInput)
	{
		HookDirectInput8Create();
	}

	// XInput based vibration
	if (RestoreVibration)
	{
		PatchXInputVibration();
	}

	// Widescreen Fix (needs to be before 'UseCustomModFolder')
	if (WidescreenFix)
	{
		Logging::Log() << "Loading the \"WidescreenFixesPack\" module...";
		WSFInit();
	}

	// Hook CreateFile APIs (needs to be before all other patches that check files in 'data' or 'sh2e' folders)
	if (UseCustomModFolder)
	{
		InstallFileSystemHooks();
	}

	// Enable No-CD Patch
	if (NoCDPatch)
	{
		PatchCDCheck();
	}

	// Patch binary
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

	// Change James' spawn point after the cutscene ends
	if (ChangeClosetSpawn)
	{
		PatchClosetSpawn();
	}

	// Applies patches to fix 60fps specific bugs
	if (SetSixtyFPS)
	{
		PatchSixtyFPS();
	}

	// Fix flashlight flicker
	if (FlashlightFlickerFix)
	{
		PatchFlashlightFlicker();
	}

	// Fix Fog issues in FMVs
	if (RemoveEnvironmentFlicker)
	{
		PatchFMV();
	}

	// Fix memo brightness
	if (FixMemoFading)
	{
		PatchMemoBrightnes();
	}

	// Causes the Options menu to exit directly to game play
	if (PauseScreenFix)
	{
		PatchPauseScreen();
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

	// Fix puzzle alignment issues
	if (PuzzleAlignmentFixes)
	{
		PatchPuzzleAlignmentFixes();
	}

	// Reenable game's special FX
	if (RestoreSpecialFX)
	{
		PatchSpecialFX();
	}

	// Enable holding attack to stomp an enemy
	if (EnableHoldToStomp)
	{
		PatchHoldToStomp();
	}

	// Removes the lock on the Wood Side Apartments gate after entering, and updates commentary on the gates near Heaven's Night and Wood Side Apartments at night
	if (FixTownGateEvents)
	{
		PatchTownGateEvents();
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

	// Fixes crash when loading Game Results
	if (GameLoadFix)
	{
		PatchGameLoad();
	}

	// Fixes momentarily "flash" when save file is loaded
	if (GameLoadFlashFix)
	{
		PatchGameLoadFlashFix();
	}

	// Fixes quick save text position and the text fading too quickly bug
	if (QuickSaveTweaks)
	{
		PatchQuickSaveTweaks();
	}

	// Patches quick saving to write .sys and .dat files on the same frame
	if (QuickSaveCancelFix)
	{
		PatchQuickSaveCancelFix();
	}

	// Fixes missing sounds in menus
	if (MenuSoundsFix)
	{
		PatchMenuSounds();
	}

	// Restores uncensored audio for the VHS fmv
	if (VHSAudioFix)
	{
		PatchVHSAudio();
	}

	// Fixes an issue where the game would play the wrong background music when pulling up the inventory screen under certain circumstances
	if (FixInventoryBGM)
	{
		PatchInventoryBGMBug();
	}

	// Fixes at chainsaw and final Marry boss's mouth attack.
	if (SpecificSoundLoopFix)
	{
		PatchSpecificSoundLoopFix();
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

	// Fog Fix
	if (FogFix)
	{
		PatchCustomFog();
	}

	// Update fullscreen images
	if (FullscreenImages)
	{
		PatchFullscreenImages();
	}

	// Update fullscreen videos
	if (FullscreenVideos)
	{
		PatchFullscreenVideos();
	}

	// Patch resolution list in the Options menu
	if (((DynamicResolution || LockResolution) && WidescreenFix) && CustomExeStrSet)
	{
		SetResolutionPatch();
	}

	// Fixes issues in the Advanced Options screen
	if (FixAdvancedOptions)
	{
		PatchAdvancedOptions();
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

	// Creates a reflection of the flashlight on glass and glossy surfaces throughout the game.
	if (FlashlightReflection)
	{
		PatchFlashlightReflection();
	}

	// Make water prettier in various areas
	if (WaterEnhancedRender)
	{
		PatchWaterEnhancement();
		PatchWaterDrawOrderFix();
	}

	// Enables a complete rewrite of the game's audio engine
	if (EnableCriWareReimplementation)
	{
		PatchCriware();
	}

	// Makes the FMVs play with consistent speed
	if (FixFMVSpeed)
	{
		PatchFMVFramerate();
	}

	// Patch delayed fade-in to hide animation artifacts
	if (DelayedFadeIn)
	{
		PatchDelayedFadeIn();
	}

	// Patch FMV subtitles to draw on top of noise grain
	if (FmvSubtitlesNoiseFix)
	{
		PatchFmvSubtitlesNoiseFix();
	}

	// Patch timing of subtitles to match the FMV framerate
	if (FmvSubtitlesSyncFix)
	{
		PatchFmvSubtitlesSyncFix();
	}

	// Patch draw order for moth objects.
	if (MothDrawOrderFix)
	{
		PatchMothDrawOrder();
	}

	// Patch mouse support for the inventory command window
	if (CommandWindowMouseFix)
	{
		PatchCommandWindowMouseFix();
	}

	// Patch to fix the final boss room culling error
	if (FixFinalBossRoom)
	{
		PatchFinalBossRoom();
	}

	// Hook input handling
	if (EnableInputTweaks)
	{
		PatchInputTweaks();
	}

	// Patch master volume slider and strings
	if (EnableMasterVolume)
	{
		if (!CustomExeStrSet)
		{
			EnableMasterVolume = false;
		}
		else
		{
			EnableMasterVolume = (
				SUCCEEDED(PatchSpeakerConfigText()) &&
				SUCCEEDED(PatchMasterVolumeSlider())
			);
		}
	}

	// Disables changing the speaker configuration in the game's options menu
	if (LockSpeakerConfig && !EnableMasterVolume)
	{
		PatchSpeakerConfigLock();
	}

	// Patch swap light and heavy melee attacks
	if (SwapLightHeavyAttack)
	{
		PatchSwapLightHeavyAttack();
	}
	
	// Add custom sfx
	if (PreserveSoundsOnLoad)
	{
		PatchCustomSFXs();
	}

	// Adds display mode option
	if (DisplayModeOption)
	{
		PatchDisplayMode();
	}

	// Red Cross health indicator in cutscene
	if (CustomAdvancedOptions || DisableRedCrossInCutScenes || DisableRedCross)
	{
		PatchRedCrossInCutscene();
	}

	// Patch to prevent James from looking at the teddy bear after picking up the bent needle
	if (TeddyBearLookFix)
	{
		PatchTeddyBearLookFix();
	}

	// Overhaul the Control Options menu
	if (ReplaceButtonText != BUTTON_ICONS_DISABLED)
	{
		PatchControlOptionsMenu();
	}
	
	// Fix spawn precondition for the Old Man Coin
	if (OldManCoinFix)
 	{
		PatchOldManCoinFix();
 	}

	// Fix shadow anomalies
	if (BFaWAtticFix)
	{
		PatchAtticShadows();
	}

	// Fix fog scale to fill the background of the Observation Deck
	if (ObservationDeckFogFix)
	{
		PatchObservationDeckFogFix();
 	}

	// Set Low Health Indicator Style
	if (LowHealthIndicatorStyle == 2)
	{
		PatchLowHealthIndicator();
	}

	// Patch boat speed with great knife equipped, and disable weapon hotkeys
	if (RowboatAnimationFix)
	{
		PatchGreatKnifeBoatSpeed();
	}

	// Fix flashlight position during the hospital shower room cutscene
	if (ShowerRoomFlashlightFix)
	{
		PatchShowerRoomFlashlightFix();
	}
	
	// Hotel Employee Elevator Cursor Color Bug Fix
	if (FixElevatorCursorColor)
	{
		PatchElevatorCursorColor();
	}

	// Allow "Load" and "Continue" options on the main menu to appear instantly
	if (MainMenuInstantLoadOptions)
	{
		PatchMainMenuInstantLoadOptions();
	}

	// Replace Advanced Options menu with custom options
	if (CustomAdvancedOptions && CustomExeStrSet)
	{
		PatchCustomAdvancedOptions();
	}
	
	// Remove the "Now loading..." and "Press Return to continue." messages
	if (DisableLoadingPressReturnMessages)
	{
		switch (GameVersion)
		{
		case SH2V_10:
			UpdateMemoryAddress((void*)0x00497356, "\x90\x90\x90\x90\x90", 5);
			UpdateMemoryAddress((void*)0x0044740B, "\x90\x90\x90\x90\x90", 5);
			UpdateMemoryAddress((void*)0x00406CE5, "\x90\x90\x90\x90\x90", 5);
			break;
		case SH2V_11:
			UpdateMemoryAddress((void*)0x00497606, "\x90\x90\x90\x90\x90", 5);
			UpdateMemoryAddress((void*)0x004475AB, "\x90\x90\x90\x90\x90", 5);
			UpdateMemoryAddress((void*)0x00406CE5, "\x90\x90\x90\x90\x90", 5);
			break;
		case SH2V_DC:
			UpdateMemoryAddress((void*)0x004475AB, "\x90\x90\x90\x90\x90", 5);
			UpdateMemoryAddress((void*)0x00406CF5, "\x90\x90\x90\x90\x90", 5);
			break;
		}
	}

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

	// Set timer
	timeBeginPeriod(1);

	// Loaded
	Logging::Log() << "Silent Hill 2 Enhancements module loaded!";
}

// Dll main function
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Clear the error code
		SetLastError(ERROR_SUCCESS);

		// Create a unique mutex name using the process ID
		std::wstring mutexName = CreateUniqueMutexName();
		g_hMutex = CreateMutex(nullptr, TRUE, mutexName.c_str());

		// Check if the mutex already exists
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// Clear the error code
			SetLastError(ERROR_SUCCESS);

			// Mutex already exists, another instance of the DLL is loaded in this process
			g_hMutex = nullptr;
			return TRUE; // Return TRUE, needs to still load but not do anything
		}

		// Store Module handle
		m_hModule = hModule;

		// Get configuration
		GetConfig();

		// Start logging
		StartLogging();

		// Get game version
		GetGameVersion();

		// Sets application DPI aware which disables DPI virtulization/High DPI scaling for this process
		if (DisableHighDPIScaling)
		{
			SetDPIAware();
		}

		// Allows application to use Windows themes
		if (WndModeBorder)
		{
			SetAppTheme();
		}

		// Fix Windows Game Explorer issue
		if (DisableGameUX)
		{
			InstallCreateProcessHooks();
		}

		// Set single core affinity
		if (SingleCoreAffinityLegacy)
		{
			SetSingleCoreAffinity();
		}

		// Set things to load on delayed access
		if (GetModuleHandleA("d3dx.dll") || !SetDelayedStart())
		{
			DelayedStart();
		}
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		// Release the mutex when the DLL is unloaded
		if (g_hMutex)
		{
			ReleaseMutex(g_hMutex);
			CloseHandle(g_hMutex);
			g_hMutex = nullptr;
		}

		// Stop thread
		m_StopThreadFlag = true;

		// Unhook window handle
		UnhookWindowHandle();

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
	}
	break;
	}

	if (UseDSOAL)
	{
		DSOAL_DllMain(hModule, fdwReason, lpReserved);
	}

	return TRUE;
}
