#pragma once
#include <stdint.h>
#include <string>

typedef enum _SH2VERSION {
	SH2V_UNKNOWN = 0,
	SH2V_10 = 1,
	SH2V_11 = 2,
	SH2V_DC = 3,
} SH2VERSION;

typedef enum _CHAPTERID {
	CHAPTER_MAIN_SCENARIO = 0,
	CHAPTER_BORN_FROM_A_WISH = 1,
} CHAPTERID;

typedef enum _EVENTINDEX {
	EVENT_LOAD_SCR = 0,
	EVENT_LOAD_ROOM = 1,
	EVENT_MAIN_MENU = 2,
	EVENT_IN_GAME = 4,
	EVENT_MAP = 5,
	EVENT_INVENTORY = 6,
	EVENT_OPTIONS_FMV = 7,
	EVENT_MEMO_LIST = 8,
	EVENT_SAVE_SCREEN = 9,
	EVENT_GAME_RESULT_10 = 10,
	EVENT_GAME_RESULT_11 = 11,
	EVENT_COMING_SOON_SPASH_SCR = 12,
	EVENT_GAME_OVER_SCR = 13,
	EVENT_GAME_FMV = 15,
	EVENT_PAUSE_MENU = 16,
} EVENTINDEX;

typedef enum _MENUEVENT {
	MENU_MAIN_MENU = 7,
	MENU_IN_GAME = 13,
	MENU_OPTIONS_FMV = 14,
	MENU_LOAD_SCR = 17,
} MENUEVENT;

typedef enum _FADESTATE {
	FADE_NONE = 0,
	FADE_TO_BLACK = 1,
	FADE_BLACK_SRC = 2,
	FADE_FROM_BLACK = 3,
} FADESTATE;

typedef enum _CUTSCENEID {
	CS_NONE = 0x00,
	CS_INTRO_BATHROOM = 0x01,
	CS_INTRO_OBSV_DECK = 0x02,
	CS_ANGELA_CEMETERY = 0x03,
	CS_ANGELA_CEMETERY_EXTRA = 0x04,
	CS_TUNNEL_RADIO = 0x06,
	CS_TUNNEL_KILL = 0x07,
	CS_TUNNEL_LEAVE = 0x08,
	CS_APT_HALLWAY_KEY = 0x09,
	CS_APT_TV_CORPSE = 0x0A,
	CS_APT_HOLE_KEY = 0x0B,
	CS_APT_CLOCK = 0x0C,
	CS_APT_MEET_EDDIE = 0x0D,
	CS_APT_RPT_CLOSET = 0x0E,
	CS_APT_JUICE_CHUTE = 0x0F,
	CS_APT_CROSSOVER = 0x10,
	CS_APT_TOILET_FISH = 0x11,
	CS_APT_ANGELA = 0x12,
	CS_APT_RPT_FIGHT = 0x13,
	CS_LAURA_WALL = 0x14,
	CS_MEET_MARIA_1 = 0x15,
	CS_MEET_MARIA_2 = 0x16,
	CS_MARIA_HOTEL_WRONG_WAY = 0x17,
	CS_BOWL_MARIA = 0x18,
	CS_BOWL_LAURA_EDDIE = 0x19,
	CS_BOWL_JAMES_EDDIE = 0x1A,
	CS_BOWL_MARIA_JAMES = 0x1B,
	CS_MARIA_BOWL_WRONG_WAY = 0x1C,
	CS_MARIA_BAR_ALLEY = 0x1D,
	CS_BAR_LOCKED = 0x1E,
	CS_MARIA_BAR_WRONG_WAY = 0x1F,
	CS_HSP_LAURA_ENTERS = 0x20,
	CS_HSP_TEDDY_BEAR = 0x21,
	CS_HSP_MARIA_BED = 0x22,
	CS_HSP_ROOF_FALL = 0x23,
	CS_HSP_ROOF_RECOVER = 0x24,
	CS_HSP_KEY_DRAIN = 0x25,
	CS_MARIA_HOSPITAL_WRONG_WAY = 0x26,
	CS_HSP_LAURA_TEDDY = 0x27,
	CS_HSP_LAURA_LETTER = 0x28,
	CS_HSP_LAURA_TRICK = 0x29,
	CS_HSP_BOSS_FINISH = 0x2A,
	CS_HSP_ALT_TRANSITION = 0x2B,
	CS_HSP_ALT_SHELF_PUSH = 0x2C,
	CS_HSP_ALT_MARIA_BASEMENT = 0x2D,
	CS_HSP_ALT_RADIO_QUIZ = 0x2E,
	CS_HSP_ALT_FRIDGE = 0x2F,
	CS_HSP_ALT_RPT_CHASE_1 = 0x30,
	CS_HSP_ALT_RPT_CHASE_2 = 0x31,
	CS_HSP_ALT_RPT_CHASE_3 = 0x32,
	CS_HSP_ALT_LAURA_LEAVES = 0x33,
	CS_STATUE_DIG = 0x34,
	CS_HOLE_JUMP_1 = 0x35,
	CS_PRIS_WELL = 0x36,
	CS_HOLE_JUMP_2 = 0x37,
	CS_PS_EDDIE = 0x38,
	CS_HOLE_JUMP_3 = 0x39,
	CS_HOLE_JUMP_3_RECOVER = 0x3A,
	CS_HOLE_JUMP_4 = 0x3B,
	CS_HOLE_JUMP_4_RECOVER = 0x3C,
	CS_HOLE_JUMP_5 = 0x3D,
	CS_HOLE_JUMP_5_RECOVER = 0x3E,
	CS_PS_MARIA_CELL_1 = 0x3F,
	CS_PS_MARIA_CELL_2 = 0x40,
	CS_PS_WIRE_CUT = 0x41,
	CS_PS_ANGELA_SCREAM = 0x42,
	CS_PS_ANGELA_BOSS_START = 0x43,
	CS_PS_ANGELA_BOSS_FINISH = 0x44,
	CS_PS_NOOSE_PULL = 0x45,
	CS_PS_HANDLE_TURN = 0x46,
	CS_PS_MARIA_CELL_DEAD = 0x47,
	CS_EDI_BOSS_1_INTRO = 0x48,
	CS_EDI_BOSS_1_LEAVE = 0x49,
	CS_EDI_BOSS_2_INTRO = 0x4A,
	CS_EDI_BOSS_2_LEAVE = 0x4B,
	CS_ROWBOAT_ENTER = 0x4C,
	CS_ROWBOAT_EXIT = 0x4D,
	CS_HTL_LAURA_PIANO = 0x4E,
	CS_HTL_CAN_OPEN = 0x4F,
	CS_HTL_MUSIC_BOX = 0x50,
	CS_HTL_VHS_TAPE_INSERT = 0x51,
	CS_HTL_VHS_WATCH_AFTER = 0x52,
	CS_HTL_ALT_ANGELA_FIRE = 0x53,
	CS_HTL_ALT_RPT_BOSS_INTRO = 0x54,
	CS_HTL_ALT_RPT_BOSS_FINISH = 0x55,
	CS_HTL_ALT_HEADPHONES = 0x56,
	CS_FINAL_BOSS_FINISH = 0x57,
	CS_END_LEAVE_PROLOGUE = 0x58,
	CS_END_LEAVE_EPILOGUE = 0x59,
	CS_END_LEAVE_LETTER = 0x5A,
	CS_END_MARIA_PROLUGE = 0x5B,
	CS_END_MARIA_EPILOGUE = 0x5C,
	CS_END_MARIA_LETTER = 0x5D,
	CS_END_WATER_PROLOGUE = 0x5E,
	CS_END_WATER_EPILOGUE = 0x5F,
	CS_END_REBIRTH_PROLOGUE = 0x60,
	CS_END_REBIRTH_EPILOGUE = 0x61,
	CS_END_DOG = 0x62,
	CS_BFAW_INTRO = 0x63,
	CS_BFAW_MEET_ERNEST = 0x64,
	CS_BFAW_CHECK_ERNEST = 0x65,
	CS_BFAW_ERNEST_CARD = 0x66,
	CS_BFAW_ERNEST_MIRACLE = 0x67,
	CS_BFAW_DELIVER_LIQUID = 0x68,
	CS_END_BFAW = 0x69,
} CUTSCENEID;

typedef enum _ROOMID {
	R_NONE = 0x00,
	R_BEGIN_BATHROOM = 0x01,
	R_OBSV_DECK = 0x02,
	R_FOREST_CEMETERY = 0x03,
	R_TOWN_EAST = 0x04,
	R_MOTORHOME = 0x05,
	R_BAR_NEELYS = 0x06,
	R_APT_E_COURTYARD = 0x07,
	R_TOWN_WEST = 0x08,
	R_BOWL_ENTRANCE = 0x09,
	R_BOWL_BACKROOM = 0x0A,
	R_BOWL_MAIN = 0x0B,
	R_HEAVENS_NIGHT_BACK = 0x0C,
	R_HEAVENS_NIGHT_FRONT = 0x0D,
	R_TOWN_LAKE = 0x0E,
	R_APT_E_STAIRCASE_N = 0x0F,
	R_APT_E_STAIRCASE_SW = 0x10,
	R_APT_E_STAIRCASE_SE = 0x11,
	R_APT_E_HALLWAY_1F = 0x12,
	R_APT_E_RM_307 = 0x13,
	R_APT_E_RM_104 = 0x14,
	R_APT_E_HALLWAY_2F = 0x15,
	R_APT_E_RM_202 = 0x16,
	R_APT_E_RM_205 = 0x17,
	R_APT_E_RM_208 = 0x18,
	R_APT_E_RM_209 = 0x19,
	R_APT_E_RM_210 = 0x1A,
	R_APT_E_RM_301 = 0x1B,
	R_APT_E_RM_303 = 0x1C,
	R_APT_E_RM_101_1 = 0x1D,
	R_APT_E_RM_101_2 = 0x1E,
	R_APT_E_HALLWAY_3F = 0x1F,
	R_APT_W_STAIRCASE_S = 0x20,
	R_APT_W_STAIRCASE_N = 0x21,
	R_APT_W_RM_105 = 0x22,
	R_APT_W_RM_109_1 = 0x23,
	R_APT_W_RM_109_2 = 0x24,
	R_APT_W_HALLWAY_1F = 0x25,
	R_APT_W_HALLWAY_2F = 0x26,
	R_APT_W_RM_203 = 0x27,
	R_APT_W_RM_208_209 = 0x28,
	R_HSP_E_STAIRCASE = 0x29,
	R_HSP_ELEVATOR = 0x2A,
	R_HSP_ROOF = 0x2B,
	R_HSP_RECEPTION_RM = 0x2C,
	R_HSP_DOC_RM = 0x2D,
	R_HSP_DR_LOUNGE = 0x2E,
	R_HSP_EXAMINING_1_RM = 0x2F,
	R_HSP_EXAMINING_2_RM = 0x30,
	R_HSP_RM_C2 = 0x31,
	R_HSP_RM_C3 = 0x32,
	R_HSP_LOBBY = 0x33,
	R_HSP_W_HALL_1F = 0x34,
	R_HSP_LOCKER_WOMENS = 0x35,
	R_HSP_LOCKER_MENS = 0x36,
	R_HSP_EXAMINING_3_RM = 0x37,
	R_HSP_RM_M2 = 0x38,
	R_HSP_RM_M3 = 0x39,
	R_HSP_RM_MA = 0x3A,
	R_HSP_E_HALL_2F = 0x3B,
	R_HSP_W_HALL_2F = 0x3C,
	R_HSP_SPC_TREAT_ROOM = 0x3E,
	R_HSP_PADDED_ROOM = 0x3F,
	R_HSP_SHOWER = 0x40,
	R_HSP_RM_S3 = 0x41,
	R_HSP_RM_S11 = 0x42,
	R_HSP_RM_S14 = 0x43,
	R_HSP_E_HALL_3F = 0x44,
	R_HSP_W_HALL_3F = 0x45,
	R_HSP_ALT_ELEVATOR = 0x46,
	R_HSP_ALT_E_STAIRCASE = 0x47,
	R_HSP_ALT_W_STAIRCASE = 0x48,
	R_HSP_ALT_GARDEN = 0x49,
	R_HSP_ALT_DIR_ROOM = 0x4A,
	R_HSP_ALT_RM_C1 = 0x4B,
	R_HSP_ALT_RM_C2 = 0x4C,
	R_HSP_ALT_LOBBY = 0x4D,
	R_HSP_ALT_W_HALL_1F = 0x4E,
	R_HSP_ALT_GARDEN_HALL = 0x4F,
	R_HSP_ALT_W_HALL_2F = 0x50,
	R_HSP_ALT_RM_M4 = 0x51,
	R_HSP_ALT_RM_M6 = 0x52,
	R_HSP_ALT_DAY_ROOM = 0x53,
	R_HSP_ALT_STORE_ROOM = 0x54,
	R_HSP_ALT_RM_S3 = 0x55,
	R_HSP_ALT_RM_S11 = 0x56,
	R_HSP_ALT_W_HALL_3F = 0x57,
	R_HSP_ALT_E_HALL_3F = 0x58,
	R_HSP_ALT_BASEMENT = 0x59,
	R_HSP_ALT_BASEMENT_BASEMENT = 0x5A,
	R_HSP_ALT_RPT_HALLWAY = 0x5B,
	R_HIST_SOC_LOBBY = 0x5C,
	R_HIST_SOC_BACKROOM = 0x5D,
	R_STRANGE_AREA_1_A = 0x5E,
	R_STRANGE_AREA_1_B = 0x5F,
	R_STRANGE_AREA_1_C = 0x60,
	R_HOLE_RM_1 = 0x61,
	R_STRANGE_AREA_2_A = 0x62,
	R_STRANGE_AREA_2_B = 0x63,
	R_HOLE_RM_2 = 0x64,
	R_BUG_RM = 0x65,
	R_PS_VISITATION_HALL = 0x66,
	R_PS_W_HALLWAY = 0x67,
	R_PS_S_CELLS = 0x68,
	R_PS_N_CELLS = 0x69,
	R_PS_SHOWERS = 0x6A,
	R_PS_E_HALLWAY = 0x6B,
	R_PS_CAFETERIA = 0x6C,
	R_PS_GUARD_RM = 0x6D,
	R_PS_ARMORY = 0x6E,
	R_PS_VISITATION_S = 0x6F,
	R_PS_VISITATION_N = 0x70,
	R_PS_STOREROOM = 0x71,
	R_PS_YARD = 0x72,
	R_PS_VISITATION_RESTROOMS = 0x73,
	R_PS_HALLWAY_BF = 0x74,
	R_PS_MORGUE = 0x75,
	R_PS_MORGUE_HOLE = 0x76,
	R_HOLE_RM_5 = 0x77,
	R_PS_ELEVATOR = 0x78,
	R_LAB_TOP_G = 0x79,
	R_LAB_BOTTOM_H = 0x7A,
	R_LAB_TOP_F = 0x7B,
	R_LAB_BOTTOM_G = 0x7C,
	R_LAB_TOP_E = 0x7D,
	R_LAB_BOTTOM_F = 0x7E,
	R_LAB_TOP_D = 0x7F,
	R_LAB_BOTTOM_E = 0x80,
	R_LAB_TOP_C = 0x81,
	R_LAB_BOTTOM_C = 0x82,
	R_LAB_BOTTOM_A = 0x83,
	R_LAB_TOP_A = 0x84,
	R_LAB_BOTTOM_I = 0x85,
	R_LAB_TOP_I = 0x86,
	R_LAB_TOP_B = 0x87,
	R_LAB_BOTTOM_B = 0x88,
	R_LAB_BOTTOM_D = 0x89,
	R_LAB_TOP_H = 0x8A,
	R_LAB_TOP_J = 0x8B,
	R_LAB_TOP_K = 0x8C,
	R_LAB_CATACOMBS = 0x8D,
	R_EDI_BOSS_HALL = 0x8E,
	R_EDI_BOSS_RM_1 = 0x8F,
	R_EDI_BOSS_RM_2 = 0x90,
	R_HTL_EMPLOYEE_STAIRS = 0x91,
	R_HTL_LOBBY = 0x92,
	R_HTL_RECEPTION_RM = 0x93,
	R_HTL_STORE_RM_1F = 0x94,
	R_HTL_EMPLOYEE_LOUNGE = 0x95,
	R_HTL_PANTRY = 0x96,
	R_HTL_OFFICE = 0x97,
	R_HTL_RESTAURANT = 0x98,
	R_HTL_MAIN_HALL_1F = 0x99,
	R_HTL_EMPLOYEE_HALL_1F = 0x9A,
	R_HTL_READING_RM = 0x9B,
	R_HTL_CLOAK_RM = 0x9C,
	R_HTL_EMPLOYEE_ELEV_RM = 0x9D,
	R_HTL_RM_202_204 = 0x9E,
	R_HTL_W_ROOM_HALL_2F = 0x9F,
	R_HTL_W_HALL_2F = 0xA0,
	R_HTL_E_HALL_2F = 0xA1,
	R_HTL_RM_312 = 0xA2,
	R_HTL_MAIN_HALL_3F = 0xA3,
	R_HTL_BAR = 0xA5,
	R_HTL_BAR_KITCHEN = 0xA6,
	R_HTL_BOILER_RM = 0xA7,
	R_HTL_HALL_BF = 0xA8,
	R_HTL_EMPLOYEE_HALL_BF = 0xA9,
	R_HTL_FIRE_STAIRCASE = 0xAA,
	R_HTL_ALT_EMPLOYEE_STAIRS = 0xAB,
	R_HTL_ALT_RPT_BOSS_RM = 0xAC,
	R_HTL_ALT_MAIN_HALL_1F = 0xAD,
	R_HTL_ALT_EMPLOYEE_HALL_1F = 0xAE,
	R_HTL_ALT_NINE_SAVE_RM = 0xAF,
	R_HTL_ALT_READING_RM = 0xB0,
	R_HTL_ALT_W_ROOM_HALL_2F = 0xB1,
	R_HTL_ALT_W_HALL_2F = 0xB2,
	R_HTL_ALT_E_HALL_2F = 0xB3,
	R_HTL_ALT_E_ROOM_HALL_2F = 0xB4,
	R_HTL_ALT_MAIN_HALL_3F = 0xB5,
	R_HTL_ALT_BAR = 0xB6,
	R_HTL_ALT_BAR_KITCHEN = 0xB7,
	R_HTL_ALT_ELEVATOR = 0xB8,
	R_HTL_ALT_EMPLOYEE_HALL_BF = 0xB9,
	R_HTL_ALT_FINAL_HALL = 0xBA,
	R_FINAL_BOSS_RM = 0xBB,
	R_END_BEDROOM_WATER = 0xBC,
	R_END_DOG_RM = 0xBD,
	R_ALT_BAR_NEELYS = 0xBE,
	R_END_BEDROOM_LEAVE = 0xBF,
	R_MAN_GRAND_ENTRANCE = 0xC0,
	R_MAN_LIVING_ROOM_1F = 0xC1,
	R_MAN_LOUNGE_2F = 0xC2,
	R_MAN_GRAVE_ROOM = 0xC3,
	R_MAN_SERV_RM = 0xC4,
	R_MAN_KIDS_RM = 0xC5,
	R_MAN_STUDY_RM = 0xC6,
	R_MAN_ATTIC = 0xC7,
	R_MAN_S_HALL_1F = 0xC8,
	R_MAN_S_HALL_STAIRCASE = 0xC9,
	R_MAN_LONG_HALLWAY = 0xCA,
	R_MAN_N_STAIRCASE = 0xCB,
	R_MAN_GUEST_HOUSE = 0xCC,
	R_MAN_CENTER_STAIRS = 0xCD,
	R_MAN_N_HALL_2F = 0xCE,
	R_MAN_GRAND_HALL_2F = 0xCF,
	R_MAN_S_HALL_2F = 0xD0,
	R_MAN_BF_TUNNEL = 0xD1,
	R_MAN_S_STAIRCASE = 0xD2,
	R_MAN_OUTSIDE_ENTRANCE = 0xD3,
	R_MAN_BLUE_CREEK_ENTRANCE = 0xD4,
	R_BFAW_MARIA_ROOM = 0xD5,
} ROOMID;

typedef enum _CONTROL_TYPE
{
	ROTATIONAL_CONTROL,
	DIRECTIONAL_CONTROL,
} CONTROL_TYPE;

enum class WEAPONTYPE : uint8_t {
	WT_NONE = 0,
	WT_HANDGUN = 1,
	WT_SHOTGUN = 2,
	WT_RIFLE = 3,
	WT_HYPER_SPRAY = 4,
	WT_PLANK = 5,
	WT_PIPE = 6,
	WT_CHAINSAW = 7,
	WT_GREAT_KNIFE = 8,
	WT_REVOLVER = 9,
	WT_CLEAVER = 10,
};

enum class ModelID;

// Shared function declaration
DWORD GetRoomID();
DWORD GetCutsceneID();
float GetCutscenePos();
float GetCutsceneTimer();
float GetCameraFOV();
float GetJamesPosX();
float GetJamesPosY();
float GetJamesPosZ();
BYTE GetFlashLightRender();
bool GetFlashLightAcquired();
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
uint16_t GetNumberOfSaves();
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
BYTE GetMemoUniqueId();
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
BYTE GetInventoryItem();
int8_t GetControlOptionsSelectedOption();
int32_t GetControlOptionsIsToStopScrolling();
int8_t GetControlOptionsSelectedColumn();
int8_t GetControlOptionsChanging();

// Shared pointer function declaration
DWORD *GetRoomIDPointer();
DWORD *GetCutsceneIDPointer();
float *GetCutscenePosPointer();
float *GetCutsceneTimerPointer();
float *GetCameraFOVPointer();
float *GetJamesPosXPointer();
float *GetJamesPosYPointer();
float *GetJamesPosZPointer();
BYTE *GetFlashLightRenderPointer();
DWORD *GetFlashLightAcquiredPointer();
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
uint16_t *GetNumberOfSavesPointer();
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
BYTE *GetMemoUniqueIdPointer();
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
BYTE* InventoryItemPointer();
BYTE* GetKeyBindsPointer();
int8_t* GetControlOptionsSelectedOptionPointer();
int32_t* GetControlOptionsIsToStopScrollingPointer();
int8_t* GetControlOptionsSelectedColumnPointer();
int8_t* GetControlOptionsChangingPointer();
WEAPONTYPE* GetWeaponRenderPointer();
WEAPONTYPE GetWeaponRender();
WEAPONTYPE* GetWeaponHandGripPointer();
WEAPONTYPE GetWeaponHandGrip();
BYTE* GetInGameVoiceEvent();

// Function patch declaration
void CheckArgumentsForPID();
void RelaunchSilentHill2();
void CheckAdminAccess();
void RemoveVirtualStoreFiles();
void RemoveCompatibilityMode();
bool SetDelayedStart();
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
void PatchAtticShadows();
void PatchBestGraphics();
void PatchBinary();
void PatchCDCheck();
void PatchCatacombsMeatRoom();
void PatchClosetSpawn();
void PatchCommandWindowMouseFix();
void PatchControlOptionsMenu();
void PatchCreatureVehicleSpawn();
void PatchCriware();
void PatchCustomAdvancedOptions();
HRESULT PatchCustomExeStr();
void PatchCustomFog();
void PatchCustomFonts();
void PatchCustomSFXs();
void PatchControllerTweaks();
void PatchCockroachesReplacement();
void PatchDelayedFadeIn();
void PatchDisplayMode();
void PatchDoubleFootstepFix();
void PatchDrawDistance();
void PatchElevatorCursorColor();
void PatchFinalBossRoom();
void PatchFireEscapeKey();
void PatchFlashlightClockPush();
void PatchFlashlightFlicker();
void PatchFlashlightReflection();
void PatchWaterEnhancement();
void PatchFMV();
void PatchFMVFramerate();
void PatchFmvSubtitlesNoiseFix();
void PatchFmvSubtitlesSyncFix();
void PatchFogParameters();
void PatchFullscreenImages();
void PatchFullscreenVideos();
void PatchGameLoad();
void PatchGameLoadFlashFix();
void PatchHealthIndicatorOption();
void PatchGreatKnifeBoatSpeed();
void PatchHoldDamage();
void PatchHoldToStomp();
void PatchInputTweaks();
void PatchInventoryBGMBug();
void PatchLakeMoonSize();
void PatchLeaveEndingCemeteryDrawDistance();
void PatchLockScreenPosition();
void PatchLowHealthIndicator();
void PatchMainMenu();
void PatchMainMenuInstantLoadOptions();
void PatchMainMenuTitlePerLang();
void PatchMapTranscription();
HRESULT PatchMasterVolumeSlider();
void PatchMemoBrightnes();
void PatchMenuSounds();
void PatchMothDrawOrder();
void PatchOldManCoinFix();
void PatchObservationDeckFogFix();
void PatchPauseScreen();
void PatchPistonRoom();
void PatchPreventChainsawSpawn();
void PatchPrisonerTimer();
void PatchPS2Flashlight();
void PatchPS2NoiseFilter();
void PatchPuzzleAlignmentFixes();
void PatchQuickSaveTweaks();
void PatchQuickSaveCancelFix();
void PatchRedCrossInCutscene();
void PatchRemoveWeaponFromCutscene();
void PatchRoom312ShadowFix();
void PatchRoomLighting();
void PatchRowboatAnimation();
void PatchSaveBGImage();
void PatchSearchViewOptionName();
void PatchSpeakerConfigLock();
void PatchSpecialFX();
void PatchSpecular();
void PatchSprayEffect();
void PatchSFXAddr();
void PatchShowerRoomFlashlightFix();
void PatchSixtyFPS();
HRESULT PatchSpeakerConfigText();
void PatchSpeakerConfigLock();
void PatchSpecialFX();
void PatchSpecificSoundLoopFix();
void PatchSpecular();
void PatchSprayEffect();
void PatchSwapLightHeavyAttack();
void PatchTeddyBearLookFix();
void PatchTexAddr();
void PatchTownGateEvents();
void PatchTreeLighting();
void PatchVHSAudio();
void PatchWaterDrawOrderFix();
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
bool IsInControlOptionsMenu();
bool CheckForSkipFrameCutscene();

void HandleMenuSounds();
void SetNewVolume();

void SetNewDisplayModeSetting();

void OnFileLoadTex(LPCSTR lpFileName);
void OnFileLoadVid(LPCSTR lpFileName);

void RunAtticShadows();
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
void RunPlayFlashlightSounds();
void RunPlayLyingFigureSounds();
void RunQuickSaveTweaks();
void RunQuickSaveCancelFix();
void RunRoomLighting();
void RunRotatingMannequin();
void RunSaveBGImage();
void RunShadowCutscene();
void RunSpecialFXScale(DWORD Height);
void RunTreeColor();

int GetNewScreenMode();

void CheckFlashlightAvailable();
float GetConditionalFlashlightBrightnessRed();
float GetConditionalFlashlightBrightnessGreen();
float GetConditionalFlashlightBrightnessBlue();
void CheckLakeMoonSize();
void CheckRoom312Flashlight();

// Define the template function declaration
template<typename T>
void GetNonScaledResolution(T& Width, T& Height);

// Variable forward declaration
extern SH2VERSION GameVersion;
extern bool IsInFullscreenImage;
extern bool IsInBloomEffect;
extern bool IsInFakeFadeout;
extern DWORD *RoomIDAddr;
extern DWORD *CutsceneIDAddr;
extern float *CutscenePosAddr;
extern float *CutsceneTimerAddr;
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
extern BYTE *EventIndexAddr;
extern BYTE *MenuEventAddr;
extern DWORD *TransitionStateAddr;
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
extern uint16_t *NumberOfSavesAddr;
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
extern WEAPONTYPE* WeaponRenderAddr;
extern WEAPONTYPE* WeaponHandGripAddr;

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
