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
void UpdateHotelWater(DWORD *SH2_RoomID);
void UpdateRoom312ShadowFix(DWORD *SH2_RoomID);
void UpdateClosetCutscene(DWORD *SH2_CutsceneID, float *SH2_CutsceneCameraPos);
void UpdateXInputVibration();

// Varable forward declaration
extern void *RoomIDAddr;
extern void *CutsceneIDAddr;
extern void *CutscenePosAddr;
