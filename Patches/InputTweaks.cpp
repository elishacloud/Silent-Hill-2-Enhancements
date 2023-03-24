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

#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "InputTweaks.h"

InputTweaks InputTweaksRef;
KeyBindsHandler KeyBinds;

BYTE* KeyBindsAddr = nullptr;

const int AnalogThreshold = 15;
const int InputDebounce = 50;
const float AnalogHalfTilt = 0.5f;
const float AnalogFullTilt = 1.0f;
const float FloatTolerance = 0.10f;

const int PauseMenuCursorTopThreshold = 210;
const int PauseMenuCursorBottomThreshold = 360;
const int PauseMenuVerticalHitbox = (PauseMenuCursorBottomThreshold - PauseMenuCursorTopThreshold) / 5;

const int PauseMenuQuitTopThreshold = 245;
const int PauseMenuQuitBottomThreshold = 275;
const int PauseMenuQuitLeftThreshold = 281;
const int PauseMenuQuitRightThreshold = 398;
const int PauseMenuHorizontalHitbox = (PauseMenuQuitRightThreshold - PauseMenuQuitLeftThreshold) / 2;

const int MemoListTopThreshold = 130;
const int MemoListBottomThreshold = 340;
const int MemoListLeftThreshold = 120;
const int MemoListRightThreshold = 540;
const int MemoListVerticalHitbox = (MemoListBottomThreshold - MemoListTopThreshold) / 7;

bool once = false;

auto LastMouseXRead = std::chrono::system_clock::now();
auto LastMouseYRead = std::chrono::system_clock::now();
auto LastWeaponSwap = std::chrono::system_clock::now();
auto LastMousePauseChange = std::chrono::system_clock::now();
int MouseXAxis = 0;
int MouseYAxis = 0;
long int MouseWheel = 0;

AnalogStick VirtualRightStick;

bool CheckKeyBindsFlag = false;

bool SetLMButton = false;
bool SetLeftKey = false;
bool SetRightKey = false;
bool SetUpKey = false;
bool SetDownKey = false;
bool CleanKeys = false;

Input Sprint;
Input RMB;
Input DebugCombo;
Input InfoCombo;
Input EscInput;
Input CancelInput;

int ForwardBackwardsAxis = 0;
int LeftRightAxis = 0;
bool OverrideSprint = false;

const int AutoHideCursorSeconds = 3;
auto LastCursorMovement = std::chrono::system_clock::now();
int LastCursorXPos = 0;
int LastCursorYPos = 0;
bool HideMouseCursor = false;
int CursorSavedXPos = 0;
int CursorSavedYPos = 0;

injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLYAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRYAxis;
injector::hook_back<void(__cdecl*)(void)> orgUpdateMousePosition;
injector::hook_back<void(__cdecl*)(void)> orgDrawCursor;
injector::hook_back<void(__cdecl*)(void)> orgSetShowCursorFlag;

BYTE* AnalogStringOne;
BYTE* AnalogStringTwo;
BYTE* AnalogStringThree;

uint8_t keyNotSetWarning[21] = { 0 };

void DrawCursor_Hook()
{
	if (HideMouseCursor)
		return;

	orgDrawCursor.fun();
}

void SetShowCursorFlag_Hook(void)
{
	orgSetShowCursorFlag.fun();
}

int8_t GetControllerLXAxis_Hook(DWORD* arg)
{
	// Alt Tab Rotating Fix
	if (GetForegroundWindow() != GameWindowHandle)
	{
		SetLMButton = false;
		RMB.State = false;
		return 0;
	}
		
	return orgGetControllerLXAxis.fun(arg);
}

int8_t GetControllerLYAxis_Hook(DWORD* arg)
{
	return orgGetControllerLYAxis.fun(arg);
}

int8_t GetControllerRXAxis_Hook(DWORD* arg)
{
	// Injecting the virtual analog stick for search view
	if (GetSearchViewFlag() == 0x6 && !VirtualRightStick.IsCentered() && EnableEnhancedMouse)
	{
		return VirtualRightStick.XAxis;
	}
	else
	{
		return orgGetControllerRXAxis.fun(arg);
	}
}

int8_t GetControllerRYAxis_Hook(DWORD* arg)
{
	// Injecting the virtual analog stick for search view
	if (GetSearchViewFlag() == 0x6 && !VirtualRightStick.IsCentered() && EnableEnhancedMouse)
	{
		return VirtualRightStick.YAxis;
	}
	else
	{
		return orgGetControllerRYAxis.fun(arg);
	}
}

void UpdateMousePosition_Hook()
{
	orgUpdateMousePosition.fun();

	auto Now = std::chrono::system_clock::now();

	if (GetEventIndex() == EVENT_IN_GAME && GetMenuEvent() != 0x07)
	{
		*GetMouseHorizontalPositionPointer() = 0;
		*GetMouseVerticalPositionPointer() = 0;
		HideMouseCursor = true;
	}
	else if (!HideMouseCursor &&
		(GetMouseVerticalPosition() != LastCursorYPos || GetMouseHorizontalPosition() != LastCursorXPos))
	{
		LastCursorXPos = GetMouseHorizontalPosition();
		LastCursorYPos = GetMouseVerticalPosition();

		CursorSavedXPos = LastCursorXPos;
		CursorSavedYPos = LastCursorYPos;

		LastCursorMovement = Now;
		HideMouseCursor = false;
	}
	else if (HideMouseCursor &&
		(GetMouseVerticalPosition() != 0 || GetMouseHorizontalPosition() != 0))
	{
		*GetMouseHorizontalPositionPointer() = CursorSavedXPos;
		*GetMouseVerticalPositionPointer() = CursorSavedYPos;

		LastCursorXPos = GetMouseHorizontalPosition();
		LastCursorYPos = GetMouseVerticalPosition();

		LastCursorMovement = Now;
		HideMouseCursor = false;
	}
	else if ((std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastCursorMovement).count() > AutoHideCursorSeconds * 1000))
	{
		*GetMouseHorizontalPositionPointer() = 0;
		*GetMouseVerticalPositionPointer() = 0;
		HideMouseCursor = true;
	}

	int CurrentMouseHorizontalPosition = GetMouseHorizontalPosition();
	int CurrentMouseVerticalPosition = GetMouseVerticalPosition();

	//TODO suppress warning warning C4244: '=': conversion from 'int16_t' to 'BYTE', possible loss of data
	// Handling of vertical and horizontal navigation for Pause and Memo screens
	if (GetEventIndex() == EVENT_PAUSE_MENU)
	{
		// X x Y 853 x 480
		if (CurrentMouseHorizontalPosition > PauseMenuQuitLeftThreshold &&
			CurrentMouseHorizontalPosition < PauseMenuQuitRightThreshold &&
			CurrentMouseVerticalPosition > PauseMenuCursorTopThreshold &&
			CurrentMouseVerticalPosition < PauseMenuCursorBottomThreshold &&
			*(BYTE*)0x00932030 == 0x00) // TODO is in quit submenu address
		{
			*GetPauseMenuButtonIndexPointer() = (CurrentMouseVerticalPosition - PauseMenuCursorTopThreshold) / PauseMenuVerticalHitbox;
		}

		// Pause menu quit submenu
		if (CurrentMouseHorizontalPosition > PauseMenuQuitLeftThreshold &&
			CurrentMouseHorizontalPosition < PauseMenuQuitRightThreshold &&
			CurrentMouseVerticalPosition > PauseMenuQuitTopThreshold &&
			CurrentMouseVerticalPosition < PauseMenuQuitBottomThreshold &&
			*(BYTE*)0x00932030 == 0x01)// TODO is in quit submenu address
		{
			*GetPauseMenuQuitIndexPointer() = (CurrentMouseHorizontalPosition - PauseMenuQuitLeftThreshold) / PauseMenuHorizontalHitbox;
		}
	}
	else if (GetEventIndex() == EVENT_MEMO_LIST)
	{
		//vertical 90 410 horizontal 120 540
		if (CurrentMouseHorizontalPosition > MemoListLeftThreshold &&
			CurrentMouseHorizontalPosition < MemoListRightThreshold)
		{
			if (CurrentMouseVerticalPosition < MemoListTopThreshold)
			{
				//TODO if first or last selected, move the list and jump the cursor down or up
			}
			else if (CurrentMouseVerticalPosition > MemoListBottomThreshold)
			{
				//TODO
			}
			else if (CurrentMouseVerticalPosition > MemoListTopThreshold &&
				CurrentMouseVerticalPosition < MemoListBottomThreshold)
			{
				//*(int32_t*)0x94d8ac = ((CurrentMouseVerticalPosition - MemoListTopThreshold) / MemoListVerticalHitbox) - 3; //TODO
			}
		}

	}
}

void InputTweaks::TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData)
{
	UNREFERENCED_PARAMETER(cbData);

	// Check number keybinds after exiting the Options screen
	if (GetEventIndex() == EVENT_OPTION_FMV)
		CheckKeyBindsFlag = true;
	if (GetEventIndex() != EVENT_OPTION_FMV && CheckKeyBindsFlag)
	{
		CheckKeyBindsFlag = false;
		CheckNumberKeyBinds();
	}

	// For controller
	if (ProxyInterface == ControllerInterfaceAddress)
	{
		// Save controller data
		ControllerData = (DIJOYSTATE*)lpvData;

		// Clear the the pause button if a quicksave is in progress
		if (GameLoadFix && (GetIsWritingQuicksave() == 1 || GetTextAddr() == 1))
			ControllerData->rgbButtons[KeyBinds.GetPauseButtonBind()] = KEY_CLEAR;

		// Clear controller data
		ControllerData = nullptr;
	}

	// For keyboard
	if (ProxyInterface == KeyboardInterfaceAddress)
	{
		auto Now = std::chrono::system_clock::now();
		auto DeltaMsWeaponSwap = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastWeaponSwap);

		// Save Keyboard Data
		KeyboardData = (BYTE*)lpvData;

		// Fix for ALT + TAB
		if (GetForegroundWindow() != GameWindowHandle)
		{
			SetLMButton = false;
			RMB.State = false;
		}

		// Reset Sprint toggle
		if (EnableToggleSprint && GetRunOption() != OPT_ANALOG && !OverrideSprint)
			Sprint.State = false;

		// Ignore Alt + Enter combo
		if ((IsKeyPressed(DIK_LMENU) || IsKeyPressed(DIK_RMENU)) && IsKeyPressed(DIK_RETURN) && 
			DynamicResolution && ScreenMode != EXCLUSIVE_FULLSCREEN)
		{
			ClearKey(DIK_LMENU);
			ClearKey(DIK_RMENU);
			ClearKey(DIK_RETURN);
			Logging::LogDebug() << __FUNCTION__ << " Detected ALT + ENTER...";
		}

		// Ignore Ctrl + G combo
		if (IsKeyPressed(DIK_LCONTROL) && IsKeyPressed(DIK_G) && EnableDebugOverlay)
		{
			ClearKey(DIK_LCONTROL);
			ClearKey(DIK_G);
			DebugCombo.State = true;
			Logging::LogDebug() << __FUNCTION__ << " Detected CTRL + G...";
		}
		else
		{
			DebugCombo.State = false;
		}

		// Ignore Ctrl + I combo
		if (IsKeyPressed(DIK_LCONTROL) && IsKeyPressed(DIK_I) && EnableInfoOverlay)
		{
			ClearKey(DIK_LCONTROL);
			ClearKey(DIK_I);
			InfoCombo.State = true;
			Logging::LogDebug() << __FUNCTION__ << " Detected CTRL + I...";
		}
		else
		{
			InfoCombo.State = false;
		}

		EscInput.State = IsKeyPressed(KeyBinds.GetKeyBind(KEY_SKIP));
		EscInput.UpdateHolding();
		CancelInput.State = IsKeyPressed(KeyBinds.GetKeyBind(KEY_CANCEL));
		CancelInput.UpdateHolding();

		// Clear the ESC and SKIP key if a quicksave is in progress, or if holding the button entering the result screen
		if (GameLoadFix && (GetIsWritingQuicksave() == 1 || GetTextAddr() == 1) ||
			(GetEventIndex() == EVENT_GAME_RESULT_TWO && (EscInput.Holding || CancelInput.Holding)))
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_SKIP));
			ClearKey(KeyBinds.GetKeyBind(KEY_CANCEL));
		}

		// Check if Debug Combo is held down
		DebugCombo.UpdateHolding();

		// Check if Info Combo is held down
		InfoCombo.UpdateHolding();

		// Check if RMB is held down
		RMB.UpdateHolding();

		// Check if Sprint is held down
		Sprint.UpdateHoldingByValue(IsKeyPressed(KeyBinds.GetKeyBind(KEY_RUN)));

		// Check for toggle sprint
		if (EnableToggleSprint && (GetRunOption() == OPT_ANALOG || OverrideSprint))
		{
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_RUN)) && !Sprint.Holding && GetEventIndex() == EVENT_IN_GAME)
			{
				Sprint.ToggleState();
			}
		}

		// Check for forward/backwards movement to fix boat stage
		ForwardBackwardsAxis = 0;
		LeftRightAxis = 0;

		if (((GetBoatFlag() == 0x01 && GetRoomID() == 0x0E) || GetSearchViewFlag() == 0x06) && EnableEnhancedMouse)
		{
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS)))
			{
				ForwardBackwardsAxis += 1;
				ClearKey(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS));
			}
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS)))
			{
				ForwardBackwardsAxis -= 1;
				ClearKey(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS));
			}

			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_TURN_LEFT)))
			{
				LeftRightAxis += 1;
				ClearKey(KeyBinds.GetKeyBind(KEY_TURN_LEFT));
			}
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_TURN_RIGHT)))
			{
				LeftRightAxis -= 1;
				ClearKey(KeyBinds.GetKeyBind(KEY_TURN_RIGHT));
			}

			if (GetRunOption() == OPT_ANALOG)
				OverrideSprint = true;
		}

		// Activate Overlays
		if (DebugCombo.State && !DebugCombo.Holding && EnableDebugOverlay)
			ShowDebugOverlay = !ShowDebugOverlay;
		if (InfoCombo.State && !InfoCombo.Holding && EnableInfoOverlay)
			ShowInfoOverlay = !ShowInfoOverlay;

		// Inject Key Presses

		// Inject ready weapon or cancel based on context, on RMB press
		if (EnableEnhancedMouse && GetEventIndex() != EVENT_MAP && GetEventIndex() != EVENT_INVENTORY && GetEventIndex() != EVENT_OPTION_FMV && 
			GetEventIndex() != EVENT_FMV && GetCutsceneID() == 0x0) 
		{
			if (RMB.State)
			{
				if (SetRMBAimFunction())
				{
					SetKey(KeyBinds.GetKeyBind(KEY_READY_WEAPON));
				}
				else
				{
					if (!RMB.Holding) {
						SetKey(KeyBinds.GetKeyBind(KEY_CANCEL));
					}
				}
			}
		}

		// Inject action, same condition as RMB but without CutsceneID == 0x00 to enable input on FIE in the middle of cutscenes, eg Laura's letter in the hotel
		if (EnableEnhancedMouse && GetEventIndex() != EVENT_MAP && GetEventIndex() != EVENT_INVENTORY && GetEventIndex() != EVENT_OPTION_FMV &&
			GetEventIndex() != EVENT_FMV)
		{
			if (SetLMButton)
				SetKey(KeyBinds.GetKeyBind(KEY_ACTION));
		}

		// Strafe keys become movement keys on menus
		if (GetEventIndex() != EVENT_IN_GAME || (GetEventIndex() == EVENT_IN_GAME && IsInFullScreenImageEvent()))
		{
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_STRAFE_LEFT)))
				SetKey(KeyBinds.GetKeyBind(KEY_TURN_LEFT));
			if (IsKeyPressed(KeyBinds.GetKeyBind(KEY_STRAFE_RIGHT)))
				SetKey(KeyBinds.GetKeyBind(KEY_TURN_RIGHT));
		}

		if (SetUpKey)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS));
			SetUpKey = false;
		}

		if (SetDownKey)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS));
			SetDownKey = false;
		}

		if (SetLeftKey)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_TURN_LEFT));
			SetLeftKey = false; 
		}

		if (SetRightKey)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_TURN_RIGHT));
			SetRightKey = false;
		}

		// Setting sprint button for the Toggle Sprint function
		if (EnableToggleSprint && (GetRunOption() == OPT_ANALOG || OverrideSprint) && GetEventIndex() != EVENT_OPTION_FMV)
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_RUN));
		}

		if (EnableToggleSprint && Sprint.State && ((GetRunOption() == OPT_ANALOG && IsMovementPressed()) || OverrideSprint) && GetEventIndex() == EVENT_IN_GAME)
		{
			SetKey(KeyBinds.GetKeyBind(KEY_RUN));
		}

		// Mouse wheel weapon swapping
		if (MouseWheel != 0 && DeltaMsWeaponSwap.count() > InputDebounce)
		{
			// Inject up and down in the memo screen
			switch (GetEventIndex()) 
			{
			case EVENT_MEMO_LIST:
				if (MouseWheel > 0)
					SetKey(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS));
				else
					SetKey(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS));

				break;

			case EVENT_IN_GAME:
				if (EnableMouseWheelSwap)
				{
					if (MouseWheel > 0)
						SetKey(KeyBinds.GetKeyBind(KEY_NEXT_WEAPON));
					else
						SetKey(KeyBinds.GetKeyBind(KEY_PREV_WEAPON));

					LastWeaponSwap = Now;
				}
				break;

			default:
				break;

			}
			MouseWheel = 0;
		}

		// Fix for Alt-Tabbing issues
		if (GetForegroundWindow() != GameWindowHandle)
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_ACTION));
			ClearKey(KeyBinds.GetKeyBind(KEY_READY_WEAPON));
		}

		// Fix for memo screen arrow keys navigation
		if (MemoScreenFix && GetEventIndex() == EVENT_MEMO_LIST)
		{
			if (IsKeyPressed(DIK_UP))
				SetKey(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS));
			if (IsKeyPressed(DIK_DOWN))
				SetKey(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS));
			if (IsKeyPressed(DIK_RETURN))
				SetKey(KeyBinds.GetKeyBind(KEY_ACTION));
		}

		// Fix for input sticking after alt+tab while holding aim and action
		if (CleanKeys)
		{
			ClearKey(KeyBinds.GetKeyBind(KEY_ACTION));
			ClearKey(KeyBinds.GetKeyBind(KEY_READY_WEAPON));
			CleanKeys = false;
		}

		// Clear Keyboard Data pointer
		KeyboardData = nullptr;
	}

	// Setup for the input functions
	if (!once)
	{
		once = true;
		
		// Hooking controller input functions
		orgGetControllerLXAxis.fun = injector::MakeCALL(GetLeftAnalogXFunctionPointer(), GetControllerLXAxis_Hook, true).get();
		orgGetControllerLYAxis.fun = injector::MakeCALL(GetLeftAnalogYFunctionPointer(), GetControllerLYAxis_Hook, true).get();
		orgGetControllerRXAxis.fun = injector::MakeCALL(GetRightAnalogXFunctionPointer(), GetControllerRXAxis_Hook, true).get();
		orgGetControllerRYAxis.fun = injector::MakeCALL(GetRightAnalogYFunctionPointer(), GetControllerRYAxis_Hook, true).get();

		orgUpdateMousePosition.fun = injector::MakeCALL(GetUpdateMousePositionFunctionPointer(), UpdateMousePosition_Hook, true).get();

		CheckNumberKeyBinds();

		if (EnableToggleSprint)
		{
			if (!GetAnalogStringAddr()) return;

			// Change the Analog string (0x68) with Toggle (0x2F)
			UpdateMemoryAddress((void*)AnalogStringOne, "\x2F", 1);
			UpdateMemoryAddress((void*)AnalogStringTwo, "\x2F", 1);
			UpdateMemoryAddress((void*)AnalogStringThree, "\x2F", 1);
		}

		// Hooking the mouse visibility function
		if (EnableEnhancedMouse)
		{
			orgDrawCursor.fun = injector::MakeCALL((DWORD*)0x00476128, DrawCursor_Hook, true).get();
			orgSetShowCursorFlag.fun = injector::MakeCALL((DWORD*)0x00454886, SetShowCursorFlag_Hook, true).get();
		}
	}
}

void InputTweaks::TweakGetDeviceData(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbObjectData, 
	LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(cbObjectData);

	// For mouse
	if (ProxyInterface == MouseInterfaceAddress)
	{
		// Save Mouse Data for axis movement
		MouseData = rgdod;
		MouseDataSize = *pdwInOut;

		// Get the current mouse state, to handle LMB/RMB
		MouseInterfaceAddress->GetDeviceState(sizeof(MouseState), &MouseState);

		// Save current mouse state
		MouseXAxis = GetMouseRelXChange();
		ReadMouseButtons();

		// In search view, inject mouse movement into the right analog stick
		if (GetSearchViewFlag() == 0x6)
		{
			VirtualRightStick.AddXValue(MouseXAxis);
			VirtualRightStick.AddYValue(GetMouseRelYChange());
		}
		else
		{
			VirtualRightStick.Recenter();
		}

		// Clear Mouse Data
		MouseData = nullptr;
		MouseDataSize = NULL;
	}
}

void InputTweaks::ClearKey(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData)
		return;

	KeyboardData[KeyIndex] = KEY_CLEAR;
}

void InputTweaks::SetKey(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData || KeyIndex == 0x0)
		return;

	KeyboardData[KeyIndex] = KEY_SET;
}

bool InputTweaks::IsKeyPressed(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData)
		return false;

	return KeyboardData[KeyIndex] == KEY_SET;
}

void InputTweaks::SetKeyboardInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
{
	KeyboardInterfaceAddress = ProxyInterface;
}

void InputTweaks::SetMouseInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
{
	MouseInterfaceAddress = ProxyInterface;
}

void InputTweaks::SetControllerInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
{
	ControllerInterfaceAddress = ProxyInterface;
}

void InputTweaks::RemoveInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
{
	if (MouseInterfaceAddress == ProxyInterface)
		MouseInterfaceAddress = nullptr;
	if (KeyboardInterfaceAddress == ProxyInterface)
		KeyboardInterfaceAddress = nullptr;
}

int32_t InputTweaks::GetMouseRelXChange()
{
	auto Now = std::chrono::system_clock::now();
	auto DeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastMouseXRead);
	LastMouseXRead = Now;

	if (DeltaMs.count() == 0)
		return MouseXAxis;

	int32_t AxisSum = 0;

	if (!MouseData || MouseDataSize == 0)
		return AxisSum;

	for (UINT i = 0; i < MouseDataSize; i++)
		if (MouseData[i].dwOfs == DIMOFS_X)
			AxisSum += (int32_t) MouseData[i].dwData;

	return AxisSum;
}

int32_t InputTweaks::GetMouseRelYChange()
{
	auto Now = std::chrono::system_clock::now();
	auto DeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastMouseYRead);
	LastMouseYRead = Now;

	if (DeltaMs.count() == 0)
		return MouseYAxis;

	int32_t AxisSum = 0;

	if (!MouseData || MouseDataSize == 0)
		return AxisSum;

	for (UINT i = 0; i < MouseDataSize; i++)
		if (MouseData[i].dwOfs == DIMOFS_Y)
			AxisSum += (int32_t)MouseData[i].dwData;

	return AxisSum;
}

void InputTweaks::ReadMouseButtons()
{		
	if (!MouseData || GetForegroundWindow() != GameWindowHandle) return;

	for (UINT i = 0; i < MouseDataSize; i++)
		switch (MouseData[i].dwOfs)
		{
#pragma warning(suppress: 4644)
		case DIMOFS_Z:
		{
			MouseWheel = MouseData[i].dwData;
		}

		default:
		{
			break;
		}
		}

	SetLMButton = (MouseState.rgbButtons[0] == KEY_SET);
	RMB.State = (MouseState.rgbButtons[1] == KEY_SET);
}

float InputTweaks::GetMouseAnalogX()
{
	if (MouseXAxis == 0 || GetSearchViewFlag() == 0x06 || GetEventIndex() != EVENT_IN_GAME)
	{
		MouseXAxis = 0;
		return 0;
	}

	int TempAxis = MouseXAxis;
	MouseXAxis = 0;

	if (TempAxis > 0)
		return TempAxis > AnalogThreshold ? AnalogFullTilt : AnalogHalfTilt;

	return TempAxis < -AnalogThreshold ? -AnalogFullTilt : -AnalogHalfTilt;
}

float InputTweaks::GetForwardAnalog()
{
	return ForwardBackwardsAxis > 0 ? -1.0f : ForwardBackwardsAxis < 0 ? 1.0f : 0.0f;
}

float InputTweaks::GetTurningAnalog()
{

	return LeftRightAxis > 0 ? -1.0f : LeftRightAxis < 0 ? 1.0f : 0.0f;
}

void InputTweaks::ClearMouseInputs()
{
	SetLMButton = false;
	RMB.State = false;
	CleanKeys = true;
}

void InputTweaks::CheckNumberKeyBinds()
{
	// Fix for binding actions to the number keys
	BYTE* NumberKeyBinds = GetNumKeysWeaponBindStartPointer();
	BYTE* ActionKeyBinds = KeyBinds.GetKeyBindsPointer();
	boolean FoundNumber = false;

	// Iterate over Number Keybinds
	for (int i = 0; i < 0xA; i++)
	{
		FoundNumber = false;

		// Iterate over Action Keybinds
		for (int j = 0; j < 0x16; j++)
		{
			if (*(ActionKeyBinds + (j * 0x8)) == DefaultNumberKeyBinds[i])
			{
				FoundNumber = true;
				break;
			}
		}

		if (FoundNumber)
		{
			*(NumberKeyBinds + (i * 0x8)) = 0x00;
		}
		else
		{
			*(NumberKeyBinds + (i * 0x8)) = (BYTE)DefaultNumberKeyBinds[i];
		}
	}
}

std::string InputTweaks::GetRightClickState()
{
	std::string Output = "Cancel";

	if (SetRMBAimFunction())
		Output = "Ready Weapon";

	if (!EnableEnhancedMouse)
		Output = "Not Enabled";

	return Output;
}

std::string InputTweaks::GetToggleSprintState()
{
	std::string Output = "Not Active";

	if (Sprint.State && (GetRunOption() == OPT_ANALOG || OverrideSprint))
		Output = "Active";

	if (!EnableToggleSprint)
		Output = "Not Enabled";

	return Output;
}

bool InputTweaks::ElevatorFix()
{
	return (GetRoomID() == 0x46 && GetTalkShowHostState() == 0x01);
}

bool InputTweaks::HotelFix()
{
	return (GetRoomID() == 0xB8 && 
		(((std::abs(GetInGameCameraPosY() - (-840.)) < FloatTolerance) || (std::abs(GetInGameCameraPosY() - (-1350.)) < FloatTolerance))) &&
		IsInFullScreenImageEvent());
}

bool InputTweaks::JamesVaultingBuildingsFix()
{
	return (GetRoomID() == 0x07 && std::abs(GetInGameCameraPosY() - (-3315.999)) < FloatTolerance);
}

bool InputTweaks::RosewaterParkFix()
{
	return (GetRoomID() == 0x08 && std::abs(GetInGameCameraPosY() - (150.)) < FloatTolerance && std::abs(GetJamesPosZ() - (78547.117)) < FloatTolerance);
}

bool InputTweaks::HospitalMonologueFix()
{
	return (GetRoomID() == 0x08 && std::abs(GetJamesPosZ() - (-6000.)) < FloatTolerance && IsInFullScreenImageEvent());
}

bool InputTweaks::FleshRoomFix()
{
	return (GetRoomID() == 0x79 || GetRoomID() == 0x8A || GetRoomID() == 0x8C);
}

bool InputTweaks::SetRMBAimFunction()
{
	return (GetEventIndex() == EVENT_IN_GAME &&
		(ElevatorFix() || HotelFix() || JamesVaultingBuildingsFix() || 
		RosewaterParkFix() || HospitalMonologueFix() || FleshRoomFix() ||
		!IsInFullScreenImageEvent()));
}

bool InputTweaks::IsInFullScreenImageEvent()
{
	return GetFullscreenImageEvent() == 0x02;
}

bool InputTweaks::GetAnalogStringAddr()
{
	constexpr BYTE AnalogStringOneSearchBytes[]{ 0x68, 0x9A, 0x00, 0x00, 0x00, 0x56, 0x6A, 0x68 };
	BYTE *AnalogString = (BYTE*)SearchAndGetAddresses(0x461F63, 0x4621D5, 0x4621D5, AnalogStringOneSearchBytes, sizeof(AnalogStringOneSearchBytes), 0x07, __FUNCTION__);

	if (!AnalogString)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog String One address!";
		return false;
	}
	else
	{
		AnalogStringOne = (BYTE*)((DWORD)AnalogString);
	}

	constexpr BYTE AnalogStringTwoSearchBytes[]{ 0x68, 0x98, 0x00, 0x00, 0x00, 0x81, 0xC7, 0x0C, 0x01, 0x00, 0x00, 0x57, 0x6A, 0x68 };
	AnalogString = (BYTE*)SearchAndGetAddresses(0x4621D1, 0x462433, 0x462433, AnalogStringTwoSearchBytes, sizeof(AnalogStringTwoSearchBytes), 0x0D, __FUNCTION__);

	if (!AnalogString)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog String Two address!";
		return false;
	}
	else
	{
		AnalogStringTwo = (BYTE*)((DWORD)AnalogString);
	}

	constexpr BYTE AnalogStringThreeSearchBytes[]{ 0x00, 0x6A, 0x68 };
	AnalogString = (BYTE*)SearchAndGetAddresses(0x464465, 0x4646DE, 0x4648E6, AnalogStringThreeSearchBytes, sizeof(AnalogStringThreeSearchBytes), 0x02, __FUNCTION__);

	if (!AnalogString)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find Analog String Three address!";
		return false;
	}
	else
	{
		AnalogStringThree = (BYTE*)((DWORD)AnalogString);
	}

	return true;
}

bool InputTweaks::IsMovementPressed()
{
	return IsKeyPressed(KeyBinds.GetKeyBind(KEY_MOVE_FORWARDS)) || IsKeyPressed(KeyBinds.GetKeyBind(KEY_MOVE_BACKWARDS)) ||
		IsKeyPressed(KeyBinds.GetKeyBind(KEY_TURN_LEFT)) || IsKeyPressed(KeyBinds.GetKeyBind(KEY_TURN_RIGHT)) || 
		IsKeyPressed(KeyBinds.GetKeyBind(KEY_STRAFE_RIGHT)) || IsKeyPressed(KeyBinds.GetKeyBind(KEY_STRAFE_LEFT));
}

void InputTweaks::SetOverrideSprint()
{
	OverrideSprint = true;
}

void InputTweaks::ClearOverrideSprint()
{
	OverrideSprint = false;
}

bool InputTweaks::GetRMBState()
{
	return RMB.State;
}

bool InputTweaks::GetLMBState()
{
	return SetLMButton;
}

BYTE KeyBindsHandler::GetKeyBind(int KeyIndex)
{
	BYTE* pButton = GetKeyBindsPointer();

	if ((pButton && *(pButton + (KeyIndex * 0x08)) == 0) && keyNotSetWarning[KeyIndex] == 0)
	{
		Logging::Log() << "ERROR: Keybind " << KEY_NAMES[KeyIndex] << " not set";
		keyNotSetWarning[KeyIndex] = 1;
	}

	return (pButton) ? *(pButton + (KeyIndex * 0x08)) : 0;
}

BYTE* KeyBindsHandler::GetKeyBindsPointer()
{
	if (KeyBindsAddr)
	{
		return KeyBindsAddr;
	}

	// Get Turn Left Button address
	constexpr BYTE TurnLeftButtonSearchBytes[]{ 0x56, 0x8B, 0x74, 0x24, 0x08, 0x83, 0xFE, 0x16, 0x7D, 0x3F };
	BYTE* Binds = (BYTE*)ReadSearchedAddresses(0x005AEF90, 0x005AF8C0, 0x005AF1E0, TurnLeftButtonSearchBytes, sizeof(TurnLeftButtonSearchBytes), 0x1D, __FUNCTION__);

	// Checking address pointer
	if (!Binds)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find KeyBinds address!";
		return nullptr;
	}

	KeyBindsAddr = (BYTE*)((DWORD)Binds);

	return KeyBindsAddr;
}

BYTE KeyBindsHandler::GetPauseButtonBind()
{
	return *(this->GetKeyBindsPointer() + 0xF0);
}

int CountCollectedMemos()
{
	auto* psVar2 = (int16_t*)0x0088c378; //TODO addresses

	auto* MemosArray = (int32_t*)0x1f7a9e0;
	int TotalMemos = 0;

	do {

		if (((unsigned int)(MemosArray)[(int)*psVar2 >> 5] >> ((byte)*psVar2 & 0x1f) & 1) != 0)
			TotalMemos = TotalMemos + 1;

		if (((unsigned int)(MemosArray)[(int)psVar2[8] >> 5] >> ((byte)psVar2[8] & 0x1f) & 1) != 0)
			TotalMemos = TotalMemos + 1;

		if (((unsigned int)(MemosArray)[(int)psVar2[0x10] >> 5] >> ((byte)psVar2[0x10] & 0x1f) & 1) != 0)
			TotalMemos = TotalMemos + 1;

		psVar2 += 0x18;
	} while ((int)psVar2 < 0x88c708);

	return TotalMemos;
}