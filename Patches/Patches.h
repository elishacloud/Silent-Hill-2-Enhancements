#pragma once

// Function forward declaration
void *GetRoomIDPointer();
void *GetCutsceneIDPointer();
void *GetCutscenePosPointer();
void *GetJamesPosPointer();
BYTE *GetFlashLightRenderPointer();
void DisableCDCheck();
void UpdateSFXAddr();
void UpdatePS2NoiseFilter();
void UpdateDrawDistance();
void UpdateCemeteryLighting();
void UpdateRowboatAnimation();
void UpdateCatacombsMeatRoom();
void UpdateRedCrossInCutscene();
void UpdateFogParameters();
void UpdateCustomFonts();
void UpdatePistonRoom();
void Update2TBHardDriveFix();
void UpdatePS2Flashlight();
void UpdateLightingTransition(DWORD *SH2_CutsceneID);
void UpdateHotelWater(DWORD *SH2_RoomID);
void UpdateRoom312ShadowFix();
void UpdateClosetCutscene(DWORD *SH2_CutsceneID, float *SH2_CutsceneCameraPos);
void UpdateHospitalChase(DWORD *SH2_RoomID, float *SH2_JamesPos);
void UpdateDynamicDrawDistance(DWORD *SH2_RoomID);
void UpdateHangOnEsc();
void UpdateXInputVibration();
void UpdateDPadMovement();
void UpdateInfiniteRumble(DWORD *SH2_RoomID);
void SetWindowHandle(HWND WindowHandle);
void UnhookWindowHandle();
void UpdateCustomExeStr();
void UpdateResolutionLock(DWORD ResX, DWORD ResY);
void UpdateMainMenuFix();
void UpdateMainMenuTitlePerLang();
void UpdateGameLoad(DWORD *SH2_RoomID, float *SH2_JamesPos);
void UpdateBloodSize(DWORD *SH2_RoomID);

// Varable forward declaration
extern void *RoomIDAddr;
extern void *CutsceneIDAddr;
extern void *CutscenePosAddr;
extern void *JamesPosAddr;
extern BYTE *FlashLightRenderAddr;
