#pragma once

typedef enum _SH2VERSION {
	SH2V_UNKNOWN = 0,
	SH2V_10 = 1,
	SH2V_11 = 2,
	SH2V_DC = 3,
} SH2VERSION;

// Shared Function declarations
BYTE *GetChapterIDPointer();
void *GetCutsceneIDPointer();
void *GetCutscenePosPointer();
BYTE *GetFlashLightRenderPointer();
void *GetJamesPosPointer();
void *GetRoomIDPointer();
DWORD *GetSpecializedLightPointer();

// Function forward declaration
void DisableCDCheck();
void PatchBinary();
void SetWindowHandle(HWND WindowHandle);
void Update2TBHardDriveFix();
void UpdateBloodSize(DWORD *SH2_RoomID);
void UpdateCatacombsMeatRoom();
void UpdateCemeteryLighting();
void UpdateClosetCutscene(DWORD *SH2_CutsceneID, float *SH2_CutsceneCameraPos);
void UpdateCreatureVehicleSpawn();
void UpdateCustomExeStr();
void UpdateCustomFonts();
void UpdateDPadMovement();
void UpdateDrawDistance();
void UpdateDynamicDrawDistance(DWORD *SH2_RoomID);
void UpdateFogParameters();
void UpdateGameLoad(DWORD *SH2_RoomID, float *SH2_JamesPos);
void UpdateHangOnEsc(DWORD *SH2_RoomID);
void UpdateHospitalChase(DWORD *SH2_RoomID, float *SH2_JamesPos);
void UpdateHotelWater(DWORD *SH2_RoomID);
void UpdateHotelRoom312FogVolumeFix(DWORD *SH2_RoomID);
void UpdateInfiniteRumble(DWORD *SH2_RoomID);
void UpdateLightingTransition(DWORD *SH2_CutsceneID);
void UpdateMainMenuFix();
void UpdateMainMenuTitlePerLang();
void UpdatePistonRoom();
void UpdatePreventChainsawSpawn();
void UpdatePS2Flashlight();
void UpdatePS2NoiseFilter();
void UpdateRedCrossInCutscene();
void UpdateResolutionLock(DWORD ResX, DWORD ResY);
void UpdateRoom312ShadowFix();
void UpdateRowboatAnimation();
void UpdateShadowCutscene(DWORD *SH2_CutsceneID);
void UpdateSpecialFX();
void UpdateSpecialFXScale(DWORD Height);
void UpdateSFXAddr();
void UpdateTexAddr();
void UpdateTownWestGateEventFix();
void UnhookWindowHandle();
void UpdateXInputVibration();
void ValidateBinary();

// Varable forward declaration
extern SH2VERSION GameVersion;
extern void *ChapterIDAddr;
extern void *CutsceneIDAddr;
extern void *CutscenePosAddr;
extern BYTE *FlashLightRenderAddr;
extern bool IsInBloomEffect;
extern void *JamesPosAddr;
extern void *RoomIDAddr;
extern BYTE *SpecializedLightAddr;

// Run code only once
#define RUNONCE() \
	{ \
		static bool RunOnce = true; \
		if (!RunOnce) \
		{ \
			return; \
		} \
		RunOnce = false; \
	} \

#define RUNCODEONCE(funct) \
	{ \
		static bool RunFixOnce = true; \
		if (RunFixOnce) \
		{ \
			funct; \
		} \
		RunFixOnce = false; \
	} \
