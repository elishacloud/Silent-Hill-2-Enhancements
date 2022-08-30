#pragma once

#include <string>

#define VISIT_BOOL_SETTINGS(visit) \
	visit(AdjustColorTemp, true) \
	visit(AudioClipDetection, true) \
	visit(AutoUpdateModule, true) \
	visit(CatacombsMeatRoomFix, true) \
	visit(ChangeClosetSpawn, true) \
	visit(CheckCompatibilityMode, true) \
	visit(CheckForAdminAccess, true) \
	visit(ClosetCutsceneFix, true) \
	visit(CreateLocalFix, true) \
	visit(d3d8to9, true) \
	visit(DisableCutsceneBorders, true) \
	visit(DisableEnlargedText, true) \
	visit(DisableGameUX, true) \
	visit(DisableHighDPIScaling, true) \
	visit(DisableLogging, false) \
	visit(DisableRedCross, false) \
	visit(DisableRedCrossInCutScenes, true) \
	visit(DisableSafeMode, true) \
	visit(DisableScreenSaver, true) \
	visit(DPadMovementFix, true) \
	visit(DynamicResolution, true) \
	visit(EnableScreenshots, true) \
	visit(EnableSFXAddrHack, true) \
	visit(EnableSMAA, false) \
	visit(EnableSoftShadows, true) \
	visit(EnableTexAddrHack, true) \
	visit(FastTransitions, true) \
	visit(Fix2D, true) \
	visit(FixAdvancedOptions, true) \
	visit(FixAptClockFlashlight, true) \
	visit(FixChainsawSpawn, true) \
	visit(FixCreatureVehicleSpawn, true) \
	visit(FixDrawingTextLine, true) \
	visit(FixFMVResetIssue, true) \
	visit(FixGPUAntiAliasing, true) \
	visit(FixHangOnEsc, true) \
	visit(FixInventoryBGM, true) \
	visit(FixMemoFading, true) \
	visit(FixMissingWallChunks, true) \
	visit(FixSaveBGImage, true) \
	visit(FixTownWestGateEvent, true) \
	visit(FlashlightFlickerFix, true) \
	visit(FogParameterFix, true) \
	visit(FogSpeedFix, true) \
	visit(GameLoadFix, true) \
	visit(GamepadControlsFix, true) \
	visit(HalogenLightFix, true) \
	visit(HookDirect3D, true) \
	visit(HookDirectSound, true) \
	visit(HospitalChaseFix, true) \
	visit(HotelWaterFix, true) \
	visit(ImproveStorageSupport, true) \
	visit(IncreaseBlood, true) \
	visit(IncreaseDrawDistance, true) \
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
	visit(MainMenuTitlePerLang, true) \
	visit(NoCDPatch, true) \
	visit(PauseScreenFix, true) \
	visit(PistonRoomFix, true) \
	visit(PS2CameraSpeed, true) \
	visit(PS2FlashlightBrightness, true) \
	visit(PS2StyleNoiseFilter, true) \
	visit(ReduceCutsceneFOV, true) \
	visit(RemoveEffectsFlicker, true) \
	visit(RemoveEnvironmentFlicker, true) \
	visit(RestoreAlternateStomp, true) \
	visit(RestoreBrightnessSelector, true) \
	visit(RestoreSpecialFX, true) \
	visit(RestoreVibration, true) \
	visit(Room312ShadowFix, true) \
	visit(RoomLightingFix, true) \
	visit(RowboatAnimationFix, true) \
	visit(SaveGameSoundFix, true) \
	visit(SetBlackPillarBoxes, true) \
	visit(Southpaw, false) \
	visit(SpecificSoundLoopFix, true) \
	visit(SpecularFix, true) \
	visit(SteamCrashFix, true) \
	visit(UnlockJapLang, false) \
	visit(UseBestGraphics, true) \
	visit(UseCustomExeStr, true) \
	visit(UseCustomFonts, true) \
	visit(UseCustomModFolder, true) \
	visit(UsePS2LowResTextures, false) \
	visit(WhiteShaderFix, true) \
	visit(WidescreenFix, true) \
	visit(WndModeBorder, true) \
	visit(WoodsideRoom205Fix, true) \
	visit(EnableDebugOverlay, false)

#define VISIT_INT_SETTINGS(visit) \
	visit(AnisotropicFiltering, 0) \
	visit(AntiAliasing, 0) \
	visit(AudioFadeOutDelayMS, 10) \
	visit(CustomFontCharHeight, 32) \
	visit(CustomFontCharWidth, 20) \
	visit(CustomFontCol, 100) \
	visit(CustomFontRow, 14) \
	visit(EnableCriWareReimplementation, 1) \
	visit(fog_transparency_layer1, 128) \
	visit(fog_transparency_layer2, 112) \
	visit(FogFix, 0xFFFF) /* Overloading the old 'fog_custom_on' option */ \
	visit(FogLayerFix, 0xFFFF) /* Overloading the old 'Fog2DFix' option */ \
	visit(FPSLimit, 0) \
	visit(FrontBufferControl, 0) \
	visit(FullscreenImages, 3) \
	visit(FullscreenVideos, 3) \
	visit(IncreaseNoiseEffectRes, 768) \
	visit(LetterSpacing, 2) \
	visit(NormalFontHeight, 30) \
	visit(NormalFontWidth, 20) \
	visit(PadNumber, 0) \
	visit(RestoreSearchCamMovement, 1) \
	visit(ResX, 0) \
	visit(ResY, 0) \
	visit(ScreenMode, 0xFFFF) /* Overloading the old 'EnableWndMode' and 'FullscreenWndMode' options */ \
	visit(SingleCoreAffinityLegacy, 0) \
	visit(SmallFontHeight, 24) \
	visit(SmallFontWidth, 16) \
	visit(SpaceSize, 7)

#define VISIT_FLOAT_SETTINGS(visit) \
	visit(fog_layer1_x1, 0.250f) \
	visit(fog_layer1_x2, 0.250f) \
	visit(fog_layer1_y1, 0.125f) \
	visit(fog_layer1_y2, 0.125f) \
	visit(fog_layer2_complexity, 0.055f) \
	visit(fog_layer2_density_add, 100.0f) \
	visit(fog_layer2_density_mult, 1.4f)

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
	visit(CustomFontCharHeight) \
	visit(CustomFontCharWidth) \
	visit(CustomFontCol) \
	visit(CustomFontRow) \
	visit(CustomModFolder) \
	visit(DisableLogging) \
	visit(EnableScreenshots) \
	visit(EnableWndMode) \
	visit(FixFMVResetIssue) \
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
	visit(FPSLimit) \
	visit(FullscreenWndMode) \
	visit(HookDirect3D) \
	visit(HookDirectSound) \
	visit(LetterSpacing) \
	visit(LoadModulesFromMemory) \
	visit(LockResolution) \
	visit(NormalFontHeight) \
	visit(NormalFontWidth) \
	visit(ResX) \
	visit(ResY) \
	visit(SmallFontHeight) \
	visit(SmallFontWidth) \
	visit(SpaceSize) \
	visit(WrapperType) \
	visit(EnableDebugOverlay)

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

extern HMODULE m_hModule;
extern bool CustomExeStrSet;
extern bool EnableCustomShaders;
extern bool IsUpdating;
extern bool m_StopThreadFlag;
extern bool AutoScaleImages;
extern bool AutoScaleVideos;

bool SetValue(const char* value);
bool IsValidSettings(char* name, char* value);
char* Read(const wchar_t* szFileName);
void Parse(char* str, NV NameValueCallback, void* lpParam = nullptr);
void __stdcall ParseCallback(char* lpName, char* lpValue, void*);
void __stdcall LogCallback(char* lpName, char* lpValue, void*);
void UpdateConfigDefaults();
