#pragma once

#define VISIT_BOOL_SETTINGS(visit) \
	visit(AutoUpdateModule, true) \
	visit(CatacombsMeatRoomFix, true) \
	visit(CemeteryLightingFix, true) \
	visit(d3d8to9, true) \
	visit(DisableRedCross, false) \
	visit(DisableRedCrossInCutScenes, true) \
	visit(EnableSFXAddrHack, true) \
	visit(EnableWndMode, false) \
	visit(FixMissingWallChunks, true) \
	visit(Fog2DFix, true) \
	visit(FullscreenWndMode, false) \
	visit(HalogenLightFix, true) \
	visit(IncreaseDrawDistance, true) \
	visit(LoadFromScriptsOnly, false) \
	visit(LoadPlugins, false) \
	visit(Nemesis2000FogFix, true) \
	visit(NoCDPatch, true) \
	visit(PS2StyleNoiseFilter, true) \
	visit(ResetScreenRes, true) \
	visit(RowboatAnimationFix, true) \
	visit(SingleCoreAffinity, true) \
	visit(UseCustomModFolder, true) \
	visit(WhiteShaderFix, true) \
	visit(WidescreenFix, true) \
	visit(WndModeBorder, true)

// Configurable setting defaults
#define DECLARE_BOOL_SETTINGS(name, unused) \
	extern bool name;

VISIT_BOOL_SETTINGS(DECLARE_BOOL_SETTINGS);

typedef void(__stdcall* NV)(char* name, char* value);

char* Read(wchar_t* szFileName);
void Parse(char* str, NV NameValueCallback);
void __stdcall ParseCallback(char* lpName, char* lpValue);
