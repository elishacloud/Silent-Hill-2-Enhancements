#pragma once

#include "Wrappers\dinput8\dinput8wrapper.h"
#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\hooking.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Patches\Patches.h"
#include "Common\Utils.h"
#include <chrono>
#include <bitset>

#define KEY_SET   0x80
#define KEY_CLEAR 0x00

typedef enum _EVENT_INDEX {
	EVENT_LOAD_SCR,
	EVENT_LOAD_ROOM,
	EVENT_MAIN_MENU,
	EVENT_IN_GAME = 0x04,
	EVENT_MAP,
	EVENT_INVENTORY,
	EVENT_OPTION_FMV,
	EVENT_MEMO_LIST,
	EVENT_SAVE_SCREEN,
	EVENT_GAME_RESULT_ONE,
	EVENT_GAME_RESULT_TWO,
	EVENT_COMING_SOON,
	EVENT_GAME_OVER,
	EVENT_FMV = 0x0F,
	EVENT_PAUSE_MENU,
} EVENT_INDEX;

typedef enum _CONTROL_TYPE {
	ROTATIONAL_CONTROL,
	DIRECTIONAL_CONTROL,
} CONTROL_TYPE;

typedef enum _RUN_SETTING {
	OPT_WALK,
	OPT_ANALOG,
	OPT_RUN,
} RUN_SETTING;

typedef enum _KEY_INDEXES
{
	KEY_TURN_LEFT,
	KEY_TURN_RIGHT,
	KEY_MOVE_FORWARDS,
	KEY_MOVE_BACKWARDS,
	KEY_STRAFE_LEFT,
	KEY_STRAFE_RIGHT,
	KEY_ACTION,
	KEY_CANCEL,
	KEY_SKIP,
	KEY_RUN,
	KEY_SEARCH_VIEW,
	KEY_INVENTORY,
	KEY_FLASHLIGHT,
	KEY_MAP,
	KEY_USE_HEALTH,
	KEY_READY_WEAPON,
	KEY_NEXT_WEAPON,
	KEY_PREV_WEAPON,
	KEY_RELOAD,
	KEY_UNEQUIP_WEAPON,
	KEY_QUICK_SAVE,
	KEY_QUICK_LOAD,
} KEY_INDEXES;

struct AnalogStick {
	int8_t XAxis = 0;
	int8_t YAxis = 0;

	void AddXValue(int Value)
	{
		int temp = XAxis + Value;
		if (temp > 126)
			XAxis = 126;
		else if (temp < -126)
			XAxis = -126;
		else
			XAxis = temp;
	}

	void AddYValue(int Value)
	{
		int temp = YAxis + Value;
		if (temp > 126)
			YAxis = 126;
		else if (temp < -126)
			YAxis = -126;
		else
			YAxis = temp;
	}

	void Recenter()
	{
		XAxis = 0;
		YAxis = 0;
	}

	bool IsCentered()
	{
		return XAxis == 0 && YAxis == 0;
	}
};

struct Input {
	bool State = false;
	bool LastState = false;
	bool Holding = false;

	void UpdateHolding()
	{
		Holding = LastState && State;
		LastState = State;
	}

	void UpdateHoldingByValue(bool Value)
	{
		Holding = LastState && Value;
		LastState = Value;
	}

	void ToggleState()
	{
		State = !State;
	}
};

class InputTweaks
{
private:
	LPDIRECTINPUTDEVICE8A KeyboardInterfaceAddress = nullptr;
	LPDIRECTINPUTDEVICE8A MouseInterfaceAddress = nullptr;

	BYTE* KeyboardData = nullptr;
	LPDIDEVICEOBJECTDATA MouseData = nullptr;
	DWORD MouseDataSize = 0;

	int DefaultNumberKeyBinds[10] = {0xB, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA};

	bool OverrideSprint;

	bool IsKeyPressed(int KeyIndex);
	void ReadMouseButtons();
	void ClearKey(int KeyIndex);
	void SetKey(int KeyIndex);
	int32_t GetMouseRelXChange();
	int32_t GetMouseRelYChange();
	void CheckNumberKeyBinds();

	// Fixes to pressing right click to skip cutscenes that aren't technically considered cutscenes by the game
	bool HotelFix();
	bool JamesVaultingBuildingsFix();
	bool RosewaterParkFix();
	bool HospitalMonologueFix();
	bool FleshRoomFix();
	bool SetRMBAimFunction();
	bool IsMovementPressed();
	bool IsInFullScreenImageEvent();

	bool GetAnalogStringAddr();

public:
	void SetKeyboardInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	void SetMouseInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	void RemoveAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	
	void TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData);
	void TweakGetDeviceData(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
	float GetMouseAnalogX();
	float GetForwardAnalog();
	float GetTurningAnalog();
	void ClearMouseInputs();
	std::string GetRightClickState();
	std::string GetToggleSprintState();
	void SetOverrideSprint();
	void ClearOverrideSprint();

	// Additional fix for cutscenes
	bool ElevatorFix();
};

class KeyBindsHandler
{
private:
	BYTE* KeyBindsAddr;

public:
	BYTE GetKeyBind(int KeyIndex);
	BYTE* GetKeyBindsPointer();
};

extern InputTweaks InputTweaksRef;