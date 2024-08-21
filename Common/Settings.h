#pragma once

#include <string>

#define VISIT_BOOL_SETTINGS(visit) \
	visit(AudioClipDetection, true) \
	visit(AutoHideMouseCursor, false) \
	visit(AutoUpdateModule, true) \
	visit(BFaWAtticFix, true) \
	visit(CatacombsMeatRoomFix, true) \
	visit(CenterPuzzleCursor, true) \
	visit(ChangeClosetSpawn, true) \
	visit(CheckCompatibilityMode, true) \
	visit(CheckForAdminAccess, true) \
	visit(ClosetCutsceneFix, true) \
	visit(CommandWindowMouseFix, true) \
	visit(CreateLocalFix, true) \
	visit(CustomAdvancedOptions, true) \
	visit(d3d8to9, true) \
	visit(DelayedFadeIn, true) \
	visit(Direct3DCreate9On12, false) \
	visit(DisableEnlargedText, true) \
	visit(DisableGameUX, true) \
	visit(DisableHighDPIScaling, true) \
	visit(DisableLoadingPressReturnMessages, true) \
	visit(DisableLogging, false) \
	visit(DisableMaximizedWindowedMode, true) \
	visit(DisableRedCross, false) \
	visit(DisableRedCrossInCutScenes, true) \
	visit(DisableSafeMode, true) \
	visit(DisableScreenSaver, true) \
	visit(DisplayModeOption, true) \
	visit(DynamicResolution, true) \
	visit(EnableDebugOverlay, true) \
	visit(EnableEnhancedMouse, true) \
	visit(EnableHoldToStomp, true) \
	visit(EnableInfoOverlay, true) \
	visit(EnableLangPath, true) \
	visit(EnableMasterVolume, true) \
	visit(EnableMouseWheelSwap, true) \
	visit(EnableScreenshots, true) \
	visit(EnableSFXAddrHack, true) \
	visit(EnableSMAA, false) \
	visit(EnableSoftShadows, true) \
	visit(EnableTexAddrHack, true) \
	visit(EnableToggleSprint, true) \
	visit(EnhanceMouseCursor, true) \
	visit(FastTransitions, true) \
	visit(ForceHybridEnumeration, true) \
	visit(FireEscapeKeyFix, true) \
	visit(Fix2D, true) \
	visit(FixAdvancedOptions, true) \
	visit(FixAptClockFlashlight, true) \
	visit(FixChainsawSpawn, true) \
	visit(FixCreatureVehicleSpawn, true) \
	visit(FixDrawingTextLine, true) \
	visit(FixElevatorCursorColor, true) \
	visit(FixFinalBossRoom, true) \
	visit(FixFMVResetIssue, true) \
	visit(FixFMVSpeed, true) \
	visit(FixHangOnEsc, true) \
	visit(FixInventoryBGM, true) \
	visit(FixMemoFading, true) \
	visit(FixMissingWallChunks, true) \
	visit(FixSaveBGImage, true) \
	visit(FixTownGateEvents, true) \
	visit(FlashlightFlickerFix, true) \
	visit(FlashlightReflection, true) \
	visit(FlashlightToggleSFX, true) \
	visit(FmvSubtitlesNoiseFix, true) \
	visit(FmvSubtitlesSyncFix, true) \
	visit(FogParameterFix, true) \
	visit(FogSpeedFix, true) \
	visit(ForceTopMost, false) \
	visit(GameLoadFix, true) \
	visit(GameLoadFlashFix, true) \
	visit(GamepadControlsFix, true) \
	visit(PuzzleAlignmentFixes, true) \
	visit(HalogenLightFix, true) \
	visit(HookDirect3D, true) \
	visit(HookDirectInput, true) \
	visit(HookDirectSound, true) \
	visit(HookWndProc, true) \
	visit(HospitalChaseFix, true) \
	visit(HotelWaterFix, true) \
	visit(ImproveStorageSupport, true) \
	visit(ImprovedWaterFX, false) \
	visit(IncreaseBlood, true) \
	visit(IncreaseDrawDistance, true) \
	visit(LegacyFixGPUAntiAliasing, false) \
	visit(LightingFix, true) \
	visit(LightingTransitionFix, true) \
	visit(LoadD3d8FromScriptsFolder, false) \
	visit(LoadFromScriptsOnly, false) \
	visit(LoadModulesFromMemory, false) \
	visit(LoadPlugins, false) \
	visit(LockResolution, true) \
	visit(LockScreenPosition, true) \
	visit(LockSpeakerConfig, true) \
	visit(MainMenuFix, true) \
	visit(MainMenuInstantLoadOptions, true) \
	visit(MainMenuTitlePerLang, true) \
	visit(MemoScreenFix, true) \
	visit(MenuSoundsFix, true) \
	visit(MothDrawOrderFix, true) \
	visit(NoCDPatch, true) \
	visit(ObservationDeckFogFix, true) \
	visit(OldManCoinFix, true) \
	visit(PauseScreenFix, true) \
	visit(PistonRoomFix, true) \
	visit(PreserveSoundsOnLoad, true) \
	visit(PS2CameraSpeed, true) \
	visit(PS2FlashlightBrightness, true) \
	visit(PS2StyleNoiseFilter, true) \
	visit(QuickSaveTweaks, true) \
	visit(QuickSaveCancelFix, true) \
	visit(ReduceCutsceneFOV, true) \
	visit(RemoveEffectsFlicker, true) \
	visit(RemoveEnvironmentFlicker, true) \
	visit(RestoreBrightnessSelector, true) \
	visit(RestoreSpecialFX, true) \
	visit(RestoreVibration, true) \
	visit(Room312ShadowFix, true) \
	visit(RoomLightingFix, true) \
	visit(RowboatAnimationFix, true) \
	visit(SetBlackPillarBoxes, true) \
	visit(SetSixtyFPS, true) \
	visit(SetSwapEffectUpgradeShim, false) \
	visit(ShowerRoomFlashlightFix, true) \
	visit(Southpaw, false) \
	visit(SpecificSoundLoopFix, true) \
	visit(SpecularFix, true) \
	visit(SteamCrashFix, true) \
	visit(SwapLightHeavyAttack, true) \
	visit(TeddyBearLookFix, true) \
	visit(UnlockJapLang, true) \
	visit(UseBestGraphics, true) \
	visit(UseCustomExeStr, true) \
	visit(UseCustomFonts, true) \
	visit(UseCustomModFolder, true) \
	visit(UseDSOAL, true) \
	visit(UsePS2LowResTextures, false) \
	visit(VHSAudioFix, false) \
	visit(WaterEnhancedRender, true) \
	visit(WhiteShaderFix, true) \
	visit(WidescreenFix, true) \
	visit(WndModeBorder, true) \
	visit(WoodsideRoom205Fix, true)

#define VISIT_INT_SETTINGS(visit) \
	visit(AnisotropicFiltering, 0) \
	visit(AntiAliasing, 0) \
	visit(AudioFadeOutDelayMS, 10) \
	visit(CRTShader, 0) \
	visit(CustomFontCharHeight, 32) \
	visit(CustomFontCharWidth, 20) \
	visit(CustomFontCol, 25) \
	visit(CustomFontRow, 17) \
	visit(DisableCutsceneBorders, 3) \
	visit(DPadMovementFix, 1) \
	visit(EnableCriWareReimplementation, 1) \
	visit(fog_transparency_layer1, 128) \
	visit(fog_transparency_layer2, 112) \
	visit(FogFix, 0xFFFF) /* Overloading the old 'fog_custom_on' option */ \
	visit(FogLayerFix, 0xFFFF) /* Overloading the old 'Fog2DFix' option */ \
	visit(FrontBufferControl, 0) \
	visit(FullscreenImages, 3) \
	visit(FullscreenVideos, 3) \
	visit(IncreaseNoiseEffectRes, 768) \
	visit(LetterSpacing, 2) \
	visit(LowHealthIndicatorStyle, 2) \
	visit(NormalFontHeight, 30) \
	visit(NormalFontWidth, 20) \
	visit(PadNumber, 0) \
	visit(RemoveForceFeedbackFilter, 1) \
	visit(ReplaceButtonText, 1) \
	visit(RestoreSearchCamMovement, 2) \
	visit(ResX, 0) \
	visit(ResY, 0) \
	visit(ScaleWindowedResolution, 0)\
	visit(ScreenMode, 0xFFFF) /* Overloading the old 'EnableWndMode' and 'FullscreenWndMode' options */ \
	visit(SingleCoreAffinityLegacy, 0) \
	visit(SmallFontHeight, 24) \
	visit(SmallFontWidth, 16) \
	visit(SpaceSize, 7) \
	visit(SpeedrunMode, 0)

#define VISIT_FLOAT_SETTINGS(visit) \
	visit(fog_layer1_x1, 0.250f) \
	visit(fog_layer1_x2, 0.250f) \
	visit(fog_layer1_y1, 0.125f) \
	visit(fog_layer1_y2, 0.125f) \
	visit(fog_layer2_complexity, 0.055f) \
	visit(fog_layer2_density_add, 100.0f) \
	visit(fog_layer2_density_mult, 1.4f) \
	visit(water_spec_mult_apt_staircase, 0.035f) \
	visit(water_spec_mult_strange_area, 0.017f) \
	visit(water_spec_mult_labyrinth, 0.017f) \
	visit(water_spec_mult_hotel, 0.05f) \
	visit(water_spec_uv_mult_hotel, 0.45f)

#define VISIT_STR_SETTINGS(visit) \
	visit(CustomModFolder, "") \
	visit(WrapperType, "")

#define VISIT_LEGACY_BOOL_SETTINGS(visit) \
	visit(EnableWndMode, true) \
	visit(fog_custom_on, true) \
	visit(Fog2DFix, true) \
	visit(FullscreenWndMode, false)

#define VISIT_ALL_SETTING(visit) \
	VISIT_BOOL_SETTINGS(visit) \
	VISIT_INT_SETTINGS(visit) \
	VISIT_FLOAT_SETTINGS(visit) \
	VISIT_STR_SETTINGS(visit) \
	VISIT_LEGACY_BOOL_SETTINGS(visit)

#define VISIT_HIDDEN_SETTING(visit) \
	visit(AnisotropicFiltering) \
	visit(AntiAliasing) \
	visit(AudioFadeOutDelayMS) \
	visit(CommandWindowMouseFix) \
	visit(CustomFontCharHeight) \
	visit(CustomFontCharWidth) \
	visit(CustomFontCol) \
	visit(CustomFontRow) \
	visit(CustomModFolder) \
	visit(DelayedFadeIn) \
	visit(Direct3DCreate9On12) \
	visit(DisableLoadingPressReturnMessages) \
	visit(DisableLogging) \
	visit(DisableRedCross) \
	visit(EnableDebugOverlay) \
	visit(EnableInfoOverlay) \
	visit(EnableScreenshots) \
	visit(EnableWndMode) \
	visit(FixFMVResetIssue) \
	visit(FixElevatorCursorColor) \
	visit(fog_custom_on) \
	visit(fog_layer1_x1) \
	visit(fog_layer1_x2) \
	visit(fog_layer1_y1) \
	visit(fog_layer1_y2) \
	visit(fog_layer2_complexity) \
	visit(fog_layer2_density_add) \
	visit(fog_layer2_density_mult) \
	visit(fog_transparency_layer1) \
	visit(fog_transparency_layer2) \
	visit(Fog2DFix) \
	visit(FullscreenWndMode) \
	visit(GameLoadFlashFix) \
	visit(HookDirect3D) \
	visit(HookDirectInput) \
	visit(HookDirectSound) \
	visit(HookWndProc) \
	visit(ImprovedWaterFX) \
	visit(LetterSpacing) \
	visit(LoadModulesFromMemory) \
	visit(LockResolution) \
	visit(NormalFontHeight) \
	visit(NormalFontWidth) \
	visit(OldManCoinFix) \
	visit(QuickSaveCancelFix) \
	visit(ResX) \
	visit(ResY) \
	visit(ScaleWindowedResolution)\
	visit(ScreenMode) \
	visit(SetSwapEffectUpgradeShim) \
	visit(ShowerRoomFlashlightFix) \
	visit(SmallFontHeight) \
	visit(SmallFontWidth) \
	visit(SpaceSize) \
	visit(water_spec_mult_apt_staircase) \
	visit(water_spec_mult_strange_area) \
	visit(water_spec_mult_labyrinth) \
	visit(water_spec_mult_hotel) \
	visit(water_spec_uv_mult_hotel) \
	visit(WrapperType)

typedef enum _SCREENMODE {
	WINDOWED = 1,
	WINDOWED_FULLSCREEN = 2,
	EXCLUSIVE_FULLSCREEN = 3,
} SCREENMODE;

typedef enum _FULLSCREENMEDIA {
	DISABLE_MEDIA_CONTROL = 0,
	FIT_MEDIA = 1,
	FILL_MEDIA = 2,
	AUTO_MEDIA_CONTROL = 3,
} FULLSCREENMEDIA;

typedef enum _FRONTBUFFERCONTROL {
	AUTO_BUFFER = 0,
	BUFFER_FROM_GDI = 1,
	BUFFER_FROM_DIRECTX = 2,
} FRONTBUFFERCONTROL;

typedef enum _CRTSHADER {
	CRT_SHADER_DISABLED = 0,
	CRT_SHADER_ENABLED = 1,
	CRT_SHADER_ENABLED_CURVATURE = 2,
} CRTSHADER;

typedef enum _REMOVEFORCEFEEDBACK {
	DISABLE_FORCEFEEDBACK_CONTROL = 0,
	AUTO_REMOVE_FORCEFEEDBACK = 1,
	REMOVE_FORCEFEEDBACK = 2,
} REMOVEFORCEFEEDBACK;

typedef enum _DPADCONTROL {
	DISABLE_PDAD_CONTROL = 0,
	DPAD_MOVEMENT_MODE = 1,
	DPAD_HYBRID_MODE = 2,
	DPAD_BUTTON_MODE = 3,
} DPADCONTROL;

typedef enum _BUTTONICONSSET {
	BUTTON_ICONS_DISABLED = 0,
	BUTTON_ICONS_GENERIC = 1,
	BUTTON_ICONS_XBOX = 2,
	BUTTON_ICONS_PLAYSTATION = 3,
	BUTTON_ICONS_NINTENDO = 4,
} BUTTONICONSSET;

// Configurable setting defaults
#define DECLARE_BOOL_SETTINGS(name, unused) \
	extern bool name;

VISIT_BOOL_SETTINGS(DECLARE_BOOL_SETTINGS);

#define DECLARE_INT_SETTINGS(name, unused) \
	extern int name;

VISIT_INT_SETTINGS(DECLARE_INT_SETTINGS);

#define DECLARE_FLOAT_SETTINGS(name, unused) \
	extern float name;

VISIT_FLOAT_SETTINGS(DECLARE_FLOAT_SETTINGS);

#define DECLARE_STR_SETTINGS(name, unused) \
	extern std::string name;

VISIT_STR_SETTINGS(DECLARE_STR_SETTINGS);

typedef void(__stdcall* NV)(char* name, char* value, void* lpParam);

struct CFGDATA
{
	DWORD Width = 0;
	DWORD Height = 0;
	DWORD VolumeLevel = 15;
	DWORD HealthIndicatorOption = 1;
	DWORD DisplayModeOption = 0;
	DWORD ScaleWindowedResolutionOption = 0;
};

extern HMODULE m_hModule;
extern CFGDATA ConfigData;
extern bool CustomExeStrSet;
extern bool EnableCustomShaders;
extern bool ShadersReady;
extern bool IsUpdating;
extern bool m_StopThreadFlag;
extern bool AutoScaleImages;
extern bool AutoScaleVideos;
extern bool AutoScaleCutscenes;
extern bool EnableCRTShader;
extern bool CRTCurveShader;
extern bool CRTNonCurveShader;
extern bool EnableInputTweaks;
extern float ScaleFactor;
extern bool IsFixGPUAntiAliasingEnabled;
extern bool IsScaledResolutionEnabled;
extern bool UsingScaledResolutions;

bool SetValue(const char* value);
bool IsValidSettings(char* name, char* value);
char* Read(const wchar_t* szFileName);
void Parse(char* strParse, NV NameValueCallback, void* lpParam = nullptr);
void __stdcall ParseCallback(char* lpName, char* lpValue, void*);
void LogSettings();
void UpdateConfigDefaults();
void UpdateScaleResolution();
