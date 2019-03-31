#pragma once

#define VISIT_BOOL_SETTINGS(visit) \
	visit(AutoUpdateModule, true) \
	visit(CatacombsMeatRoomFix, true) \
	visit(CemeteryLightingFix, true) \
	visit(ClosetCutsceneFix, true) \
	visit(d3d8to9, true) \
	visit(DisableLogging, false) \
	visit(DisableRedCross, false) \
	visit(DisableRedCrossInCutScenes, true) \
	visit(EnableSFXAddrHack, true) \
	visit(EnableSelfShadows, false) \
	visit(EnableWndMode, true) \
	visit(FixHangOnEsc, true) \
	visit(FixMissingWallChunks, true) \
	visit(Fog2DFix, true) \
	visit(FogParameterFix, true) \
	visit(FullscreenWndMode, true) \
	visit(HalogenLightFix, true) \
	visit(HotelWaterFix, true) \
	visit(ImproveStorageSupport, true) \
	visit(IncreaseDrawDistance, true) \
	visit(LightingTransitionFix, true) \
	visit(LoadFromScriptsOnly, false) \
	visit(LoadPlugins, false) \
	visit(Nemesis2000FogFix, true) \
	visit(NoCDPatch, true) \
	visit(PistonRoomFix, true) \
	visit(PS2FlashlightBrightness, true) \
	visit(PS2StyleNoiseFilter, true) \
	visit(ResetScreenRes, true) \
	visit(Room312ShadowFix, true) \
	visit(RowboatAnimationFix, true) \
	visit(SingleCoreAffinity, true) \
	visit(UseCustomModFolder, true) \
	visit(WhiteShaderFix, true) \
	visit(WidescreenFix, true) \
	visit(WndModeBorder, true) \
	visit(UseCustomFonts, true) \
	visit(DisableEnlargedText, true) \
	visit(XInputVibration, true) \
	visit(UseCustomExeStr, true) \
	visit(UnlockJapLang, false)

#define VISIT_INT_SETTINGS(visit) \
	visit(CustomFontCol, 100) \
	visit(CustomFontRow, 14) \
	visit(CustomFontCharWidth, 20) \
	visit(CustomFontCharHeight, 32) \
	visit(NormalFontWidth, 20) \
	visit(NormalFontHeight, 30) \
	visit(SmallFontWidth, 14) \
	visit(SmallFontHeight, 24) \
	visit(PadNumber, 0)

// Configurable setting defaults
#define DECLARE_BOOL_SETTINGS(name, unused) \
	extern bool name;

VISIT_BOOL_SETTINGS(DECLARE_BOOL_SETTINGS);

#define DECLARE_INT_SETTINGS(name, unused) \
	extern int name;

VISIT_INT_SETTINGS(DECLARE_INT_SETTINGS);

typedef void(__stdcall* NV)(char* name, char* value);

char* Read(wchar_t* szFileName);
void Parse(char* str, NV NameValueCallback);
void __stdcall ParseCallback(char* lpName, char* lpValue);
