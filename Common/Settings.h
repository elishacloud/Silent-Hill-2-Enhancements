#pragma once

#include <string>

#define VISIT_BOOL_SETTINGS(visit) \
	visit(AutoUpdateModule, true) \
	visit(CatacombsMeatRoomFix, true) \
	visit(CemeteryLightingFix, true) \
	visit(ClosetCutsceneFix, true) \
	visit(d3d8to9, true) \
	visit(DisableGameUX, true) \
	visit(DisableEnlargedText, true) \
	visit(DisableLogging, false) \
	visit(DisableRedCross, false) \
	visit(DisableRedCrossInCutScenes, true) \
	visit(DPadMovementFix, true) \
	visit(EnableSFXAddrHack, true) \
	visit(EnableSelfShadows, false) \
	visit(EnableSoftShadows, false) \
	visit(EnableWndMode, true) \
	visit(FixHangOnEsc, true) \
	visit(FixMissingWallChunks, true) \
	visit(Fog2DFix, true) \
	visit(FogParameterFix, true) \
	visit(FullscreenWndMode, true) \
	visit(HalogenLightFix, true) \
	visit(HospitalChaseFix, true) \
	visit(HotelWaterFix, true) \
	visit(ImproveStorageSupport, true) \
	visit(IncreaseDrawDistance, true) \
	visit(LightingTransitionFix, true) \
	visit(LoadD3d8FromScriptsFolder, false) \
	visit(LoadFromScriptsOnly, false) \
	visit(LoadModulesFromMemory, false) \
	visit(LoadPlugins, false) \
	visit(LockResolution, true) \
	visit(MainMenuFix, true) \
	visit(MainMenuTitlePerLang, true) \
	visit(Nemesis2000FogFix, true) \
	visit(NoCDPatch, true) \
	visit(PistonRoomFix, true) \
	visit(PS2FlashlightBrightness, true) \
	visit(PS2StyleNoiseFilter, true) \
	visit(RemoveEffectsFlicker, true) \
	visit(Room312ShadowFix, true) \
	visit(RowboatAnimationFix, true) \
	visit(UnlockJapLang, false) \
	visit(UseCustomExeStr, true) \
	visit(UseCustomFonts, true) \
	visit(UseCustomModFolder, true) \
	visit(WhiteShaderFix, true) \
	visit(WidescreenFix, true) \
	visit(WndModeBorder, true) \
	visit(XInputVibration, true)

#define VISIT_INT_SETTINGS(visit) \
	visit(CustomFontCol, 100) \
	visit(CustomFontRow, 14) \
	visit(CustomFontCharWidth, 20) \
	visit(CustomFontCharHeight, 32) \
	visit(NormalFontWidth, 20) \
	visit(NormalFontHeight, 30) \
	visit(PadNumber, 0) \
	visit(SingleCoreAffinity, 1) \
	visit(SingleCoreAffinityTimer, 5000) \
	visit(SmallFontWidth, 14) \
	visit(SmallFontHeight, 24)

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

extern bool WidescreenFixLoaded;

typedef void(__stdcall* NV)(char* name, char* value);

char* Read(wchar_t* szFileName);
void Parse(char* str, NV NameValueCallback);
void __stdcall ParseCallback(char* lpName, char* lpValue);
