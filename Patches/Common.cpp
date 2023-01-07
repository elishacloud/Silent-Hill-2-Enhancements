/**
* Copyright (C) 2022 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

// Variables
BYTE *ChapterIDAddr = nullptr;
DWORD *CutsceneIDAddr = nullptr;
float *CutscenePosAddr = nullptr;
float *CameraFOVAddr = nullptr;
float *FlashlightBrightnessAddr = nullptr;
BYTE *FlashLightRenderAddr = nullptr;
BYTE *FlashlightSwitchAddr = nullptr;
float *JamesPosXAddr = nullptr;
float *JamesPosYAddr = nullptr;
float *JamesPosZAddr = nullptr;
DWORD *OnScreenAddr = nullptr;
BYTE *EventIndexAddr = nullptr;
BYTE *MenuEventAddr = nullptr;
DWORD *RoomIDAddr = nullptr;
DWORD *SpecializedLight1Addr = nullptr;
DWORD *SpecializedLight2Addr = nullptr;
DWORD *TransitionStateAddr = nullptr;
BYTE *FullscreenImageEventAddr = nullptr;
float *InGameCameraPosYAddr = nullptr;
BYTE *InventoryStatusAddr = nullptr;
DWORD *LoadingScreenAddr = nullptr;
BYTE *PauseMenuButtonIndexAddr = nullptr;
BYTE *PauseMenuQuitIndexAddr = nullptr;
float *FPSCounterAddr = nullptr;
int16_t *ShootingKillsAddr = nullptr;
int16_t *MeleeKillsAddr = nullptr;
float *BoatMaxSpeedAddr = nullptr;
int8_t *ActionDifficultyAddr;
int8_t *RiddleDifficultyAddr;
int8_t *NumberOfSavesAddr;
float *InGameTimeAddr;
float *WalkingDistanceAddr;
float *RunningDistanceAddr;
int16_t *ItemsCollectedAddr;
float *DamagePointsTakenAddr;
uint8_t *SecretItemsCollectedAddr;
float *BoatStageTimeAddr;
int32_t* MouseVerticalPositionAddr;
int32_t* MouseHorizontalPositionAddr;
DWORD* LeftAnalogXFunctionAddr;
DWORD* LeftAnalogYFunctionAddr;
DWORD* RightAnalogXFunctionAddr;
DWORD* RightAnalogYFunctionAddr;
DWORD* UpdateMousePositionFunctionAddr;
BYTE* SearchViewFlagAddr;
int32_t* EnableInputAddr;
BYTE* AnalogXAddr;
BYTE* ControlTypeAddr;
BYTE* RunOptionAddr;
BYTE* NumKeysWeaponBindStartAddr;
BYTE *TalkShowHostStateAddr;
BYTE *BoatFlagAddr;
int32_t *IsWritingQuicksaveAddr;
int32_t *TextAddrAddr;
float *WaterAnimationSpeedPointer;
int16_t *FlashlightOnSpeedPointer;
float* LowHealthIndicatorFlashSpeedPointer;
float *WaterAnimationSpeedAddr;
int16_t *FlashlightOnSpeedAddr;
float* LowHealthIndicatorFlashSpeedAddr;
float* StaircaseFlamesLightingSpeedAddr;
float* WaterLevelLoweringStepsAddr;
float* WaterLevelRisingStepsAddr;
float* BugRoomFlashlightFixAddr;
uint8_t* SixtyFPSFMVFixAddr;
uint8_t* GrabDamageAddr;
float* FrametimeAddr;
DWORD* MeatLockerFogFixOneAddr;
DWORD* MeatLockerFogFixTwoAddr;
DWORD* MeatLockerHangerFixOneAddr;
DWORD* MeatLockerHangerFixTwoAddr;
BYTE* ClearTextAddr;
float* MeetingMariaCutsceneFogCounterOneAddr;
float* MeetingMariaCutsceneFogCounterTwoAddr;
float* RPTClosetCutsceneMannequinDespawnAddr;
float* RPTClosetCutsceneBlurredBarsDespawnAddr;

bool ShowDebugOverlay = false;
bool ShowInfoOverlay = false;
std::string AuxDebugOvlString = "";
bool IsControllerConnected = false;
HWND GameWindowHandle = NULL;
int LastEventIndex = 0x00;

DWORD GetRoomID()
{
	DWORD *pRoomID = GetRoomIDPointer();

	return (pRoomID) ? *pRoomID : 0;
}

DWORD *GetRoomIDPointer()
{
	if (RoomIDAddr)
	{
		return RoomIDAddr;
	}

	// Get room ID address
	constexpr BYTE RoomIDSearchBytes[]{ 0x83, 0xF8, 0x04, 0x0F, 0x87, 0xCE, 0x00, 0x00, 0x00 };
	void *RoomFunctAddr = (void*)SearchAndGetAddresses(0x0052A4A0, 0x0052A7D0, 0x0052A0F0, RoomIDSearchBytes, sizeof(RoomIDSearchBytes), 0xD7);

	// Checking address pointer
	if (!RoomFunctAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find room ID function address!";
		return nullptr;
	}

	// Check address
	if (!CheckMemoryAddress(RoomFunctAddr, "\x83\x3D", 2))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return nullptr;
	}
	RoomFunctAddr = (void*)((DWORD)RoomFunctAddr + 0x02);

	memcpy(&RoomIDAddr, RoomFunctAddr, sizeof(DWORD));

	return RoomIDAddr;
}

DWORD GetCutsceneID()
{
	DWORD *pCutsceneID = GetCutsceneIDPointer();

	return (pCutsceneID) ? *pCutsceneID : 0;
}

DWORD *GetCutsceneIDPointer()
{
	if (CutsceneIDAddr)
	{
		return CutsceneIDAddr;
	}

	// Get cutscene ID address
	constexpr BYTE CutsceneIDSearchBytes[]{ 0x8B, 0x56, 0x08, 0x89, 0x10, 0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x50, 0xC3 };
	void *CutsceneFunctAddr = (void*)SearchAndGetAddresses(0x004A0293, 0x004A0543, 0x0049FE03, CutsceneIDSearchBytes, sizeof(CutsceneIDSearchBytes), 0x1D);

	// Checking address pointer
	if (!CutsceneFunctAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find cutscene ID function address!";
		return nullptr;
	}

	// Check address
	if (!CheckMemoryAddress(CutsceneFunctAddr, "\xA1", 1))
	{
		Logging::Log() << __FUNCTION__ << " Error: memory addresses don't match!";
		return nullptr;
	}
	CutsceneFunctAddr = (void*)((DWORD)CutsceneFunctAddr + 0x01);

	memcpy(&CutsceneIDAddr, CutsceneFunctAddr, sizeof(DWORD));

	return CutsceneIDAddr;
}

float GetCutscenePos()
{
	float *pCutscenePos = GetCutscenePosPointer();

	return (pCutscenePos) ? *pCutscenePos : 0.0f;
}

float *GetCutscenePosPointer()
{
	if (CutscenePosAddr)
	{
		return CutscenePosAddr;
	}

	// Get cutscene Pos address
	constexpr BYTE CutscenePosSearchBytes[]{ 0x40, 0x88, 0x54, 0x24, 0x0B, 0x88, 0x4C, 0x24, 0x0A, 0x8B, 0x4C, 0x24, 0x08, 0x8B, 0xD1, 0x89, 0x0D };
	CutscenePosAddr = (float*)ReadSearchedAddresses(0x004A04DB, 0x004A078B, 0x004A004B, CutscenePosSearchBytes, sizeof(CutscenePosSearchBytes), 0x11);

	// Checking address pointer
	if (!CutscenePosAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find cutscene Pos function address!";
		return nullptr;
	}

	return CutscenePosAddr;
}

float GetCameraFOV()
{
	float *pCameraFOV = GetCameraFOVPointer();

	return (pCameraFOV) ? *pCameraFOV : 0.0f;
}

float *GetCameraFOVPointer()
{
	if (CameraFOVAddr)
	{
		return CameraFOVAddr;
	}

	// Get Camera FOV address
	constexpr BYTE CameraFOVSearchBytes[]{ 0x8D, 0x7C, 0x24, 0x3C, 0x50, 0xF3, 0xA5, 0xE8 };
	CameraFOVAddr = (float*)ReadSearchedAddresses(0x0048978E, 0x00489A2E, 0x00489C3E, CameraFOVSearchBytes, sizeof(CameraFOVSearchBytes), 0x0E);

	// Checking address pointer
	if (!CameraFOVAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Camera Pos function address!";
		return nullptr;
	}

	return CameraFOVAddr;
}

float GetJamesPosX()
{
	float *pJamesPosX = GetJamesPosXPointer();

	return (pJamesPosX) ? *pJamesPosX : 0.0f;
}

float *GetJamesPosXPointer()
{
	if (JamesPosXAddr)
	{
		return JamesPosXAddr;
	}

	// Get James Pos X address
	constexpr BYTE JamesPosXSearchBytes[]{ 0x4A, 0x8D, 0x88, 0xCC, 0x02, 0x00, 0x00, 0x89, 0x88, 0x94, 0x01, 0x00, 0x00, 0x8B, 0xC1, 0x75, 0xEF, 0x33, 0xC9, 0x89, 0x88, 0x94, 0x01, 0x00, 0x00, 0xB8 };
	void *JamesPositionX = (float*)ReadSearchedAddresses(0x00538070, 0x005383A0, 0x00537CC0, JamesPosXSearchBytes, sizeof(JamesPosXSearchBytes), -0x10);

	// Checking address pointer
	if (!JamesPositionX)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find James Pos X function address!";
		return nullptr;
	}
	JamesPosXAddr = (float*)((DWORD)JamesPositionX + 0x1C);

	return JamesPosXAddr;
}

float GetJamesPosY()
{
	float *pJamesPosY = GetJamesPosYPointer();

	return (pJamesPosY) ? *pJamesPosY : 0.0f;
}

float *GetJamesPosYPointer()
{
	if (JamesPosYAddr)
	{
		return JamesPosYAddr;
	}

	// Get James Pos Y address
	void *JamesPositionY = GetJamesPosXPointer();

	// Checking address pointer
	if (!JamesPositionY)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find James Pos Y function address!";
		return nullptr;
	}
	JamesPosYAddr = (float*)((DWORD)JamesPositionY + 0x04);

	return JamesPosYAddr;
}

float GetJamesPosZ()
{
	float *pJamesPosZ = GetJamesPosZPointer();

	return (pJamesPosZ) ? *pJamesPosZ : 0.0f;
}

float *GetJamesPosZPointer()
{
	if (JamesPosZAddr)
	{
		return JamesPosZAddr;
	}

	// Get James Pos Z address
	void *JamesPositionZ = GetJamesPosXPointer();

	// Checking address pointer
	if (!JamesPositionZ)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find James Pos Z function address!";
		return nullptr;
	}
	JamesPosZAddr = (float*)((DWORD)JamesPositionZ + 0x08);

	return JamesPosZAddr;
}

BYTE GetFlashLightRender()
{
	BYTE *pFlashLightRender = GetFlashLightRenderPointer();

	return (pFlashLightRender) ? *pFlashLightRender : 0;
}

BYTE *GetFlashLightRenderPointer()
{
	if (FlashLightRenderAddr)
	{
		return FlashLightRenderAddr;
	}

	// Get address for flashlight render
	constexpr BYTE FlashLightRenderSearchBytes[]{ 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x33, 0xC0, 0x66, 0xA3 };
	FlashLightRenderAddr = (BYTE*)ReadSearchedAddresses(0x0050A1D6, 0x0050A506, 0x00509E26, FlashLightRenderSearchBytes, sizeof(FlashLightRenderSearchBytes), 0x14);

	// Checking address pointer
	if (!FlashLightRenderAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find flashlight render address!";
		return nullptr;
	}

	return FlashLightRenderAddr;
}

BYTE GetChapterID()
{
	BYTE *pChapterID = GetChapterIDPointer();

	return (pChapterID) ? *pChapterID : 0;
}

BYTE *GetChapterIDPointer()
{
	if (ChapterIDAddr)
	{
		return ChapterIDAddr;
	}

	// Get address for flashlight render
	constexpr BYTE ChapterIDSearchBytes[]{ 0x00, 0x83, 0xC4, 0x04, 0xC3, 0x6A, 0x04, 0xE8 };
	ChapterIDAddr = (BYTE*)ReadSearchedAddresses(0x00446A5F, 0x00446BFF, 0x00446BFF, ChapterIDSearchBytes, sizeof(ChapterIDSearchBytes), -0x0D);

	// Checking address pointer
	if (!ChapterIDAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find chapter ID address!";
		return nullptr;
	}

	return ChapterIDAddr;
}

DWORD GetSpecializedLight1()
{
	DWORD *pSpecializedLight1 = GetSpecializedLight1Pointer();

	return (pSpecializedLight1) ? *pSpecializedLight1 : 0;
}

DWORD *GetSpecializedLight1Pointer()
{
	if (SpecializedLight1Addr)
	{
		return SpecializedLight1Addr;
	}

	// Get address for flashlight render
	constexpr BYTE SpecializedLightSearchBytes[]{ 0x8B, 0x44, 0x24, 0x04, 0x8B, 0x4C, 0x24, 0x08, 0xA3 };
	SpecializedLight1Addr = (DWORD*)ReadSearchedAddresses(0x00445630, 0x004457F0, 0x004457F0, SpecializedLightSearchBytes, sizeof(SpecializedLightSearchBytes), 0x09);

	// Checking address pointer
	if (!SpecializedLight1Addr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find specialized light address 1!";
		return nullptr;
	}

	return SpecializedLight1Addr;
}

DWORD GetSpecializedLight2()
{
	DWORD *pSpecializedLight2 = GetSpecializedLight2Pointer();

	return (pSpecializedLight2) ? *pSpecializedLight2 : 0;
}

DWORD *GetSpecializedLight2Pointer()
{
	if (SpecializedLight2Addr)
	{
		return SpecializedLight2Addr;
	}

	// Get address for flashlight render
	constexpr BYTE SpecializedLightSearchBytes[]{ 0x00, 0x00, 0x00, 0x52, 0x6A, 0x22, 0x50, 0x89, 0x1D };
	SpecializedLight2Addr = (DWORD*)ReadSearchedAddresses(0x004FFA1B, 0x004FFD4B, 0x004FF66B, SpecializedLightSearchBytes, sizeof(SpecializedLightSearchBytes), 0x09);

	// Checking address pointer
	if (!SpecializedLight2Addr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find specialized light address 2!";
		return nullptr;
	}

	return SpecializedLight2Addr;
}

BYTE GetFlashlightSwitch()
{
	BYTE *pFlashlightSwitch = GetFlashlightSwitchPointer();

	return (pFlashlightSwitch) ? *pFlashlightSwitch : 0;
}

BYTE *GetFlashlightSwitchPointer()
{
	if (FlashlightSwitchAddr)
	{
		return FlashlightSwitchAddr;
	}

	// Get address for flashlight on/off switch address
	constexpr BYTE FlashlightSwitchSearchBytes[]{ 0x83, 0xF8, 0x33, 0x53, 0x56, 0x0F, 0x87 };
	FlashlightSwitchAddr = (BYTE*)ReadSearchedAddresses(0x0043ED25, 0x0043EEE5, 0x0043EEE5, FlashlightSwitchSearchBytes, sizeof(FlashlightSwitchSearchBytes), 0x29);

	// Checking address pointer
	if (!FlashlightSwitchAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find flashlight on/off switch address!";
		return nullptr;
	}

	return FlashlightSwitchAddr;
}

float GetFlashlightBrightnessRed()
{
	float *pFlashlightBrightness = GetFlashlightBrightnessPointer();

	return (pFlashlightBrightness) ? *pFlashlightBrightness : 0.0f;
}

float GetFlashlightBrightnessGreen()
{
	float *pFlashlightBrightness = GetFlashlightBrightnessPointer();

	return (pFlashlightBrightness) ? *(float*)((DWORD)pFlashlightBrightness + 0x04) : 0.0f;
}

float GetFlashlightBrightnessBlue()
{
	float *pFlashlightBrightness = GetFlashlightBrightnessPointer();

	return (pFlashlightBrightness) ? *(float*)((DWORD)pFlashlightBrightness + 0x08) : 0.0f;
}

float *GetFlashlightBrightnessPointer()
{
	if (FlashlightBrightnessAddr)
	{
		return FlashlightBrightnessAddr;
	}

	// Get address for flashlight brightness address
	constexpr BYTE FlashlightBrightnessSearchBytes[]{ 0x8D, 0x54, 0x24, 0x2C, 0x52, 0x8D, 0x44, 0x24, 0x40, 0x50, 0x8D, 0x4C, 0x24, 0x54, 0x51, 0x68 };
	FlashlightBrightnessAddr = (float*)ReadSearchedAddresses(0x0047B4A5, 0x0047B745, 0x0047B955, FlashlightBrightnessSearchBytes, sizeof(FlashlightBrightnessSearchBytes), 0x10);

	// Checking address pointer
	if (!FlashlightBrightnessAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find flashlight brightness address!";
		return nullptr;
	}

	return FlashlightBrightnessAddr;
}

DWORD GetOnScreen()
{
	DWORD *pOnScreen = GetOnScreenPointer();

	return (pOnScreen) ? *pOnScreen : 0;
}

DWORD *GetOnScreenPointer()
{
	if (OnScreenAddr)
	{
		return OnScreenAddr;
	}

	// Get address for on-screen address
	constexpr BYTE OnScreenSearchBytes[]{ 0x33, 0xC0, 0x5B, 0xC3, 0x68 };
	OnScreenAddr = (DWORD*)ReadSearchedAddresses(0x0043F205, 0x0043F3C5, 0x0043F3C5, OnScreenSearchBytes, sizeof(OnScreenSearchBytes), 0x50);

	// Checking address pointer
	if (!OnScreenAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find on-screen address!";
		return nullptr;
	}

	return OnScreenAddr;
}

BYTE GetEventIndex()
{
	BYTE *pEventIndex = GetEventIndexPointer();

	return (pEventIndex) ? *pEventIndex : 0;
}

BYTE *GetEventIndexPointer()
{
	if (EventIndexAddr)
	{
		return EventIndexAddr;
	}

	// Get address for event index address
	constexpr BYTE EventIndexSearchBytes[]{ 0x5E, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x5B, 0xC3, 0x8B, 0xFF };
	EventIndexAddr = (BYTE*)ReadSearchedAddresses(0x0043F28A, 0x0043F44A, 0x0043F44A, EventIndexSearchBytes, sizeof(EventIndexSearchBytes), -0x35);

	// Checking address pointer
	if (!EventIndexAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find event index address!";
		return nullptr;
	}

	return EventIndexAddr;
}

BYTE GetMenuEvent()
{
	BYTE *pMenuEvent = GetMenuEventPointer();

	return (pMenuEvent) ? *pMenuEvent : 0;
}

BYTE *GetMenuEventPointer()
{
	if (MenuEventAddr)
	{
		return MenuEventAddr;
	}

	// Get menu event addresses
	constexpr BYTE MenuEventSearchBytes[]{ 0x83, 0xC4, 0x04, 0x33, 0xF6, 0x56, 0x6A, 0x01, 0x68, 0xF0, 0x00, 0x00, 0x00, 0x68, 0x00, 0x01, 0x00, 0x00, 0x6A, 0x1A, 0x50, 0xE8 };
	MenuEventAddr = (BYTE*)ReadSearchedAddresses(0x0044765D, 0x004477FD, 0x004477FD, MenuEventSearchBytes, sizeof(MenuEventSearchBytes), -0x3E);

	// Checking address pointer
	if (!MenuEventAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find event index address!";
		return nullptr;
	}

	return MenuEventAddr;
}

DWORD GetTransitionState()
{
	DWORD *pTransitionState = GetTransitionStatePointer();

	return (pTransitionState) ? *pTransitionState : 0;
}

DWORD *GetTransitionStatePointer()
{
	if (TransitionStateAddr)
	{
		return TransitionStateAddr;
	}

	// Get address for transition state
	constexpr BYTE TransitionAddrSearchBytes[]{ 0x83, 0xF8, 0x19, 0x7E, 0x72, 0x83, 0xF8, 0x1A, 0x75, 0x05, 0xBF, 0x01, 0x00, 0x00, 0x00, 0x39, 0x1D };
	TransitionStateAddr = (DWORD*)ReadSearchedAddresses(0x0048E87B, 0x0048EB1B, 0x0048ED2B, TransitionAddrSearchBytes, sizeof(TransitionAddrSearchBytes), 0x2A);

	// Checking address pointer
	if (!TransitionStateAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find transition state address!";
		return nullptr;
	}

	return TransitionStateAddr;
}

BYTE GetFullscreenImageEvent()
{
	BYTE *pFullscreenImageEvent = GetFullscreenImageEventPointer();

	return (pFullscreenImageEvent) ? *pFullscreenImageEvent : 0;
}

// Get full screen image event address
BYTE *GetFullscreenImageEventPointer()
{
	if (FullscreenImageEventAddr)
	{
		return FullscreenImageEventAddr;
	}

	// Get address for fullsceen image event
	constexpr BYTE FullscreenImageSearchBytes[]{ 0x33, 0xC0, 0x85, 0xC9, 0x0F, 0x94, 0xC0, 0xC3, 0x90, 0x90, 0xD9, 0x44, 0x24, 0x04, 0xD8, 0x64, 0x24, 0x0C, 0xD9, 0x1D };
	FullscreenImageEventAddr = (BYTE*)ReadSearchedAddresses(0x0048AFD6, 0x0048B276, 0x0048B486, FullscreenImageSearchBytes, sizeof(FullscreenImageSearchBytes), -0x37);

	// Checking address pointer
	if (!FullscreenImageEventAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return nullptr;
	}

	return FullscreenImageEventAddr;
}

float GetInGameCameraPosY()
{
	float *pInGameCameraPosY = GetInGameCameraPosYPointer();

	return (pInGameCameraPosY) ? *pInGameCameraPosY : 0.0f;
}

// Get Camera in-game position Y
float *GetInGameCameraPosYPointer()
{
	if (InGameCameraPosYAddr)
	{
		return InGameCameraPosYAddr;
	}

	// In-game camera Y
	constexpr BYTE InGameCameraYSearchBytes[]{ 0x8B, 0x08, 0x8B, 0x50, 0x04, 0x8B, 0x40, 0x08, 0x89, 0x44, 0x24, 0x0C, 0xA1 };
	InGameCameraPosYAddr = (float*)ReadSearchedAddresses(0x005155ED, 0x0051591D, 0x0051523D, InGameCameraYSearchBytes, sizeof(InGameCameraYSearchBytes), 0x17);

	// Checking address pointer
	if (!InGameCameraPosYAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return nullptr;
	}

	return InGameCameraPosYAddr;
}

BYTE GetInventoryStatus()
{
	BYTE *pInventoryStatus = GetInventoryStatusPointer();

	return (pInventoryStatus) ? *pInventoryStatus : 3;
}

// Get inventory status
BYTE *GetInventoryStatusPointer()
{
	if (InventoryStatusAddr)
	{
		return InventoryStatusAddr;
	}

	// In-game camera Y
	constexpr BYTE InventoryStatusSearchBytes[]{ 0x83, 0xF8, 0x03, 0x74, 0x08, 0x83, 0xF8, 0x01, 0x74, 0x03, 0x33, 0xC0, 0xC3, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3 };
	InventoryStatusAddr = (BYTE*)ReadSearchedAddresses(0x00476065, 0x00476305, 0x00476061, InventoryStatusSearchBytes, sizeof(InventoryStatusSearchBytes), -0x04);

	// Checking address pointer
	if (!InventoryStatusAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return nullptr;
	}

	return InventoryStatusAddr;
}

DWORD GetLoadingScreen()
{
	DWORD *pLoadingScreen = GetLoadingScreenPointer();

	return (pLoadingScreen) ? *pLoadingScreen : 0;
}

// Get loading screen status
DWORD *GetLoadingScreenPointer()
{
	if (LoadingScreenAddr)
	{
		return LoadingScreenAddr;
	}

	// In-game camera Y
	constexpr BYTE LoadingScreenSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x75, 0x23, 0xE8 };
	LoadingScreenAddr = (DWORD*)ReadSearchedAddresses(0x00406C13, 0x00406C13, 0x00406C23, LoadingScreenSearchBytes, sizeof(LoadingScreenSearchBytes), 0x14);

	// Checking address pointer
	if (!LoadingScreenAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return nullptr;
	}

	return LoadingScreenAddr;
}

BYTE GetPauseMenuButtonIndex()
{
	BYTE *PauseMenuIndex = GetPauseMenuButtonIndexPointer();

	return (PauseMenuIndex) ? *PauseMenuIndex : 0;
}

BYTE *GetPauseMenuButtonIndexPointer()
{
	if (PauseMenuButtonIndexAddr)
	{
		return PauseMenuButtonIndexAddr;
	}

	// Get Pause Menu Button Index address
	constexpr BYTE PauseMenuButtonIndexBytes[]{ 0x68, 0xFF, 0x00, 0x00, 0x00, 0x6A, 0x7F, 0x6A, 0x7F, 0x6A, 0x7F };
	auto PauseMenuIndexAddr = reinterpret_cast<void*>(SearchAndGetAddresses(0x00407497, 0x00407497, 0x004074A7, PauseMenuButtonIndexBytes,
		sizeof(PauseMenuButtonIndexBytes), 0x10));

	// Checking address pointer
	if (!PauseMenuIndexAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Button Index Pointer address!";
		return nullptr;
	}

	PauseMenuIndexAddr = reinterpret_cast<void*>(reinterpret_cast<DWORD>(PauseMenuIndexAddr) + 0x01);

	memcpy(&PauseMenuButtonIndexAddr, PauseMenuIndexAddr, sizeof(DWORD));

	return PauseMenuButtonIndexAddr;
}

BYTE GetPauseMenuQuitIndex()
{
	BYTE *PauseMenuIndex = GetPauseMenuQuitIndexPointer();

	return (PauseMenuIndex) ? *PauseMenuIndex : 0;
}

BYTE *GetPauseMenuQuitIndexPointer()
{
	return (BYTE*)0x009326B0;
	if (PauseMenuQuitIndexAddr)
	{
		return PauseMenuQuitIndexAddr;
	}

	// Get Pause Menu Quit Index address
	constexpr BYTE PauseMenuQuitIndexBytes[]{ 0x68, 0xFF, 0x00, 0x00, 0x00, 0x6A, 0x7F, 0x6A, 0x7F, 0x6A, 0x7F };
	auto PauseMenuIndexAddr = reinterpret_cast<void*>(SearchAndGetAddresses(0x00407497, 0x00407497, 0x004074A7, PauseMenuQuitIndexBytes,
		sizeof(PauseMenuQuitIndexBytes), 0x10));

	// Checking address pointer
	if (!PauseMenuIndexAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Quit Index Pointer address!";
		return nullptr;
	}

	PauseMenuIndexAddr = reinterpret_cast<void*>(reinterpret_cast<DWORD>(PauseMenuIndexAddr) + 0x01);

	memcpy(&PauseMenuQuitIndexAddr, PauseMenuIndexAddr, sizeof(DWORD));

	return PauseMenuQuitIndexAddr;
}

float GetFPSCounter()
{
	float *pFPSCounter = GetFPSCounterPointer();

	return (pFPSCounter) ? *pFPSCounter : 0;
}

float *GetFPSCounterPointer()
{
	if (FPSCounterAddr)
	{
		return FPSCounterAddr;
	}

	// Get FPS Counter address
	constexpr BYTE FPSCounterSearchBytes[]{ 0x89, 0x4c, 0x24, 0x18, 0x89, 0x44, 0x24, 0x1c };
	float *FPSCounter = (float*)ReadSearchedAddresses(0x004F6D1F, 0x004F6FCF, 0x004F688F, FPSCounterSearchBytes, sizeof(FPSCounterSearchBytes), 0x81);

	// Checking address pointer
	if (!FPSCounter)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find FPS Counter address!";
		return nullptr;
	}

	FPSCounterAddr = (float*)((DWORD)FPSCounter + 0x0);

	return FPSCounterAddr;
}

int16_t GetShootingKills()
{
	int16_t *pShootingKills = GetShootingKillsPointer();

	return (pShootingKills) ? *pShootingKills : 0;
}

int16_t *GetShootingKillsPointer()
{
	if (FPSCounterAddr)
	{
		return ShootingKillsAddr;
	}

	// Get Shooting Kills address
	constexpr BYTE ShootingKillsSearchBytes[]{ 0x0F, 0xB7, 0x44, 0x24, 0x04, 0x48, 0x83, 0xF8, 0x19 };
	int16_t *ShootingKills = (int16_t*)ReadSearchedAddresses(0x00539B10, 0x00539E40, 0x00539760, ShootingKillsSearchBytes, sizeof(ShootingKillsSearchBytes), 0x1C);

	// Checking address pointer
	if (!ShootingKills)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Shooting Kills address!";
		return nullptr;
	}

	ShootingKillsAddr = (int16_t*)((DWORD)ShootingKills);

	return ShootingKillsAddr;
}

int16_t GetMeleeKills()
{
	int16_t *pMeleeKills = GetMeleeKillsPointer();

	return (pMeleeKills) ? *pMeleeKills : 0;
}

int16_t *GetMeleeKillsPointer()
{
	if (MeleeKillsAddr)
	{
		return MeleeKillsAddr;
	}

	// Get Melee Kills address
	constexpr BYTE MeleeKillsSearchBytes[]{ 0x0F, 0xB7, 0x44, 0x24, 0x04, 0x48, 0x83, 0xF8, 0x19 };
	int16_t *MeleeKills = (int16_t*)ReadSearchedAddresses(0x00539B10, 0x00539E40, 0x00539760, MeleeKillsSearchBytes, sizeof(MeleeKillsSearchBytes), 0x24);

	// Checking address pointer
	if (!MeleeKills)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Melee Kills address!";
		return nullptr;
	}

	MeleeKillsAddr = (int16_t*)((DWORD)MeleeKills);

	return MeleeKillsAddr;
}

float GetBoatMaxSpeed()
{
	float *pBoatMaxSpeed = GetBoatMaxSpeedPointer();

	return (pBoatMaxSpeed) ? *pBoatMaxSpeed : 0;
}

float *GetBoatMaxSpeedPointer()
{
	if (BoatMaxSpeedAddr)
	{
		return BoatMaxSpeedAddr;
	}

	// Get Boat Max Speed address
	constexpr BYTE BoatMaxSpeedSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x75, 0x09, 0x8B, 0x44, 0x24, 0x04 };
	float *BoatMaxSpeed = (float*)ReadSearchedAddresses(0x00539B8A, 0x00539EBA, 0x005397DA, BoatMaxSpeedSearchBytes, sizeof(BoatMaxSpeedSearchBytes), 0x0C);

	// Checking address pointer
	if (!BoatMaxSpeed)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Boat Max Speed address!";
		return nullptr;
	}

	BoatMaxSpeedAddr = (float*)((DWORD)BoatMaxSpeed);

	return BoatMaxSpeedAddr;
}

int8_t GetActionDifficulty()
{
	int8_t *pActionDifficulty = GetActionDifficultyPointer();

	return (pActionDifficulty) ? *pActionDifficulty : 0;
}

int8_t *GetActionDifficultyPointer()
{
	if (ActionDifficultyAddr)
	{
		return ActionDifficultyAddr;
	}

	// Get Action Difficulty address
	constexpr BYTE ActionDifficultySearchBytes[]{ 0x83, 0xC4, 0x18, 0x83, 0xF8, 0x1C, 0x74, 0x25 };
	int8_t *ActionDifficulty = (int8_t*)ReadSearchedAddresses(0x0055768D, 0x005579BD, 0x005572DD, ActionDifficultySearchBytes, sizeof(ActionDifficultySearchBytes), 0x0A);

	// Checking address pointer
	if (!ActionDifficulty)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Action Difficulty address!";
		return nullptr;
	}

	ActionDifficultyAddr = (int8_t*)((DWORD)ActionDifficulty);

	return ActionDifficultyAddr;
}

int8_t GetRiddleDifficulty()
{
	int8_t *pRiddleDifficulty = GetRiddleDifficultyPointer();

	return (pRiddleDifficulty) ? *pRiddleDifficulty : 0;
}

int8_t *GetRiddleDifficultyPointer()
{
	if (RiddleDifficultyAddr)
	{
		return RiddleDifficultyAddr;
	}

	// Get Riddle Difficulty address
	constexpr BYTE RiddleDifficultySearchBytes[]{ 0xEB, 0x27, 0x66, 0x3D, 0x32, 0x00, 0x75, 0x11 };
	int8_t *RiddleDifficulty = (int8_t*)ReadSearchedAddresses(0x0046BF90, 0x0046C230, 0x0046C440, RiddleDifficultySearchBytes, sizeof(RiddleDifficultySearchBytes), 0x0A);

	// Checking address pointer
	if (!RiddleDifficulty)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Riddle Difficulty address!";
		return nullptr;
	}

	RiddleDifficultyAddr = (int8_t*)((DWORD)RiddleDifficulty);

	return RiddleDifficultyAddr;
}

int8_t GetNumberOfSaves()
{
	int8_t *pNumberOfSaves = GetNumberOfSavesPointer();

	return (pNumberOfSaves) ? *pNumberOfSaves : 0;
}

int8_t *GetNumberOfSavesPointer()
{
	if (NumberOfSavesAddr)
	{
		return NumberOfSavesAddr;
	}

	// Get Number of Saves address
	constexpr BYTE NumberOfSavesSearchBytes[]{ 0xE8, 0xEB, 0xED, 0xFF, 0xFF, 0x83, 0xF8, 0x02 };
	int8_t *NumberOfSaves = (int8_t*)ReadSearchedAddresses(0x00454340, 0x004545A0, 0x004545A0, NumberOfSavesSearchBytes, sizeof(NumberOfSavesSearchBytes), 0x0D);

	// Checking address pointer
	if (!NumberOfSaves)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Number of Saves address!";
		return nullptr;
	}

	NumberOfSavesAddr = (int8_t*)((DWORD)NumberOfSaves);

	return NumberOfSavesAddr;
}

float GetInGameTime()
{
	float *pInGameTime = GetInGameTimePointer();

	return (pInGameTime) ? *pInGameTime : 0;
}

float *GetInGameTimePointer()
{
	if (InGameTimeAddr)
	{
		return InGameTimeAddr;
	}

	// Get In Game Timer address
	constexpr BYTE InGameTimeSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x7A, 0x07, 0xB8, 0x0A, 0x00 };
	float *InGameTime = (float*)ReadSearchedAddresses(0x00539E27, 0x0053A157, 0x00539A77, InGameTimeSearchBytes, sizeof(InGameTimeSearchBytes), -0x14);

	// Checking address pointer
	if (!InGameTime)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find In Game Timer address!";
		return nullptr;
	}

	InGameTimeAddr = (float*)((DWORD)InGameTime);

	return InGameTimeAddr;
}

float GetWalkingDistance()
{
	float *pWalkingDistance = GetWalkingDistancePointer();

	return (pWalkingDistance) ? *pWalkingDistance : 0;
}

float *GetWalkingDistancePointer()
{
	if (WalkingDistanceAddr)
	{
		return WalkingDistanceAddr;
	}

	// Get Walking Distance address
	constexpr BYTE WalkingDistanceSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x7A, 0x2D, 0x8B, 0x44, 0x24 };
	float *WalkingDistance = (float*)ReadSearchedAddresses(0x00539ACD, 0x00539DFD, 0x0053971D, WalkingDistanceSearchBytes, sizeof(WalkingDistanceSearchBytes), 0x28);

	// Checking address pointer
	if (!WalkingDistance)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Walking Distance address!";
		return nullptr;
	}

	WalkingDistanceAddr = (float*)((DWORD)WalkingDistance);

	return WalkingDistanceAddr;
}

float GetRunningDistance()
{
	float *pRunningDistance = GetRunningDistancePointer();

	return (pRunningDistance) ? *pRunningDistance : 0;
}

float *GetRunningDistancePointer()
{
	if (RunningDistanceAddr)
	{
		return RunningDistanceAddr;
	}

	// Get Running Distance address
	constexpr BYTE RunningDistanceSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x7A, 0x2D, 0x8B, 0x44, 0x24 };
	float *RunningDistance = (float*)ReadSearchedAddresses(0x00539ACD, 0x00539DFD, 0x0053971D, RunningDistanceSearchBytes, sizeof(RunningDistanceSearchBytes), 0x17);

	// Checking address pointer
	if (!RunningDistance)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Running Distance address!";
		return nullptr;
	}

	RunningDistanceAddr = (float*)((DWORD)RunningDistance);

	return RunningDistanceAddr;
}

int16_t GetItemsCollected()
{
	int16_t *pItemsCollected = GetItemsCollectedPointer();

	return (pItemsCollected) ? *pItemsCollected : 0;
}

int16_t *GetItemsCollectedPointer()
{
	if (ItemsCollectedAddr)
	{
		return ItemsCollectedAddr;
	}

	// Get Items Collected address
	constexpr BYTE ItemsCollectedSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x75, 0x09, 0x8B, 0x44, 0x24, 0x04 };
	int16_t *ItemsCollected = (int16_t*)ReadSearchedAddresses(0x00539B8A, 0x00539EBA, 0x005397DA, ItemsCollectedSearchBytes, sizeof(ItemsCollectedSearchBytes), 0x19);

	// Checking address pointer
	if (!ItemsCollected)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Items Collected address!";
		return nullptr;
	}

	ItemsCollectedAddr = (int16_t*)((DWORD)ItemsCollected);

	return ItemsCollectedAddr;
}

float GetDamagePointsTaken()
{
	float *pDamagePointsTaken = GetDamagePointsTakenPointer();

	return (pDamagePointsTaken) ? *pDamagePointsTaken : 0;
}

float *GetDamagePointsTakenPointer()
{
	if (DamagePointsTakenAddr)
	{
		return DamagePointsTakenAddr;
	}

	// Get Damage Points Taken address
	constexpr BYTE DamagePointsTakenSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x75, 0x09, 0x8B, 0x44, 0x24, 0x04 };
	float *DamagePointsTaken = (float*)ReadSearchedAddresses(0x00539B8A, 0x00539EBA, 0x005397DA, DamagePointsTakenSearchBytes, sizeof(DamagePointsTakenSearchBytes), 0x28);

	// Checking address pointer
	if (!DamagePointsTaken)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Damage Points Taken address!";
		return nullptr;
	}

	DamagePointsTakenAddr = (float*)((DWORD)DamagePointsTaken);

	return DamagePointsTakenAddr;
}

uint8_t GetSecretItemsCollected()
{
	uint8_t *pSecretItemsCollected = GetSecretItemsCollectedPointer();

	return (pSecretItemsCollected) ? *pSecretItemsCollected : 0;
}

uint8_t *GetSecretItemsCollectedPointer()
{
	if (SecretItemsCollectedAddr)
	{
		return SecretItemsCollectedAddr;
	}

	// Get Secret Items Collected address
	constexpr BYTE SecretItemsCollectedSearchBytes[]{ 0xF6, 0xC1, 0x02, 0x74, 0x01, 0x40, 0xF6, 0xC1, 0x04, 0x74, 0x01, 0x40 };
	uint8_t *SecretItemsCollected = (uint8_t*)ReadSearchedAddresses(0x00539D46, 0x0053A076, 0x00539996, SecretItemsCollectedSearchBytes, sizeof(SecretItemsCollectedSearchBytes), -0x16);

	// Checking address pointer
	if (!SecretItemsCollected)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Secret Items Collected address!";
		return nullptr;
	}

	SecretItemsCollectedAddr = (uint8_t*)((DWORD)SecretItemsCollected);

	return SecretItemsCollectedAddr;
}

float GetBoatStageTime()
{
	float *pBoatStageTime = GetBoatStageTimePointer();

	return (pBoatStageTime) ? *pBoatStageTime : 0;
}

float *GetBoatStageTimePointer()
{
	if (BoatStageTimeAddr)
	{
		return BoatStageTimeAddr;
	}

	// Get Boat Stage Time address
	constexpr BYTE BoatStageTimeSearchBytes[]{ 0xB9, 0x3C, 0x00, 0x00, 0x00, 0xF7, 0xF9, 0x8B, 0xE8, 0x8B, 0xFA };
	float *BoatStageTime = (float*)ReadSearchedAddresses(0x00447159, 0x004472F9, 0x004472F9, BoatStageTimeSearchBytes, sizeof(BoatStageTimeSearchBytes), -0x0B);

	// Checking address pointer
	if (!BoatStageTime)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Boat Stage Time address!";
		return nullptr;
	}

	BoatStageTimeAddr = (float*)((DWORD)BoatStageTime);

	return BoatStageTimeAddr;
}

int32_t GetMouseVerticalPosition()
{
	int32_t *pMouseVerticalPosition = GetMouseVerticalPositionPointer();

	return (pMouseVerticalPosition) ? *pMouseVerticalPosition : 0;
}

int32_t *GetMouseVerticalPositionPointer()
{
	if (MouseVerticalPositionAddr)
	{
		return MouseVerticalPositionAddr;
	}

	// Get MouseVerticalPosition address
	constexpr BYTE MouseVerticalPositionSearchBytes[]{ 0x8B, 0x08, 0x50, 0xFF, 0x51, 0x18, 0x85, 0xC0, 0x7C, 0x33 };
	int32_t *MouseVerticalPosition = (int32_t*)ReadSearchedAddresses(0x0045A49F, 0x0045A6FF, 0x0045A6FF, MouseVerticalPositionSearchBytes, sizeof(MouseVerticalPositionSearchBytes), 0x1C);

	// Checking address pointer
	if (!MouseVerticalPosition)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find MouseVerticalPosition address!";
		return nullptr;
	}

	MouseVerticalPositionAddr = (int32_t*)((DWORD)MouseVerticalPosition);

	return MouseVerticalPositionAddr;
}

int32_t GetMouseHorizontalPosition()
{
	int32_t *pMouseHorizontalPosition = GetMouseHorizontalPositionPointer();

	return (pMouseHorizontalPosition) ? *pMouseHorizontalPosition : 0;
}

int32_t *GetMouseHorizontalPositionPointer()
{
	if (MouseHorizontalPositionAddr)
	{
		return MouseHorizontalPositionAddr;
	}

	// Get MouseHorizontalPosition address
	constexpr BYTE MouseHorizontalPositionSearchBytes[]{ 0x8B, 0x08, 0x50, 0xFF, 0x51, 0x18, 0x85, 0xC0, 0x7C, 0x33 };
	int32_t *MouseHorizontalPosition = (int32_t*)ReadSearchedAddresses(0x0045A49F, 0x0045A6FF, 0x0045A6FF, MouseHorizontalPositionSearchBytes, sizeof(MouseHorizontalPositionSearchBytes), 0x17);

	// Checking address pointer
	if (!MouseHorizontalPosition)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find MouseHorizontalPosition address!";
		return nullptr;
	}

	MouseHorizontalPositionAddr = (int32_t*)((DWORD)MouseHorizontalPosition);

	return MouseHorizontalPositionAddr;
}

DWORD *GetLeftAnalogXFunctionPointer()
{
	if (LeftAnalogXFunctionAddr)
	{
		return LeftAnalogXFunctionAddr;
	}

	// Get Analog Stick function address
	constexpr BYTE LeftAnalogXFunctionSearchBytes[]{ 0xF6, 0xC4, 0x05, 0x7A, 0x06, 0xDD, 0xD8, 0xD9, 0xC0, 0xEB, 0x15 };
	void *LeftAnalogXAddr = (void*)SearchAndGetAddresses(0x0052E7D3, 0x0052EB03, 0x0052E423, LeftAnalogXFunctionSearchBytes, sizeof(LeftAnalogXFunctionSearchBytes), -0x14E);

	// Checking address pointer
	if (!LeftAnalogXAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog Stick function address!";
		return nullptr;
	}

	LeftAnalogXFunctionAddr = (DWORD*)LeftAnalogXAddr;

	return LeftAnalogXFunctionAddr;
}

DWORD *GetLeftAnalogYFunctionPointer()
{
	if (LeftAnalogYFunctionAddr)
	{
		return LeftAnalogYFunctionAddr;
	}

	// Get Analog Stick function address
	constexpr BYTE LeftAnalogYFunctionSearchBytes[]{ 0xF6, 0xC4, 0x05, 0x7A, 0x06, 0xDD, 0xD8, 0xD9, 0xC0, 0xEB, 0x15 };
	void *LeftAnalogYAddr = (void*)SearchAndGetAddresses(0x0052E7D3, 0x0052EB03, 0x0052E423, LeftAnalogYFunctionSearchBytes, sizeof(LeftAnalogYFunctionSearchBytes), -0x13E);

	// Checking address pointer
	if (!LeftAnalogYAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog Stick function address!";
		return nullptr;
	}

	LeftAnalogYFunctionAddr = (DWORD*)LeftAnalogYAddr;

	return LeftAnalogYFunctionAddr;
}

DWORD *GetRightAnalogXFunctionPointer()
{
	if (RightAnalogXFunctionAddr)
	{
		return RightAnalogXFunctionAddr;
	}

	// Get Analog Stick function address
	constexpr BYTE RightAnalogXFunctionSearchBytes[]{ 0xF6, 0xC4, 0x05, 0x7A, 0x06, 0xDD, 0xD8, 0xD9, 0xC0, 0xEB, 0x15 };
	void *RightAnalogXAddr = (void*)SearchAndGetAddresses(0x0052E7D3, 0x0052EB03, 0x0052E423, RightAnalogXFunctionSearchBytes, sizeof(RightAnalogXFunctionSearchBytes), -0x12B);

	// Checking address pointer
	if (!RightAnalogXAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog Stick function address!";
		return nullptr;
	}

	RightAnalogXFunctionAddr = (DWORD*)RightAnalogXAddr;

	return RightAnalogXFunctionAddr;
}

DWORD *GetRightAnalogYFunctionPointer()
{
	if (RightAnalogYFunctionAddr)
	{
		return RightAnalogYFunctionAddr;
	}

	// Get Analog Stick function address
	constexpr BYTE RightAnalogYFunctionSearchBytes[]{ 0xF6, 0xC4, 0x05, 0x7A, 0x06, 0xDD, 0xD8, 0xD9, 0xC0, 0xEB, 0x15 };
	void *RightAnalogYAddr = (void*)SearchAndGetAddresses(0x0052E7D3, 0x0052EB03, 0x0052E423, RightAnalogYFunctionSearchBytes, sizeof(RightAnalogYFunctionSearchBytes), -0x11B);

	// Checking address pointer
	if (!RightAnalogYAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog Stick function address!";
		return nullptr;
	}

	RightAnalogYFunctionAddr = (DWORD*)RightAnalogYAddr;

	return RightAnalogYFunctionAddr;
}

DWORD *GetUpdateMousePositionFunctionPointer()
{
	if (UpdateMousePositionFunctionAddr)
	{
		return UpdateMousePositionFunctionAddr;
	}

	// Get Update Mouse Position function address
	constexpr BYTE UpdateMousePositionFunctionSearchBytes[]{ 0x89, 0x74, 0x24, 0x58, 0x89, 0x74 };
	void *UpdateMousePositionAddr = (void*)SearchAndGetAddresses(0x004588F1, 0x00458B51, 0x00458B51, UpdateMousePositionFunctionSearchBytes, sizeof(UpdateMousePositionFunctionSearchBytes), 0x40);

	// Checking address pointer
	if (!UpdateMousePositionAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Update Mouse Position function address!";
		return nullptr;
	}

	UpdateMousePositionFunctionAddr = (DWORD*)UpdateMousePositionAddr;

	return UpdateMousePositionFunctionAddr;
}

BYTE GetSearchViewFlag()
{
	BYTE *pCancelButton = GetSearchViewFlagPointer();

	return (pCancelButton) ? *pCancelButton : 0;
}

BYTE *GetSearchViewFlagPointer()
{
	if (SearchViewFlagAddr)
	{
		return SearchViewFlagAddr;
	}

	// Get Search View Flag address
	constexpr BYTE SearchViewFlagSearchBytes[]{ 0x83, 0xC4, 0x08, 0x5F, 0x5E, 0x83, 0xC8, 0x20, 0x5D };
	BYTE *SearchViewFlag = (BYTE*)ReadSearchedAddresses(0x0051F7A2, 0x0051FAD2, 0x0051F3F2, SearchViewFlagSearchBytes, sizeof(SearchViewFlagSearchBytes), 0x0A);

	// Checking address pointer
	if (!SearchViewFlag)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Search View Flag address!";
		return nullptr;
	}

	SearchViewFlagAddr = (BYTE*)((DWORD)SearchViewFlag + 1);

	return SearchViewFlagAddr;
}

int32_t GetEnableInput()
{
	int32_t *pEnableInput = GetEnableInputPointer();

	return (pEnableInput) ? *pEnableInput : 0;
}

int32_t *GetEnableInputPointer()
{
	if (EnableInputAddr)
	{
		return EnableInputAddr;
	}

	// Get EnableInput address
	constexpr BYTE EnableInputSearchBytes[]{ 0xC1, 0xE0, 0x04, 0x03, 0xC1, 0x8B, 0x40, 0x0C, 0x8B, 0xF0 };
	int32_t *EnableInput = (int32_t*)ReadSearchedAddresses(0x0048C005, 0x0048C2A5, 0x0048C4B5, EnableInputSearchBytes, sizeof(EnableInputSearchBytes), -0x12);

	// Checking address pointer
	if (!EnableInput)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find EnableInput address!";
		return nullptr;
	}

	EnableInputAddr = (int32_t*)((DWORD)EnableInput);

	return EnableInputAddr;
}

BYTE GetAnalogX()
{
	BYTE *pCancelButton = GetAnalogXPointer();

	return (pCancelButton) ? *pCancelButton : 0;
}

BYTE *GetAnalogXPointer()
{
	if (AnalogXAddr)
	{
		return AnalogXAddr;
	}

	// Get Analog X address
	constexpr BYTE AnalogXSearchBytes[]{ 0X83, 0xC4, 0x10, 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x0F, 0x8A, 0x96 };
	BYTE *AnalogX = (BYTE*)ReadSearchedAddresses(0x0054EFDF, 0x0054F30F, 0x0054EC2F, AnalogXSearchBytes, sizeof(AnalogXSearchBytes), 0x26);

	// Checking address pointer
	if (!AnalogX)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog X address!";
		return nullptr;
	}

	AnalogXAddr = (BYTE*)((DWORD)AnalogX);

	return AnalogXAddr;
}

BYTE GetControlType()
{
	BYTE *pCancelButton = GetControlTypePointer();

	return (pCancelButton) ? *pCancelButton : 0;
}

BYTE *GetControlTypePointer()
{
	if (ControlTypeAddr)
	{
		return ControlTypeAddr;
	}

	// Get Control Type address
	constexpr BYTE ControlTypeSearchBytes[]{ 0x83, 0xC4, 0x10, 0x68, 0xB8, 0x01, 0x00, 0x00, 0x6A, 0x1E };
	BYTE *ControlType = (BYTE*)ReadSearchedAddresses(0x004676E9, 0x00467989, 0x00467B99, ControlTypeSearchBytes, sizeof(ControlTypeSearchBytes), 0x16);

	// Checking address pointer
	if (!ControlType)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Control Type address!";
		return nullptr;
	}

	ControlTypeAddr = (BYTE*)((DWORD)ControlType);

	return ControlTypeAddr;
}

BYTE GetRunOption()
{
	BYTE *pCancelButton = GetRunOptionPointer();

	return (pCancelButton) ? *pCancelButton : 0;
}

BYTE *GetRunOptionPointer()
{
	if (RunOptionAddr)
	{
		return RunOptionAddr;
	}

	// Get Run Option address
	constexpr BYTE RunOptionSearchBytes[]{ 0x83, 0xC4, 0x10, 0x68, 0xB8, 0x01, 0x00, 0x00, 0x6A, 0x1E };
	BYTE *RunOption = (BYTE*)ReadSearchedAddresses(0x004676E9, 0x00467989, 0x00467B99, RunOptionSearchBytes, sizeof(RunOptionSearchBytes), 0x16);

	// Checking address pointer
	if (!RunOption)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Run Option address!";
		return nullptr;
	}

	RunOptionAddr = (BYTE*)((DWORD)RunOption) - 0x06;

	return RunOptionAddr;
}

BYTE GetNumKeysWeaponBindStart()
{
	BYTE *pNumKeysWeaponBindStart = GetNumKeysWeaponBindStartPointer();

	return (pNumKeysWeaponBindStart) ? *pNumKeysWeaponBindStart : 0;
}

// Get start of Numpad weapon keybinds  address
BYTE *GetNumKeysWeaponBindStartPointer()
{
	if (NumKeysWeaponBindStartAddr)
	{
		return NumKeysWeaponBindStartAddr;
	}

	// Get address for start of Numpad weapon keybinds 
	constexpr BYTE FullscreenImageSearchBytes[]{ 0x83, 0xC0, 0x08, 0x3D, 0xB0, 0x00, 0x00, 0x00, 0x7C, 0xDE, 0x33, 0xC0, 0x8B };
	NumKeysWeaponBindStartAddr = (BYTE*)ReadSearchedAddresses(0x005AECE8, 0x005AF598, 0x005AEEB8, FullscreenImageSearchBytes, sizeof(FullscreenImageSearchBytes), 0x20);

	// Checking address pointer
	if (!NumKeysWeaponBindStartAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find start of Numpad weapon keybinds memory address!";
		return nullptr;
	}

	NumKeysWeaponBindStartAddr += 0x44;

	return NumKeysWeaponBindStartAddr;
}

BYTE GetTalkShowHostState()
{
	BYTE *pTalkShowHostState = GetTalkShowHostStatePointer();

	return (pTalkShowHostState) ? *pTalkShowHostState : 0;
}

// Get talk show host state address
BYTE *GetTalkShowHostStatePointer()
{
	if (TalkShowHostStateAddr)
	{
		return TalkShowHostStateAddr;
	}

	// Get address for start of Numpad weapon keybinds 
	constexpr BYTE FullscreenImageSearchBytes[]{ 0x83, 0xC4, 0x04, 0x83, 0xF8, 0xFF, 0x75, 0x03 };
	TalkShowHostStateAddr = (BYTE*)ReadSearchedAddresses(0x00517DFA, 0x0051812A, 0x00517A4A, FullscreenImageSearchBytes, sizeof(FullscreenImageSearchBytes), 0x11);

	// Checking address pointer
	if (!TalkShowHostStateAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find talk show host state memory address!";
		return nullptr;
	}

	TalkShowHostStateAddr += 0x08;

	return TalkShowHostStateAddr;
}

BYTE GetBoatFlag()
{
	BYTE *pActionButton = GetBoatFlagPointer();

	return (pActionButton) ? *pActionButton : 0;
}

BYTE *GetBoatFlagPointer()
{
	if (BoatFlagAddr)
	{
		return BoatFlagAddr;
	}

	// Get Boat Flag address
	constexpr BYTE ActionButtonSearchBytes[]{ 0x66, 0x89, 0x72, 0xFE };
	BYTE *BoatFlag = (BYTE*)ReadSearchedAddresses(0x005A58D1, 0x005A6181, 0x005A5AA1, ActionButtonSearchBytes, sizeof(ActionButtonSearchBytes), -0x3C);

	// Checking address pointer
	if (!BoatFlag)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Boat Flag address!";
		return nullptr;
	}

	BoatFlagAddr = (BYTE*)((DWORD)BoatFlag + 0xB2);

	return BoatFlagAddr;
}

int32_t GetIsWritingQuicksave()
{
	int32_t *pIsWritingQuicksave = GetIsWritingQuicksavePointer();

	return (pIsWritingQuicksave) ? *pIsWritingQuicksave : 0;
}

int32_t *GetIsWritingQuicksavePointer()
{
	if (IsWritingQuicksaveAddr)
	{
		return IsWritingQuicksaveAddr;
	}

	// Get IsWritingQuicksave address
	constexpr BYTE IsWritingQuicksaveSearchBytes[]{ 0x85, 0xC0, 0x74, 0x18, 0x89 };
	int32_t *IsWritingQuicksave = (int32_t*)ReadSearchedAddresses(0x00402530, 0x00402530, 0x00402530, IsWritingQuicksaveSearchBytes, sizeof(IsWritingQuicksaveSearchBytes), 0x06);

	// Checking address pointer
	if (!IsWritingQuicksave)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find IsWritingQuicksave address!";
		return nullptr;
	}

	IsWritingQuicksaveAddr = (int32_t*)((DWORD)IsWritingQuicksave);

	return IsWritingQuicksaveAddr;
}

int32_t GetTextAddr()
{
	int32_t *pTextAddr = GetTextAddrPointer();

	return (pTextAddr) ? *pTextAddr : 0;
}

int32_t *GetTextAddrPointer()
{
	if (TextAddrAddr)
	{
		return TextAddrAddr;
	}

	// Get TextAddr address
	constexpr BYTE TextAddrSearchBytes[]{ 0x85, 0xC0, 0x74, 0x18, 0x89 };
	int32_t *TextAddr = (int32_t*)ReadSearchedAddresses(0x00402530, 0x00402530, 0x00402530, TextAddrSearchBytes, sizeof(TextAddrSearchBytes), 0x06);

	// Checking address pointer
	if (!TextAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find TextAddr address!";
		return nullptr;
	}

	TextAddrAddr = (int32_t*)((DWORD)TextAddr + 0x04);

	return TextAddrAddr;
}

float *GetWaterAnimationSpeedPointer()
{
	if (WaterAnimationSpeedAddr)
	{
		return WaterAnimationSpeedAddr;
	}
	
	// Get Water Animation Speed address
	constexpr BYTE WaterAnimationSpeedSearchBytes[]{ 0x88, 0x5E, 0x08, 0xC7, 0x46, 0x0C, 0x00, 0x00, 0x20, 0x41 };
	float *WaterAnimationSpeed = (float*)ReadSearchedAddresses(0x004D4300, 0x004D45B0, 0x004D3E70, WaterAnimationSpeedSearchBytes, sizeof(WaterAnimationSpeedSearchBytes), -0x28);

	// Checking address pointer
	if (!WaterAnimationSpeed)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Water Animation Speed address!";
		return nullptr;
	}

	WaterAnimationSpeedAddr = (float*)((DWORD)WaterAnimationSpeed);

	return WaterAnimationSpeedAddr;
}

int16_t *GetFlashlightOnSpeedPointer()
{
	if (FlashlightOnSpeedAddr)
	{
		return FlashlightOnSpeedAddr;
	}
	
	// Get Items Collected address
	constexpr BYTE FlashlightOnSpeedSearchBytes[]{ 0x74, 0x11, 0x6A, 0x00, 0xE8, 0xAD, 0x79 };
	int16_t *FlashlightOnSpeed = (int16_t*)SearchAndGetAddresses(0x0050A59A, 0x0050A8CA, 0x0050A8CA, FlashlightOnSpeedSearchBytes, sizeof(FlashlightOnSpeedSearchBytes), 0x23);

	// Checking address pointer
	if (!FlashlightOnSpeed)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Flashlight On Speed address!";
		return nullptr;
	}

	FlashlightOnSpeedAddr = (int16_t*)((DWORD)FlashlightOnSpeed);

	return FlashlightOnSpeedAddr;
}

float *GetLowHealthIndicatorFlashSpeedPointer()
{
	if (LowHealthIndicatorFlashSpeedAddr)
	{
		return LowHealthIndicatorFlashSpeedAddr;
	}

	// Get Water Animation Speed address
	constexpr BYTE LowHealthIndicatorFlashSpeedSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x75, 0x18, 0xC7, 0x05 };
	float *LowHealthIndicatorFlashSpeed = (float*)ReadSearchedAddresses(0x00476323, 0x004765C3, 0x004767D3, LowHealthIndicatorFlashSpeedSearchBytes, sizeof(LowHealthIndicatorFlashSpeedSearchBytes), 0x13);

	// Checking address pointer
	if (!LowHealthIndicatorFlashSpeed)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Low Health Indicator Speed address!";
		return nullptr;
	}

	LowHealthIndicatorFlashSpeedAddr = (float*)((DWORD)LowHealthIndicatorFlashSpeed);

	return LowHealthIndicatorFlashSpeedAddr;
}

float *GetStaircaseFlamesLightingPointer()
{
	if (StaircaseFlamesLightingSpeedAddr)
	{
		return StaircaseFlamesLightingSpeedAddr;
	}

	// Get Staircase Flames Lighting Speed address
	constexpr BYTE StaircaseFlamesLightingSpeedSearchBytes[]{ 0x8D, 0x04, 0x0A, 0x99, 0x52, 0x50, 0x33, 0xDB };
	float *StaircaseFlamesLightingSpeed = (float*)ReadSearchedAddresses(0x00576CE1, 0x00577591, 0x00576EB1, StaircaseFlamesLightingSpeedSearchBytes, sizeof(StaircaseFlamesLightingSpeedSearchBytes), 0x131);

	// Checking address pointer
	if (!StaircaseFlamesLightingSpeed)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Staircase Flames Lighting Speed address!";
		return nullptr;
	}

	StaircaseFlamesLightingSpeedAddr = (float*)((DWORD)StaircaseFlamesLightingSpeed);

	return StaircaseFlamesLightingSpeedAddr;
}

float *GetWaterLevelLoweringStepsPointer()
{
	if (WaterLevelLoweringStepsAddr)
	{
		return WaterLevelLoweringStepsAddr;
	}

	// Get Water Level Lowering Steps address
	constexpr BYTE WaterLevelLoweringStepsSearchBytes[]{ 0x83, 0xC4, 0x08, 0xA9, 0x00, 0x00, 0x08, 0x00, 0xB9, 0x00, 0x00, 0x10, 0x00 };
	float *WaterLevelLoweringSteps = (float*)ReadSearchedAddresses(0x004E317C, 0x004E342C, 0x004E2CEC, WaterLevelLoweringStepsSearchBytes, sizeof(WaterLevelLoweringStepsSearchBytes), 0x1A);

	// Checking address pointer
	if (!WaterLevelLoweringSteps)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Staircase Water Level Lowering Steps address!";
		return nullptr;
	}

	WaterLevelLoweringStepsAddr = (float*)((DWORD)WaterLevelLoweringSteps);

	return WaterLevelLoweringStepsAddr;
}

float *GetWaterLevelRisingStepsPointer()
{
	if (WaterLevelRisingStepsAddr)
	{
		return WaterLevelRisingStepsAddr;
	}

	// Get Water Level Rising Steps address
	constexpr BYTE WaterLevelRisingStepsSearchBytes[]{ 0xE8, 0x30, 0xDB, 0xFE, 0xFF };
	float *WaterLevelRisingSteps = (float*)ReadSearchedAddresses(0x004E3F7B, 0x004E422B, 0x004E3AEB, WaterLevelRisingStepsSearchBytes, sizeof(WaterLevelRisingStepsSearchBytes), 0x1D);

	// Checking address pointer
	if (!WaterLevelRisingSteps)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Staircase Water Level Rising Steps address!";
		return nullptr;
	}

	WaterLevelRisingStepsAddr = (float*)((DWORD)WaterLevelRisingSteps);

	return WaterLevelRisingStepsAddr;
}

float *GetBugRoomFlashlightFixPointer()
{
	if (BugRoomFlashlightFixAddr)
	{
		return BugRoomFlashlightFixAddr;
	}

	// Get Bug Room Flashlight Fix Steps address
	constexpr BYTE BugRoomFlashlightFixSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x7A, 0x15, 0xC7 };
	float *BugRoomFlashlightFix = (float*)ReadSearchedAddresses(0x0050AA78, 0x0050ADA8, 0x0050A6C8, BugRoomFlashlightFixSearchBytes, sizeof(BugRoomFlashlightFixSearchBytes), -0x10);

	// Checking address pointer
	if (!BugRoomFlashlightFix)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Bug Room Flashlight Fix address!";
		return nullptr;
	}

	BugRoomFlashlightFixAddr = (float*)((DWORD)BugRoomFlashlightFix);

	return BugRoomFlashlightFixAddr;
}

uint8_t *GetSixtyFPSFMVFixPointer()
{
	if (SixtyFPSFMVFixAddr)
	{
		return SixtyFPSFMVFixAddr;
	}

	// Get Sixty FPS FMV Fix address
	constexpr BYTE SixtyFPSFMVFixSearchBytes[]{ 0xE8, 0x4C, 0xF6, 0xFF, 0xFF, 0xE8 };
	uint8_t *SixtyFPSFMVFix = (uint8_t*)SearchAndGetAddresses(0x0043E4DF, 0x0043E69F, 0x0043E69F, SixtyFPSFMVFixSearchBytes, sizeof(SixtyFPSFMVFixSearchBytes), -0x74F);

	// Checking address pointer
	if (!SixtyFPSFMVFix)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Sixty FPS FMV Fix address!";
		return nullptr;
	}

	SixtyFPSFMVFix = (uint8_t*)((DWORD)SixtyFPSFMVFix);

	return SixtyFPSFMVFix;
}

uint8_t *GetGrabDamagePointer()
{
	if (GrabDamageAddr)
	{
		return GrabDamageAddr;
	}

	// Get Sixty Grab Damage address
	constexpr BYTE GrabDamageSearchBytes[]{ 0x89, 0xBE, 0x1C, 0x01, 0x00, 0x00, 0xD9, 0x86 };
	uint8_t *GrabDamage = (uint8_t*)SearchAndGetAddresses(0x005359BC, 0x00535CEC, 0x0053560C, GrabDamageSearchBytes, sizeof(GrabDamageSearchBytes), 0x19);

	// Checking address pointer
	if (!GrabDamage)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Grab Damage address!";
		return nullptr;
	}

	GrabDamage = (uint8_t*)((DWORD)GrabDamage);

	return GrabDamage;
}

float GetFrametime()
{
	float *pFrameTime = GetFrametimePointer();

	return (pFrameTime) ? *pFrameTime : 0;
}

float *GetFrametimePointer()
{
	if (FrametimeAddr)
	{
		return FrametimeAddr;
	}

	// Get Frametime address
	constexpr BYTE FrametimeSearchBytes[]{ 0x68, 0xFF, 0x00, 0x00, 0x00, 0xEB, 0x02 };
	float *Frametime = (float*)ReadSearchedAddresses(0x0044770E, 0x004478AE, 0x004478AE, FrametimeSearchBytes, sizeof(FrametimeSearchBytes), 0xE2);

	// Checking address pointer
	if (!Frametime)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Frametime  address!";
		return nullptr;
	}

	FrametimeAddr = (float*)((DWORD)Frametime);

	return FrametimeAddr;
}

DWORD *GetMeatLockerFogFixOnePointer()
{
	if (MeatLockerFogFixOneAddr)
	{
		return MeatLockerFogFixOneAddr;
	}

	// Get Meat Locker Fog Fix Address
	constexpr BYTE MeatLockerFogFixOneSearchBytes[]{ 0xE8, 0xAC, 0xFB, 0xFF, 0xFF, 0x83 };
	MeatLockerFogFixOneAddr = (DWORD*)SearchAndGetAddresses(0x00489E7F, 0x0048A11F, 0x0048A32F, MeatLockerFogFixOneSearchBytes, sizeof(MeatLockerFogFixOneSearchBytes), 0x0C);

	// Checking address pointer
	if (!MeatLockerFogFixOneAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Meat Locker Fog Fix memory address!";
		return nullptr;
	}

	return MeatLockerFogFixOneAddr;
}

DWORD *GetMeatLockerFogFixTwoPointer()
{
	if (MeatLockerFogFixTwoAddr)
	{
		return MeatLockerFogFixTwoAddr;
	}

	// Get Meat Locker Fog Fix Address
	constexpr BYTE MeatLockerFogFixTwoSearchBytes[]{ 0xE8, 0xAC, 0xFB, 0xFF, 0xFF, 0x83 };
	MeatLockerFogFixTwoAddr = (DWORD*)SearchAndGetAddresses(0x00489E7F, 0x0048A11F, 0x0048A32F, MeatLockerFogFixTwoSearchBytes, sizeof(MeatLockerFogFixTwoSearchBytes), 0x1D);

	// Checking address pointer
	if (!MeatLockerFogFixTwoAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Meat Locker Fog Fix memory address!";
		return nullptr;
	}

	return MeatLockerFogFixTwoAddr;
}

DWORD *GetMeatLockerHangerFixOnePointer()
{
	if (MeatLockerHangerFixOneAddr)
	{
		return MeatLockerHangerFixOneAddr;
	}

	// Get Meat Locker Hanger Fix Address
	constexpr BYTE MeatLockerHangerFixOneSearchBytes[]{ 0xD9, 0x44, 0x24, 0x18, 0xD8, 0x86 };
	MeatLockerHangerFixOneAddr = (DWORD*)SearchAndGetAddresses(0x004B240E, 0x004B26BE, 0x004B1F7E, MeatLockerHangerFixOneSearchBytes, sizeof(MeatLockerHangerFixOneSearchBytes), 0x18);

	// Checking address pointer
	if (!MeatLockerHangerFixOneAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Meat Locker Hanger Fix memory address!";
		return nullptr;
	}

	return MeatLockerHangerFixOneAddr;
}

DWORD *GetMeatLockerHangerFixTwoPointer()
{
	if (MeatLockerHangerFixTwoAddr)
	{
		return MeatLockerHangerFixTwoAddr;
	}

	// Get Meat Locker Hanger Fix Address
	constexpr BYTE MeatLockerHangerFixTwoSearchBytes[]{ 0xD9, 0x44, 0x24, 0x18, 0xD8, 0x86 };
	MeatLockerHangerFixTwoAddr = (DWORD*)SearchAndGetAddresses(0x004B240E, 0x004B26BE, 0x004B1F7E, MeatLockerHangerFixTwoSearchBytes, sizeof(MeatLockerHangerFixTwoSearchBytes), 0x86);

	// Checking address pointer
	if (!MeatLockerHangerFixTwoAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Meat Locker Hanger Fix memory address!";
		return nullptr;
	}

	return MeatLockerHangerFixTwoAddr;
}

BYTE *GetClearTextPointer()
{
	if (ClearTextAddr)
	{
		return ClearTextAddr;
	}

	// Get Clear Text address
	constexpr BYTE ClearTextSearchBytes[]{ 0x00, 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x66, 0x83, 0x3D };
	BYTE *ClearText = (uint8_t*)ReadSearchedAddresses(0x0047EB59, 0x0047EDF9, 0x0047F009, ClearTextSearchBytes, sizeof(ClearTextSearchBytes), 0x0A);

	// Checking address pointer
	if (!ClearText)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Clear Text address!";
		return nullptr;
	}

	ClearText = (BYTE*)((DWORD)ClearText);

	return ClearText;
}

float *GetMeetingMariaCutsceneFogCounterOnePointer()
{
	if (MeetingMariaCutsceneFogCounterOneAddr)
	{
		return MeetingMariaCutsceneFogCounterOneAddr;
	}

	// Get MeetingMariaCutsceneFogCounterOne address
	constexpr BYTE MeetingMariaCutsceneFogCounterOneSearchBytes[]{ 0x83, 0xC4, 0x04, 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x7A, 0x1C };
	float *MeetingMariaCutsceneFogCounterOne = (float*)ReadSearchedAddresses(0x00594593, 0x00594E43, 0x00594763, MeetingMariaCutsceneFogCounterOneSearchBytes, sizeof(MeetingMariaCutsceneFogCounterOneSearchBytes), -0x04);

	// Checking address pointer
	if (!MeetingMariaCutsceneFogCounterOne)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find MeetingMariaCutsceneFogCounterOne  address!";
		return nullptr;
	}

	MeetingMariaCutsceneFogCounterOneAddr = (float*)((DWORD)MeetingMariaCutsceneFogCounterOne);

	return MeetingMariaCutsceneFogCounterOneAddr;
}

float *GetMeetingMariaCutsceneFogCounterTwoPointer()
{
	if (MeetingMariaCutsceneFogCounterTwoAddr)
	{
		return MeetingMariaCutsceneFogCounterTwoAddr;
	}

	// Get MeetingMariaCutsceneFogCounterTwo address
	constexpr BYTE MeetingMariaCutsceneFogCounterTwoSearchBytes[]{ 0x83, 0xC4, 0x04, 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x7A, 0x1C };
	float *MeetingMariaCutsceneFogCounterTwo = (float*)ReadSearchedAddresses(0x00594593, 0x00594E43, 0x00594763, MeetingMariaCutsceneFogCounterTwoSearchBytes, sizeof(MeetingMariaCutsceneFogCounterTwoSearchBytes), -0x04);

	// Checking address pointer
	if (!MeetingMariaCutsceneFogCounterTwo)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find MeetingMariaCutsceneFogCounterTwo  address!";
		return nullptr;
	}

	MeetingMariaCutsceneFogCounterTwoAddr = (float*)((DWORD)MeetingMariaCutsceneFogCounterTwo - 0x04);

	return MeetingMariaCutsceneFogCounterTwoAddr;
}

float *GetRPTClosetCutsceneMannequinDespawnPointer()
{
	if (RPTClosetCutsceneMannequinDespawnAddr)
	{
		return RPTClosetCutsceneMannequinDespawnAddr;
	}

	// Get RPTClosetCutsceneMannequinDespawn address
	constexpr BYTE RPTClosetCutsceneMannequinDespawnSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x7A, 0x07, 0xB8, 0x03, 0x00, 0x00, 0x00 };
	float *RPTClosetCutsceneMannequinDespawn = (float*)ReadSearchedAddresses(0x00539E57, 0x0053A187, 0x00539AA7, RPTClosetCutsceneMannequinDespawnSearchBytes, sizeof(RPTClosetCutsceneMannequinDespawnSearchBytes), -0x04);

	// Checking address pointer
	if (!RPTClosetCutsceneMannequinDespawn)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find RPTClosetCutsceneMannequinDespawn  address!";
		return nullptr;
	}

	RPTClosetCutsceneMannequinDespawnAddr = (float*)((DWORD)RPTClosetCutsceneMannequinDespawn);

	return RPTClosetCutsceneMannequinDespawnAddr;
}

float *GetRPTClosetCutsceneBlurredBarsDespawnPointer()
{
	if (RPTClosetCutsceneBlurredBarsDespawnAddr)
	{
		return RPTClosetCutsceneBlurredBarsDespawnAddr;
	}

	// Get RPTClosetCutsceneBlurredBarsDespawn address
	constexpr BYTE RPTClosetCutsceneBlurredBarsDespawnSearchBytes[]{ 0xC7, 0x44, 0x24, 0x50, 0x00, 0x00, 0x80, 0x3F, 0xC7, 0x44, 0x24, 0x54, 0x00, 0x00, 0x00, 0x00 };
	float *RPTClosetCutsceneBlurredBarsDespawn = (float*)ReadSearchedAddresses(0x005995CF, 0x00599E7F, 0x0059979F, RPTClosetCutsceneBlurredBarsDespawnSearchBytes, sizeof(RPTClosetCutsceneBlurredBarsDespawnSearchBytes), 0x4F);

	// Checking address pointer
	if (!RPTClosetCutsceneBlurredBarsDespawn)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find RPTClosetCutsceneBlurredBarsDespawn  address!";
		return nullptr;
	}

	RPTClosetCutsceneBlurredBarsDespawnAddr = (float*)((DWORD)RPTClosetCutsceneBlurredBarsDespawn);

	return RPTClosetCutsceneBlurredBarsDespawnAddr;
}
