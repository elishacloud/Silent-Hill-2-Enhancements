/**
* Copyright (C) 2022 mercury501
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

#pragma once

#include "Wrappers\dinput8\dinput8wrapper.h"
#include "Patches\Patches.h"
#include "Common\Utils.h"
#include <chrono>
#include <bitset>

#define KEY_SET   0x80
#define KEY_CLEAR 0x00

#define FOREACH_KEY(KEY) \
        KEY(KEY_TURN_LEFT) \
        KEY(KEY_TURN_RIGHT) \
        KEY(KEY_MOVE_FORWARDS) \
        KEY(KEY_MOVE_BACKWARDS) \
        KEY(KEY_STRAFE_LEFT) \
        KEY(KEY_STRAFE_RIGHT) \
        KEY(KEY_ACTION) \
        KEY(KEY_CANCEL) \
        KEY(KEY_SKIP) \
        KEY(KEY_RUN) \
        KEY(KEY_SEARCH_VIEW) \
        KEY(KEY_INVENTORY) \
        KEY(KEY_FLASHLIGHT) \
        KEY(KEY_MAP) \
        KEY(KEY_USE_HEALTH) \
        KEY(KEY_READY_WEAPON) \
        KEY(KEY_NEXT_WEAPON) \
        KEY(KEY_PREV_WEAPON) \
        KEY(KEY_RELOAD) \
        KEY(KEY_UNEQUIP_WEAPON) \
        KEY(KEY_QUICK_SAVE) \
        KEY(KEY_QUICK_LOAD) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum _KEY_INDEXES
{
	FOREACH_KEY(GENERATE_ENUM)
} KEY_INDEXES;

static const char *KEY_NAMES[] =
{
	FOREACH_KEY(GENERATE_STRING)
};

typedef enum _EVENT_INDEX 
{
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

typedef enum _CONTROL_TYPE 
{
	ROTATIONAL_CONTROL,
	DIRECTIONAL_CONTROL,
} CONTROL_TYPE;

typedef enum _RUN_SETTING 
{
	OPT_WALK,
	OPT_ANALOG,
	OPT_RUN,
} RUN_SETTING;

struct AnalogStick 
{
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
			XAxis = (int8_t)temp;
	}

	void AddYValue(int Value)
	{
		int temp = YAxis + Value;
		if (temp > 126)
			YAxis = 126;
		else if (temp < -126)
			YAxis = -126;
		else
			YAxis = (int8_t)temp;
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

struct Input 
{
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
	LPDIRECTINPUTDEVICE8A ControllerInterfaceAddress = nullptr;

	BYTE* KeyboardData = nullptr;
	LPDIDEVICEOBJECTDATA MouseData = nullptr;
	DWORD MouseDataSize = 0;
	LPDIJOYSTATE ControllerData = nullptr;

	DIMOUSESTATE MouseState;

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

	bool GetAnalogStringAddr();

public:
	void SetKeyboardInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	void SetMouseInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	void SetControllerInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	void RemoveInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	
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

	bool GetRMBState();
	bool GetLMBState();

	bool IsInFullScreenImageEvent();

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
	BYTE GetPauseButtonBind();
};

// Hitboxes for pause and memo menu
class Hitboxes
{
private:
	int top;
	int left;
	int height;	
	int width;
	int VerticalNumber;
	int HorizontalNumber;

public:
	Hitboxes(){}
	Hitboxes(int top, int left, int height, int width, int VerticalNumber, int HorizontalNumber)
	{
		this->top = top;
		this->left = left;
		this->height = height;
		this->width = width;
		this->VerticalNumber = VerticalNumber;
		this->HorizontalNumber = HorizontalNumber;
	}

	int GetTop() { return this->top; }
	int GetHeight() { return this->height; }
	int GetLeft() { return this->left; }
	int GetWidth() { return this->width; }
	int GetVerticalNumber() { return this->VerticalNumber; }
	int GetRight() { return this->left + (this->HorizontalNumber * this->width); }
	int GetBottom() { return this->top + (this->VerticalNumber * this->height); }

	bool IsMouseInBounds(int MouseHor, int MouseVer)
	{
		return MouseHor > this->GetLeft() &&
			MouseHor < this->GetRight() &&
			MouseVer > this->GetTop() &&
			MouseVer < this->GetBottom();
	}
	int GetVerticalIndex(int MousePos) { return (MousePos - this->top) / this->height; }
	int GetHorizontalIndex(int MousePos) { return (MousePos - this->left) / this->width; }
};

class MemoHitboxes
{
private:
	Hitboxes Odd;
	Hitboxes Even;

	Hitboxes GetHitbox(int MemoNumber) 
	{ 
		return (MemoNumber % 2 != 0) ? Odd : Even; 
	}

	int GetVerticalOffset(int MemoNumber)
	{
		return ((this->GetHitbox(MemoNumber).GetVerticalNumber() - MemoNumber) *
			this->Odd.GetHeight() / 2);
	}

public:
	MemoHitboxes(int EvenTop, int OddTop, int left, int height, int width)
	{
		this->Odd = Hitboxes(OddTop, left, height, width, 11, 1);
		this->Even = Hitboxes(EvenTop, left, height, width, 10, 1);
	}

	int GetEnabledVerticalIndex(int MousePos, int MemoNumber)
	{
		//TODO remove
		AuxDebugOvlString = "\rVertical index: ";
		AuxDebugOvlString.append(std::to_string(this->GetHitbox(MemoNumber).GetVerticalIndex(MousePos) -
			((this->GetHitbox(MemoNumber).GetVerticalNumber() - MemoNumber) / 2)));

		return this->GetHitbox(MemoNumber).GetVerticalIndex(MousePos) -
			((this->GetHitbox(MemoNumber).GetVerticalNumber() - MemoNumber) / 2);
	}

	bool IsMouseInBounds(int MouseHor, int MouseVer, int MemoNumber) // TODO change to consider the memo number
	{
		int VOffset = this->GetVerticalOffset(MemoNumber);

		return MouseHor > this->GetHitbox(MemoNumber).GetLeft() &&
			MouseHor < this->GetHitbox(MemoNumber).GetRight() &&
			MouseVer > this->GetHitbox(MemoNumber).GetTop() + VOffset &&
			MouseVer < this->GetHitbox(MemoNumber).GetBottom() - VOffset;
	}
};

void DrawCursor_Hook(void);
void SetShowCursorFlag_Hook(void);

extern InputTweaks InputTweaksRef;