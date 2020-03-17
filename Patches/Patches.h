#pragma once

typedef enum _SH2VERSION {
	SH2V_UNKNOWN = 0,
	SH2V_10 = 1,
	SH2V_11 = 2,
	SH2V_DC = 3,
} SH2VERSION;

// Shared Function declarations
BYTE *GetChapterIDPointer();
DWORD *GetCutsceneIDPointer();
float *GetCutscenePosPointer();
float *GetFlashlightBrightnessPointer();
BYTE *GetFlashLightRenderPointer();
BYTE *GetFlashlightSwitchPointer();
float *GetJamesPosPointer();
DWORD *GetOnScreenPointer();
BYTE *GetPauseMenuPointer();
DWORD *GetRoomIDPointer();
DWORD *GetSpecializedLightPointer1();
DWORD *GetSpecializedLightPointer2();
DWORD *GetTransitionStatePointer();

// Function forward declaration
void CheckArgumentsForPID();
void DisableCDCheck();
void PatchBinary();
void SetCustomFogFix();
void SetDefaultFullscreenBackground();
void SetWindowHandle(HWND WindowHandle);
void Update2TBHardDriveFix();
void UpdateAdminAccess();
void UpdateBestGraphics();
void UpdateBloodSize(DWORD *SH2_RoomID);
void UpdateCatacombsMeatRoom();
void UpdateCemeteryLighting();
void UpdateClosetCutscene(DWORD *SH2_CutsceneID, float *SH2_CutsceneCameraPos);
void UpdateCreatureVehicleSpawn();
void UpdateCustomExeStr();
void UpdateCustomFonts();
void UpdateControllerTweaks();
void UpdateDrawDistance();
void UpdateDynamicDrawDistance(DWORD *SH2_RoomID);
void UpdateFogParameters();
void UpdateGameLoad(DWORD *SH2_RoomID, float *SH2_JamesPos);
void UpdateHangOnEsc(DWORD *SH2_RoomID);
void UpdateHospitalChase(DWORD *SH2_RoomID, float *SH2_JamesPos);
void UpdateHotelWater(DWORD *SH2_RoomID);
void UpdateHotelRoom312FogVolumeFix(DWORD *SH2_RoomID);
void UpdateInfiniteRumble(DWORD *SH2_RoomID);
void UpdateInnerFlashlightGlow(DWORD Height);
void UpdateLightingTransition(DWORD *SH2_CutsceneID);
void UpdateLockScreenPosition();
void UpdateMainMenuFix();
void UpdateMainMenuTitlePerLang();
void UpdatePistonRoom();
void UpdatePreventChainsawSpawn();
void UpdatePS2Flashlight();
void UpdatePS2NoiseFilter();
void UpdateRedCrossInCutscene();
void UpdateResolutionLock(DWORD ResX, DWORD ResY);
void UpdateRoom312ResolutionFix(void *WidthAddress);
void UpdateRoom312ShadowFix();
void UpdateRotatingMannequin(DWORD *SH2_RoomID);
void UpdateRowboatAnimation();
void UpdateShadowCutscene(DWORD *SH2_CutsceneID);
void UpdateSpecialFX();
void UpdateSpecialFXScale(DWORD Height);
void UpdateSFXAddr();
void UpdateTexAddr();
void UpdateTownWestGateEventFix();
void UpdateTreeColor(DWORD *SH2_CutsceneID);
void UpdateTreeLighting();
void UnhookWindowHandle();
void UpdateXInputVibration();
void ValidateBinary();

// Variable forward declaration
extern SH2VERSION GameVersion;
extern BYTE *ChapterIDAddr;
extern DWORD *CutsceneIDAddr;
extern float *CutscenePosAddr;
extern float *FlashlightBrightnessAddr;
extern BYTE *FlashLightRenderAddr;
extern BYTE *FlashlightSwitchAddr;
extern bool IsInBloomEffect;
extern bool IsInFakeFadeout;
extern float *JamesPosAddr;
extern DWORD *OnScreenAddr;				// 0 = load screen, 4 = normal in-game, 5 = maps, 6 = inventory screen, 9 = save screen
extern BYTE *PauseMenuAddr;
extern DWORD *RoomIDAddr;
extern DWORD *SpecializedLightAddr1;
extern DWORD *SpecializedLightAddr2;
extern DWORD *TransitionStateAddr;		// 1 = fades the game image to black, 2 = solid black screen, 3 = fades from black back to the in game screen

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
