#pragma once

// Function forward declaration
void *GetRoomIDPointer();
void *GetCutsceneIDPointer();
void *GetCutscenePosPointer();
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
void UpdateLightingTransition();
void UpdateHotelWater(DWORD *SH2_RoomID);
void UpdateRoom312ShadowFix(DWORD *SH2_RoomID);
void UpdateClosetCutscene(DWORD *SH2_CutsceneID, float *SH2_CutsceneCameraPos);
void UpdateHospitalChase(DWORD *SH2_RoomID);
void UpdateDynamicDrawDistance(DWORD *SH2_RoomID);
void UpdateHangOnEsc();
void UpdateXInputVibration();
void UpdateDPadMovement();
void UpdateInfiniteRumble(DWORD *SH2_RoomID);
void SetWindowHandle(HWND WindowHandle);
void UnhookWindowHandle();
void UpdateCustomExeStr();
void UpdateResolutionLock();
void UpdateMainMenuFix();
void UpdateMainMenuTitlePerLang();

// Varable forward declaration
extern void *RoomIDAddr;
extern void *CutsceneIDAddr;
extern void *CutscenePosAddr;
