#pragma once
#include <stdint.h>

typedef enum _SH2VERSION {
	SH2V_UNKNOWN = 0,
	SH2V_10 = 1,
	SH2V_11 = 2,
	SH2V_DC = 3,
} SH2VERSION;

enum class ModelID;

// Shared function declaration
DWORD GetRoomID();
DWORD GetCutsceneID();
float GetCutscenePos();
float GetCameraFOV();
float GetJamesPosX();
float GetJamesPosY();
float GetJamesPosZ();
BYTE GetFlashLightRender();
BYTE GetChapterID();
DWORD GetSpecializedLight1();
DWORD GetSpecializedLight2();
BYTE GetFlashlightSwitch();
float GetFlashlightBrightnessRed();
float GetFlashlightBrightnessGreen();
float GetFlashlightBrightnessBlue();
DWORD GetOnScreen();
BYTE GetEventIndex();
BYTE GetMenuEvent();
DWORD GetTransitionState();
BYTE GetFullscreenImageEvent();
float GetInGameCameraPosY();
BYTE GetInventoryStatus();
DWORD GetLoadingScreen();
BYTE GetPauseMenuButtonIndex();
float GetFPSCounter();
int16_t GetShootingKills();
int16_t GetMeleeKills();
float GetBoatMaxSpeed();
int8_t GetActionDifficulty();
int8_t GetRiddleDifficulty();
int8_t GetNumberOfSaves();
float GetInGameTime();
float GetWalkingDistance();
float GetRunningDistance();
int16_t GetItemsCollected();
float GetDamagePointsTaken();
uint8_t GetSecretItemsCollected();
float GetBoatStageTime();

// Shared pointer function declaration
DWORD *GetRoomIDPointer();
DWORD *GetCutsceneIDPointer();
float *GetCutscenePosPointer();
float *GetCameraFOVPointer();
float *GetJamesPosXPointer();
float *GetJamesPosYPointer();
float *GetJamesPosZPointer();
BYTE *GetFlashLightRenderPointer();
BYTE *GetChapterIDPointer();
DWORD *GetSpecializedLight1Pointer();
DWORD *GetSpecializedLight2Pointer();
BYTE *GetFlashlightSwitchPointer();
float *GetFlashlightBrightnessPointer();
DWORD *GetOnScreenPointer();
BYTE *GetEventIndexPointer();
BYTE *GetMenuEventPointer();
DWORD *GetTransitionStatePointer();
BYTE *GetFullscreenImageEventPointer();
float *GetInGameCameraPosYPointer();
BYTE *GetInventoryStatusPointer();
DWORD *GetLoadingScreenPointer();
BYTE *GetPauseMenuButtonIndexPointer();
float *GetFPSCounterPointer();
int16_t *GetShootingKillsPointer();
int16_t *GetMeleeKillsPointer();
float *GetBoatMaxSpeedPointer();
int8_t *GetActionDifficultyPointer();
int8_t *GetRiddleDifficultyPointer();
int8_t *GetNumberOfSavesPointer();
float *GetInGameTimePointer();
float *GetWalkingDistancePointer();
float *GetRunningDistancePointer();
int16_t *GetItemsCollectedPointer();
float *GetDamagePointsTakenPointer();
uint8_t *GetSecretItemsCollectedPointer();
float *GetBoatStageTimePointer();

// Function patch declaration
void CheckArgumentsForPID();
void RelaunchSilentHill2();
void CheckAdminAccess();
void RemoveVirtualStoreFiles();
void RemoveCompatibilityMode();
void SetDelayedStart();
void SetFullscreenImagesRes(DWORD Width, DWORD Height);
void SetFullscreenVideoRes(DWORD Width, DWORD Height);
void UpdateResolutionPatches(LONG Width, LONG Height);
void SetResolutionList(DWORD Width, DWORD Height);
void SetResolutionPatch();
void SetRoom312Resolution(void *WidthAddress);
void UnhookWindowHandle();
void ValidateBinary();

void Patch2TBHardDrive();
void PatchAdvancedOptions();
void PatchAlternateStomp();
void PatchBestGraphics();
void PatchBinary();
void PatchCDCheck();
void PatchCatacombsMeatRoom();
void PatchClosetSpawn();
void PatchCreatureVehicleSpawn();
void PatchCriware();
HRESULT PatchCustomExeStr();
void PatchCustomFog();
void PatchCustomFonts();
void PatchControllerTweaks();
void PatchDrawDistance();
void PatchFlashlightClockPush();
void PatchFlashlightFlicker();
void PatchFogParameters();
void PatchFullscreenImages();
void PatchFullscreenVideos();
void PatchGameLoad();
void PatchInventoryBGMBug();
void PatchLockScreenPosition();
void PatchMainMenu();
void PatchMainMenuTitlePerLang();
void PatchMemoBrightnes();
void PatchPauseScreen();
void PatchPistonRoom();
void PatchPreventChainsawSpawn();
void PatchPS2Flashlight();
void PatchPS2NoiseFilter();
void PatchRedCrossInCutscene();
void PatchRoom312ShadowFix();
void PatchRoomLighting();
void PatchRowboatAnimation();
void PatchSaveBGImage();
void PatchSpeakerConfigLock();
void PatchSpecialFX();
void PatchSpecular();
void PatchSFXAddr();
void PatchSpecificSoundLoopFix();
void PatchTexAddr();
void PatchTownWestGateEvent();
void PatchTreeLighting();
void PatchWindowIcon();
void PatchWindowTitle();
void PatchXInputVibration();
void PatchSaveGameSound();
void PatchQuickSavePos();

void FindGetModelID();
int GetCurrentMaterialIndex();
bool IsJames(ModelID id);
bool IsMariaExcludingEyes(ModelID id);
bool IsMariaEyes(ModelID id);
bool isConfirmationPromptOpen();

void OnFileLoadTex(LPCSTR lpFileName);
void OnFileLoadVid(LPCSTR lpFileName);

void RunBloodSize();
void RunClosetCutscene();
void RunClosetSpawn();
void RunDynamicDrawDistance();
void RunFlashlightClockPush();
void RunFogSpeed();
void RunGameLoad();
void RunHangOnEsc();
void RunHospitalChase();
void RunHotelRoom312FogVolumeFix();
void RunHotelWater();
void RunInfiniteRumbleFix();
void RunInnerFlashlightGlow(DWORD Height);
void RunLightingTransition();
void RunRoomLighting();
void RunRotatingMannequin();
void RunSaveBGImage();
void RunShadowCutscene();
void RunSpecialFXScale(DWORD Height);
void RunTreeColor();

// Variable forward declaration
extern SH2VERSION GameVersion;
extern bool IsInFullscreenImage;
extern bool IsInBloomEffect;
extern bool IsInFakeFadeout;
extern DWORD *RoomIDAddr;
extern DWORD *CutsceneIDAddr;
extern float *CutscenePosAddr;
extern float *CameraFOVAddr;
extern float *JamesPosXAddr;
extern float *JamesPosYAddr;
extern float *JamesPosZAddr;
extern BYTE *FlashLightRenderAddr;
extern BYTE *ChapterIDAddr;
extern DWORD *SpecializedLight1Addr;
extern DWORD *SpecializedLight2Addr;
extern BYTE *FlashlightSwitchAddr;
extern float *FlashlightBrightnessAddr;
extern DWORD *OnScreenAddr;							/* 0 = load screen
										4 = normal in-game
										5 = maps
										6 = inventory screen
										9 = save screen */

extern BYTE *EventIndexAddr;							/*0 = load screen
										1 = load room
										2 = main menu
										4 = in-game
										5 = map
										6 = inventory
										7 = options menu & FMVs (from Movie Menu)
										8 = memo list
										9 = save screen
										10/11 = game result
										12 = "COMING SOON!" splash screen
										13 = game over screen
										15 = FMVs
										16 = PC pause menu*/

extern BYTE *MenuEventAddr;							/* 7 = main menu event index */

extern DWORD *TransitionStateAddr;						/* 1 = fades the game image to black
										2 = solid black screen
										3 = fades from black back to the in game screen */

extern BYTE *FullscreenImageEventAddr;
extern float *InGameCameraPosYAddr;
extern BYTE *InventoryStatusAddr;
extern DWORD *LoadingScreenAddr;
extern int SpecularFlag;
extern bool UseFakeLight;
extern bool InSpecialLightZone;
extern bool IsInGameResults;
extern BYTE *PauseMenuButtonIndexAddr;
extern float *FPSCounterAddr;
extern int16_t *ShootingKillsAddr;
extern int16_t *MeleeKillsAddr;
extern float *BoatMaxSpeedAddr;
extern int8_t *ActionDifficultyAddr;
extern int8_t *RiddleDifficultyAddr;
extern int8_t *NumberOfSavesAddr;
extern float *InGameTimerAddr;
extern float *WalkingDistanceAddr;
extern float *RunningDistanceAddr;
extern int16_t *ItemsCollectedAddr;
extern float *DamagePointsTakenAddr;
extern uint8_t *SecretItemsCollectedAddr;
extern float *BoatStageTimeAddr;
extern bool ShowDebugOverlay;
extern bool ShowInfoOverlay;

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
