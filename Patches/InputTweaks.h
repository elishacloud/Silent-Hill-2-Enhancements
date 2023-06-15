/**
* Copyright (C) 2023 mercury501
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

	// Fixes to pressing right click to skip cutscenes that aren't technically considered cutscenes by the game
	bool HotelFix();
	bool JamesVaultingBuildingsFix();
	bool RosewaterParkFix();
	bool HospitalMonologueFix();
	bool FleshRoomFix();
	bool SetRMBAimFunction();
	bool IsMovementPressed();

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

	void CheckNumberKeyBinds();
	bool GetAnalogStringAddr();

	bool GetRMBState();
	bool GetLMBState();

	void InitializeHitboxes(float AspectRatio);

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
	int rows;
	int columns;

	float AspectRatio;
	float ConstantAspectRatio = 16.f / 9.f;

	int GetNormalizedHorizontal(int size)
	{
#pragma warning(disable : 4244)
		return (size * AspectRatio) / ConstantAspectRatio;
	}

public:
	Hitboxes(){}
	Hitboxes(int top, int left, int height, int width, int rows, int columns, float AspectRatio)
	{
		this->AspectRatio = AspectRatio;

		this->top = top;
		this->left = GetNormalizedHorizontal(left);
		this->height = height;
		this->width = GetNormalizedHorizontal(width);
		this->rows = rows;
		this->columns = columns;
	}
	Hitboxes(int top, int left, int height, int width, int rows, int columns)
	{
		this->AspectRatio = 16.f / 9.f;

		this->top = top;
		this->left = GetNormalizedHorizontal(left);
		this->height = height;
		this->width = GetNormalizedHorizontal(width);
		this->rows = rows;
		this->columns = columns;
	}

	int GetTop() { return this->top; }
	int GetHeight() { return this->height; }
	int GetLeft() { return this->left; }
	int GetWidth() { return this->width; }
	int GetRowsCount() { return this->rows; }
	int GetRight() { return this->left + (this->columns * this->width); }
	int GetBottom() { return this->top + (this->rows * this->height); }

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
		return ((this->GetHitbox(MemoNumber).GetRowsCount() - MemoNumber) *
			this->Odd.GetHeight() / 2);
	}

public:
	MemoHitboxes(int EvenTop, int OddTop, int left, int height, int width)
	{
		this->Odd = Hitboxes(OddTop, left, height, width, 11, 1);
		this->Even = Hitboxes(EvenTop, left, height, width, 10, 1);
	}
	MemoHitboxes(int EvenTop, int OddTop, int left, int height, int width, float AspectRatio)
	{
		this->Odd = Hitboxes(OddTop, left, height, width, 11, 1, AspectRatio);
		this->Even = Hitboxes(EvenTop, left, height, width, 10, 1, AspectRatio);
	}

	int GetEnabledVerticalIndex(int MousePos, int MemoNumber)
	{
		return this->GetHitbox(MemoNumber).GetVerticalIndex(MousePos) -
			((this->GetHitbox(MemoNumber).GetRowsCount() - MemoNumber) / 2);
	}

	bool IsMouseVerticallyInBounds(int MouseVer, int MemoNumber)
	{
		int VOffset = this->GetVerticalOffset(MemoNumber);

		return MouseVer > this->GetHitbox(MemoNumber).GetTop() + VOffset &&
			MouseVer < this->GetHitbox(MemoNumber).GetBottom() - VOffset;
	}

	bool IsMouseHorizontallyInBounds(int MouseHor, int MemoNumber)
	{
		return MouseHor > this->GetHitbox(MemoNumber).GetLeft() &&
			MouseHor < this->GetHitbox(MemoNumber).GetRight();
	}

	bool IsMouseInBounds(int MouseHor, int MouseVer, int MemoNumber)
	{
		return this->IsMouseHorizontallyInBounds(MouseHor, MemoNumber) &&
			this->IsMouseVerticallyInBounds(MouseVer, MemoNumber);
	}

	// Functions only used when the memo list is scrolling (11+ memos)

	int ConvertHitboxValue(int SelectedHitbox) { return -SelectedHitbox + 5; }
	int GetHeight() { return this->Odd.GetHeight(); }
	int GetTop() { return this->Odd.GetTop(); }

	int GetClampedMemoIndex(int offset, int TotalMemoCount, int SelectedMemoIndex)
	{
		int step = offset > 0 ? 1 : -1;
		int CalculatedIndex = SelectedMemoIndex;
		int temp = 0;

		for (int i = 1; i <= std::abs(offset); i++)
		{
			temp = CalculatedIndex + step;

			if (temp > (TotalMemoCount - 1))
			{
				CalculatedIndex = 0;
				continue;
			}
			if (temp < 0)
			{
				CalculatedIndex = TotalMemoCount - 1;
				continue;
			}

			CalculatedIndex = temp;
		}

		return CalculatedIndex;
	}

	int IsMouseTopOrBot(int MouseHor, int MouseVer)
	{
		if (!(MouseHor > this->Odd.GetLeft() &&
			MouseHor < this->Odd.GetRight()))
			return 0;

		if (MouseVer < this->Odd.GetTop())
			return 1;
		if (MouseVer > this->Odd.GetBottom())
			return -1;

		return 0;
	}
};

class CursorPositionHandler
{
private:
	int CursorSavedXPos = 0;
	int CursorSavedYPos = 0;
public:
	void UpdateCursorPos()
	{
		if (!AutoHideMouseCursor)
			return;

		CursorSavedXPos = GetMouseHorizontalPosition();
		CursorSavedYPos = GetMouseVerticalPosition();
	}

	void RestoreCursorPos()
	{
		if (!AutoHideMouseCursor)
			return;

		*GetMouseHorizontalPositionPointer() = CursorSavedXPos;
		*GetMouseVerticalPositionPointer() = CursorSavedYPos;
	}

	void MoveCursorToOrigin()
	{
		if (!AutoHideMouseCursor)
			return;

		*GetMouseHorizontalPositionPointer() = 0;
		*GetMouseVerticalPositionPointer() = 0;
	}

	void CenterCursor()
	{
		*GetPuzzleCursorHorizontalPosPointer() = 0.0f;
		*GetPuzzleCursorVerticalPosPointer() = 0.0f;
	}
};

void DrawCursor_Hook(void);
void SetShowCursorFlag_Hook(void);

extern InputTweaks InputTweaksRef;