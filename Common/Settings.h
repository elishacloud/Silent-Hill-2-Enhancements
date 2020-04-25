#pragma once

#include <string>

#define VISIT_BOOL_SETTINGS(visit) \
	visit(AutoUpdateModule, true) \
	visit(CatacombsMeatRoomFix, true) \
	visit(CemeteryLightingFix, true) \
	visit(CheckForAdminAccess, true) \
	visit(ClosetCutsceneFix, true) \
	visit(CreateLocalFix, true) \
	visit(DisableScreenSaver, true) \
	visit(d3d8to9, true) \
	visit(DisableCutsceneBorders, true) \
	visit(DisableGameUX, true) \
	visit(DisableEnlargedText, true) \
	visit(DisableHighDPIScaling, true) \
	visit(DisableLogging, false) \
	visit(DisableRedCross, false) \
	visit(DisableRedCrossInCutScenes, true) \
	visit(DisableSafeMode, true) \
	visit(DPadMovementFix, true) \
	visit(EnableSFXAddrHack, true) \
	visit(EnableTexAddrHack, true) \
	visit(EnableSoftShadows, true) \
	visit(EnableWndMode, true) \
	visit(FastTransitions, true) \
	visit(Fix2D, true) \
	visit(FixChainsawSpawn, true) \
	visit(FixCreatureVehicleSpawn, true) \
	visit(FixDrawingTextLine, true) \
	visit(FixGPUAntiAliasing, true) \
	visit(FixHangOnEsc, true) \
	visit(FixMissingWallChunks, true) \
	visit(FixTownWestGateEvent, true) \
	visit(FMVWidescreenMode, true) \
	visit(fog_custom_on, true) \
	visit(Fog2DFix, true) \
	visit(FogParameterFix, true) \
	visit(FogSpeedFix, true) \
	visit(FullscreenImages, true) \
	visit(FullscreenWndMode, true) \
	visit(GamepadControlsFix, true) \
	visit(GameLoadFix, true) \
	visit(HalogenLightFix, true) \
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
	visit(RestoreSpecialFX, true) \
	visit(RestoreVibration, true) \
	visit(Room312ShadowFix, true) \
	visit(RowboatAnimationFix, true) \
	visit(SetBlackPillarBoxes, true) \
	visit(Southpaw, false) \
	visit(SteamCrashFix, true) \
	visit(UnlockJapLang, false) \
	visit(UseBestGraphics, true) \
	visit(UseCustomExeStr, true) \
	visit(UseCustomFonts, true) \
	visit(UseCustomModFolder, true) \
	visit(WhiteShaderFix, true) \
	visit(WidescreenFix, true) \
	visit(WndModeBorder, true) \
	visit(WoodsideRoom205Fix, true)

#define VISIT_INT_SETTINGS(visit) \
	visit(AntiAliasing, 0) \
	visit(AnisotropicFiltering, 1) \
	visit(CustomFontCol, 100) \
	visit(CustomFontRow, 14) \
	visit(CustomFontCharWidth, 20) \
	visit(CustomFontCharHeight, 32) \
	visit(fog_transparency_layer1, 128) \
	visit(fog_transparency_layer2, 138) \
	visit(FMVWidescreenEnhancementPackCompatibility, 2) \
	visit(LetterSpacing, 2) \
	visit(NormalFontWidth, 20) \
	visit(NormalFontHeight, 30) \
	visit(PadNumber, 0) \
	visit(ResX, 0) \
	visit(ResY, 0) \
	visit(FPSLimit, 0) \
	visit(IncreaseNoiseEffectRes, 512) \
	visit(RestoreSearchCamMovement, 1) \
	visit(SingleCoreAffinity, 2) \
	visit(SingleCoreAffinityTimer, 5000) \
	visit(SmallFontWidth, 14) \
	visit(SmallFontHeight, 24) \
	visit(SpaceSize, 7)

#define VISIT_FLOAT_SETTINGS(visit) \
	visit(fog_layer1_x1, 0.250f) \
	visit(fog_layer1_x2, 0.250f) \
	visit(fog_layer1_y1, 0.250f) \
	visit(fog_layer1_y2, 0.250f) \
	visit(fog_layer2_complexity, 0.066406f) \
	visit(fog_layer2_density_add, 100.0f) \
	visit(fog_layer2_density_mult, 1.4f)

#define VISIT_STR_SETTINGS(visit) \
	visit(CustomModFolder, "")

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

typedef void(__stdcall* NV)(char* name, char* value);

char* Read(wchar_t* szFileName);
void Parse(char* str, NV NameValueCallback);
void __stdcall ParseCallback(char* lpName, char* lpValue);
void __stdcall LogCallback(char* lpName, char* lpValue);
