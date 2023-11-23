#pragma once
#include <stdint.h>
#include <string>

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
BYTE GetEventIndex();
BYTE GetMenuEvent();
DWORD GetTransitionState();
BYTE GetFullscreenImageEvent();
float GetInGameCameraPosX();
float GetInGameCameraPosY();
float GetInGameCameraPosZ();
BYTE GetInventoryStatus();
DWORD GetLoadingScreen();
BYTE GetPauseMenuButtonIndex();
BYTE GetPauseMenuQuitIndex();
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
int32_t GetMouseVerticalPosition();
int32_t GetMouseHorizontalPosition();
BYTE GetSearchViewFlag();
int32_t GetEnableInput();
BYTE GetAnalogX();
BYTE GetControlType();
BYTE GetRunOption();
BYTE GetNumKeysWeaponBindStart();
BYTE GetTalkShowHostState();
BYTE GetBoatFlag();
int32_t GetIsWritingQuicksave();
int32_t GetTextAddr();
float GetFrametime();
BYTE GetInputAssignmentFlag();
BYTE GetQuitSubmenuFlag();
int32_t GetMemoListIndex();
int32_t GetMemoListHitbox();
int16_t GetMemoCountIndex();
int32_t GetMemoInventory();
BYTE GetMousePointerVisibleFlag();
BYTE GetReadingMemoFlag();
float GetGlobalFadeHoldValue();
float GetPuzzleCursorHorizontalPos();
float GetPuzzleCursorVerticalPos();
BYTE GetPlayerIsDying();
BYTE GetMariaNpcIsDying();
int8_t GetOptionsPage();
int8_t GetOptionsSubPage();
int32_t GetInternalVerticalRes();
int32_t GetInternalHorizontalRes();
int16_t GetSelectedOption();
bool IsHardwareSoundEnabled();
BYTE GetSFXVolume();
BYTE GetWorldColorR();
BYTE GetWorldColorG();
BYTE GetWorldColorB();

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
BYTE *GetEventIndexPointer();
BYTE *GetMenuEventPointer();
DWORD *GetTransitionStatePointer();
BYTE *GetFullscreenImageEventPointer();
float* GetInGameCameraPosXPointer();
float* GetInGameCameraPosYPointer();
float* GetInGameCameraPosZPointer();
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
int32_t *GetMouseVerticalPositionPointer();
int32_t *GetMouseHorizontalPositionPointer();
DWORD *GetLeftAnalogXFunctionPointer();
DWORD *GetLeftAnalogYFunctionPointer();
DWORD *GetRightAnalogXFunctionPointer();
DWORD *GetRightAnalogYFunctionPointer();
DWORD *GetUpdateMousePositionFunctionPointer();
BYTE *GetSearchViewFlagPointer();
int32_t *GetEnableInputPointer();
BYTE *GetAnalogXPointer();
BYTE *GetControlTypePointer();
BYTE *GetRunOptionPointer();
BYTE *GetNumKeysWeaponBindStartPointer();
BYTE *GetTalkShowHostStatePointer();
BYTE *GetBoatFlagPointer();
BYTE *GetRunOptionPointer();
int32_t *GetIsWritingQuicksavePointer();
int32_t *GetTextAddrPointer();
float *GetWaterAnimationSpeedPointer();
int16_t *GetFlashlightOnSpeedPointer();
float *GetLowHealthIndicatorFlashSpeedPointer();
float *GetStaircaseFlamesLightingPointer();
float *GetWaterLevelLoweringStepsPointer();
float *GetWaterLevelRisingStepsPointer();
float *GetBugRoomFlashlightFixPointer();
uint8_t *GetSixtyFPSFMVFixPointer();
uint8_t *GetGrabDamagePointer();
float *GetFrametimePointer();
DWORD *GetMeatLockerFogFixOnePointer();
DWORD *GetMeatLockerHangerFixOnePointer();
DWORD *GetMeatLockerFogFixTwoPointer();
DWORD *GetMeatLockerHangerFixTwoPointer();
BYTE *GetClearTextPointer();
float *GetMeetingMariaCutsceneFogCounterOnePointer();
float *GetMeetingMariaCutsceneFogCounterTwoPointer();
float *GetRPTClosetCutsceneMannequinDespawnPointer();
float *GetRPTClosetCutsceneBlurredBarsDespawnPointer();
BYTE *GetInputAssignmentFlagPointer();
BYTE *GetPauseMenuQuitIndexPointer();
BYTE *GetQuitSubmenuFlagPointer();
int32_t *GetMemoListIndexPointer();
int32_t *GetMemoListHitboxPointer();
int16_t *GetMemoCountIndexPointer();
int32_t *GetMemoInventoryPointer();
BYTE *GetMousePointerVisibleFlagPointer();
BYTE *GetReadingMemoFlagPointer();
DWORD *GetDrawCursorPointer();
DWORD *GetSetShowCursorPointer();
BYTE* GetInputAssignmentFlagPointer();
float* GetGlobalFadeHoldValuePointer();
float* GetFinalBossBottomWalkwaySpawnPointer();
float* GetFinalBossBottomFloorSpawnPointer();
float* GetFinalBossBlackBoxSpawnPointer();
float* GetFinalBossDrawDistancePointer();
DWORD* GetCanSaveFunctionPointer();
float* GetPuzzleCursorHorizontalPosPointer();
float* GetPuzzleCursorVerticalPosPointer();
BYTE* GetPlayerIsDyingPointer();
BYTE* GetMariaNpcIsDyingPointer();
DWORD* GetDrawOptionsFunPointer();
BYTE* GetSpkOptionTextOnePointer();
BYTE* GetSpkOptionTextTwoPointer();
int8_t* GetOptionsPagePointer();
int32_t* GetInternalVerticalResPointer();
DWORD* GetConfirmOptionsOnePointer();
DWORD* GetConfirmOptionsTwoPointer();
BYTE* GetRenderOptionsRightArrowFunPointer();
BYTE* GetStartOfOptionSpeakerPointer();
BYTE* GetDecrementMasterVolumePointer();
BYTE* GetIncrementMasterVolumePointer();
BYTE* GetOptionsRightArrowHitboxPointer();
BYTE* GetCheckForChangedOptionsPointer();
DWORD* GetPlaySoundFunPointer();
BYTE* GetDiscardOptionBOPointer();
BYTE* GetDiscardOptionPointer();
DWORD* GetDeltaTimeFunctionPointer();
bool* GetHardwareSoundEnabledPointer();
BYTE* SFXVolumePointer();
BYTE* WorldColorRPointer();
BYTE* WorldColorGPointer();
BYTE* WorldColorBPointer();

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
void PatchBestGraphics();
void PatchBinary();
void PatchCDCheck();
void PatchCatacombsMeatRoom();
void PatchClosetSpawn();
void PatchCommandWindowMouseFix();
void PatchCreatureVehicleSpawn();
void PatchCriware();
HRESULT PatchCustomExeStr();
void PatchCustomFog();
void PatchCustomFonts();
void PatchCustomSFXs();
void PatchControllerTweaks();
void PatchDelayedFadeIn();
void PatchDoubleFootstepFix();
void PatchDrawDistance();
void PatchFinalBossRoom();
void PatchFireEscapeKey();
void PatchFlashlightClockPush();
void PatchFlashlightFlicker();
void PatchFMV();
void PatchFMVFramerate();
void PatchFmvSubtitlesNoiseFix();
void PatchFmvSubtitlesSyncFix();
void PatchFogParameters();
void PatchFullscreenImages();
void PatchFullscreenVideos();
void PatchGameLoad();
void PatchHoldDamage();
void PatchHoldToStomp();
void PatchInputTweaks();
void PatchInventoryBGMBug();
void PatchLockScreenPosition();
void PatchMainMenu();
void PatchMainMenuTitlePerLang();
void PatchMapTranscription();
void PatchMasterVolumeSlider();
void PatchMemoBrightnes();
void PatchMenuSounds();
void PatchMothDrawOrder();
void PatchPauseScreen();
void PatchPistonRoom();
void PatchPreventChainsawSpawn();
void PatchPrisonerTimer();
void PatchPS2Flashlight();
void PatchPS2NoiseFilter();
void PatchQuickSaveTweaks();
void PatchQuickSaveCancelFix();
void PatchRedCrossInCutscene();
void PatchRoom312ShadowFix();
void PatchRoomLighting();
void PatchRowboatAnimation();
void PatchSaveBGImage();
void PatchSpeakerConfigLock();
void PatchSpecialFX();
void PatchSpecular();
void PatchSprayEffect();
void PatchSFXAddr();
void PatchSixtyFPS();
void PatchSpeakerConfigText();
void PatchSpecificSoundLoopFix();
void PatchSwapLightHeavyAttack();
void PatchTexAddr();
void PatchTownWestGateEvent();
void PatchTreeLighting();
void PatchVHSAudio();
void PatchWindowIcon();
void PatchWindowTitle();
void PatchXInputVibration();

void FindGetModelID();
int GetCurrentMaterialIndex();
bool IsJames(ModelID id);
bool IsMariaExcludingEyes(ModelID id);
bool IsMariaEyes(ModelID id);
bool isConfirmationPromptOpen();
int CountCollectedMemos();
bool IsInFullScreenImageEvent();
bool IsInMainOptionsMenu();
bool IsInOptionsMenu();

void HandleMenuSounds();
void SetNewVolume();

void OnFileLoadTex(LPCSTR lpFileName);
void OnFileLoadVid(LPCSTR lpFileName);

void RunBloodSize();
void RunClosetCutscene();
void RunClosetSpawn();
void RunDynamicDrawDistance();
void RunFinalBossRoomFix();
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
void RunPlayAdditionalSounds();
void RunQuickSaveTweaks();
void RunQuickSaveCancelFix();
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

extern BYTE *EventIndexAddr;			/*0 = load screen
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

extern BYTE *MenuEventAddr;				/* 7 = main menu event index */

extern DWORD *TransitionStateAddr;		/* 1 = fades the game image to black
										2 = solid black screen
										3 = fades from black back to the in game screen */

extern BYTE *FullscreenImageEventAddr;
extern float* InGameCameraPosXAddr;
extern float* InGameCameraPosYAddr;
extern float* InGameCameraPosZAddr;
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
extern int32_t* MouseVerticalPositionAddr;
extern int32_t* MouseHorizontalPositionAddr;
extern DWORD* LeftAnalogXFunctionAddr;
extern DWORD* LeftAnalogYFunctionAddr;
extern DWORD* RightAnalogXFunctionAddr;
extern DWORD* RightAnalogYFunctionAddr;
extern DWORD* UpdateMousePositionFunctionAddr;
extern BYTE* SearchViewFlagAddr;
extern int32_t* EnableInputAddr;
extern BYTE* AnalogXAddr;
extern BYTE* ControlTypeAddr;
extern BYTE* NumKeysWeaponBindStartAddr;
extern BYTE* TalkShowHostStateAddr;
extern BYTE* InputAssignmentFlagAddr;
extern float* PuzzleCursorHorizontalPosAddr;
extern float* PuzzleCursorVerticalPosAddr;
extern DWORD* GetDeltaTimeFunctionAddr;

extern bool ShowDebugOverlay;
extern bool ShowInfoOverlay;
extern std::string AuxDebugOvlString;
extern bool IsControllerConnected;
extern HWND GameWindowHandle;

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
