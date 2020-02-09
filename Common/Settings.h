#pragma once

#include <string>

#define VISIT_BOOL_SETTINGS(visit) \
	visit(AutoUpdateModule, true) \
	visit(CatacombsMeatRoomFix, true) \
	visit(CemeteryLightingFix, true) \
	visit(CheckForAdminAccess, true) \
	visit(ClosetCutsceneFix, true) \
	visit(CreateLocalFix, true) \
	visit(d3d8to9, true) \
	visit(DisableCutsceneBorders, true) \
	visit(DisableGameUX, true) \
	visit(DisableEnlargedText, true) \
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
	visit(FixHangOnEsc, true) \
	visit(FixMissingWallChunks, true) \
	visit(FixTownWestGateEvent, true) \
	visit(FMVWidescreenMode, true) \
	visit(FMVWidescreenEnhancementPackCompatibility, true) \
	visit(Fog2DFix, true) \
	visit(FogParameterFix, true) \
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
	visit(Nemesis2000FogFix, true) \
	visit(NoCDPatch, true) \
	visit(PistonRoomFix, true) \
	visit(PS2CameraSpeed, true) \
	visit(PS2FlashlightBrightness, true) \
	visit(PS2StyleNoiseFilter, true) \
	visit(ReduceCutsceneFOV, true) \
	visit(RemoveEffectsFlicker, true) \
	visit(RemoveEnvironmentFlicker, true) \
	visit(RestoreSpecialFX, true) \
	visit(RestoreVibration, true) \
	visit(Room312PauseScreenFix, true) \
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
	visit(AnisotropicFiltering, 1) \
	visit(CustomFontCol, 100) \
	visit(CustomFontRow, 14) \
	visit(CustomFontCharWidth, 20) \
	visit(CustomFontCharHeight, 32) \
	visit(LetterSpacing, 2) \
	visit(NormalFontWidth, 20) \
	visit(NormalFontHeight, 30) \
	visit(PadNumber, 0) \
	visit(ResX, 0) \
	visit(ResY, 0) \
	visit(FPSLimit, 0) \
	visit(IncreaseNoiseEffectRes, 512) \
	visit(RestoreSearchCamMovement, 1) \
	visit(SingleCoreAffinity, 1) \
	visit(SingleCoreAffinityTimer, 5000) \
	visit(SmallFontWidth, 14) \
	visit(SmallFontHeight, 24) \
	visit(SpaceSize, 7)

#define VISIT_STR_SETTINGS(visit) \
	visit(CustomModFolder, "")

// Configurable setting defaults
#define DECLARE_BOOL_SETTINGS(name, unused) \
	extern bool name;

VISIT_BOOL_SETTINGS(DECLARE_BOOL_SETTINGS);

#define DECLARE_INT_SETTINGS(name, unused) \
	extern int name;

VISIT_INT_SETTINGS(DECLARE_INT_SETTINGS);

#define DECLARE_STR_SETTINGS(name, unused) \
	extern std::string name;

VISIT_STR_SETTINGS(DECLARE_STR_SETTINGS);

typedef void(__stdcall* NV)(char* name, char* value);

char* Read(wchar_t* szFileName);
void Parse(char* str, NV NameValueCallback);
void __stdcall ParseCallback(char* lpName, char* lpValue);
