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

#include "InputTweaks.h"

const int AnalogThreshold = 10;
const int InputDebounce = 50;
const int PauseMenuMouseThreshold = 15;
const int AnalogHalfTilt = 80;
const int AnalogFullTilt = 126;

bool once = false;

auto LastMouseXRead = std::chrono::system_clock::now();
auto LastMouseYRead = std::chrono::system_clock::now();
auto LastWeaponSwap = std::chrono::system_clock::now();
auto LastMousePauseChange = std::chrono::system_clock::now();
int LastMouseVerticalPos = 0xF0;
int LastMouseHorizontalPos = 0xFC;
int MouseXAxis = 0;
int MouseYAxis = 0;
long int MouseWheel = 0;
int AnalogX = 0;
AnalogStick VirtualRightStick;
bool PauseMenuVerticalChanged = false;
bool PauseMenuHorizontalChanged = false;

bool SetLeftMouseButton = false;
bool SetRightMouseButton = false;
bool SetLeftKey = false;
bool SetRightKey = false;
bool SetUpKey = false;
bool SetDownKey = false;
bool FirstFrameNoInputFlag = false;

injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLYAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRYAxis;
injector::hook_back<void(__cdecl*)(void)> orgUpdateMousePosition;

int8_t GetControllerLXAxis_Hook(DWORD* arg)
{
	int TempXAxis = MouseXAxis;
	MouseXAxis = 0;

	// Fix for James spinning after Alt+ Tab
	if (GetForegroundWindow() != GameWindowHandle || GetEnableInput() != 0xFFFFFFFF || GetSearchViewFlag() == 0x6)
	{
		return orgGetControllerLXAxis.fun(arg);
	}

	if (TempXAxis != 0)
	{
		FirstFrameNoInputFlag = true;

		if (TempXAxis > 0)
		{
			AnalogX = TempXAxis > AnalogThreshold ? AnalogFullTilt : AnalogHalfTilt;
		}
		else if (TempXAxis < 0)
		{
			AnalogX = TempXAxis < -AnalogThreshold ? -AnalogFullTilt : -AnalogHalfTilt;
		}
		
	} 
	else
	{
		if (FirstFrameNoInputFlag)
		{
			FirstFrameNoInputFlag = false;
		}
		else
		{
			AnalogX = 0;
			return orgGetControllerLXAxis.fun(arg);
		}

	}

	return AnalogX;
}

int8_t GetControllerLYAxis_Hook(DWORD* arg)
{
	return orgGetControllerLYAxis.fun(arg);
}

int8_t GetControllerRXAxis_Hook(DWORD* arg)
{
	if (GetSearchViewFlag() == 0x6 && !VirtualRightStick.IsCentered())
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
	if (GetSearchViewFlag() == 0x6 && !VirtualRightStick.IsCentered())
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

	if (GetEventIndex() == EVENT_PAUSE_MENU || GetEventIndex() == EVENT_MEMO_LIST)
	{
		auto Now = std::chrono::system_clock::now();
		auto DeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastMousePauseChange);

		// Vertical navigation
		if (PauseMenuVerticalChanged && DeltaMs.count() > InputDebounce)
		{
			LastMouseVerticalPos = GetMouseVerticalPosition();

			PauseMenuVerticalChanged = false;
		}

		if (std::abs(GetMouseVerticalPosition() - LastMouseVerticalPos) > PauseMenuMouseThreshold && !PauseMenuVerticalChanged)
		{
			if (GetMouseVerticalPosition() > LastMouseVerticalPos && (GetPauseMenuButtonIndex() != 0x04 || GetEventIndex() != EVENT_PAUSE_MENU))
			{
				SetDownKey = true;
			}
			else if (GetMouseVerticalPosition() < LastMouseVerticalPos && (GetPauseMenuButtonIndex() != 0x00 || GetEventIndex() != EVENT_PAUSE_MENU))
			{
				SetUpKey = true;
			} 

			LastMousePauseChange = Now;
			PauseMenuVerticalChanged = true;
			
		}
		else
		{
			LastMouseVerticalPos = GetMouseVerticalPosition();
		}

		// Horizontal navigation
		if (PauseMenuHorizontalChanged && DeltaMs.count() > InputDebounce)
		{
			LastMouseHorizontalPos = GetMouseHorizontalPosition();

			PauseMenuHorizontalChanged = false;
		}

		if (std::abs(GetMouseHorizontalPosition() - LastMouseHorizontalPos) > PauseMenuMouseThreshold && !PauseMenuHorizontalChanged)
		{
			if (GetMouseHorizontalPosition() > LastMouseHorizontalPos && GetPauseMenuQuitIndex() == 0)
			{
				SetRightKey = true;
			}
			else if (GetMouseHorizontalPosition() < LastMouseHorizontalPos && GetPauseMenuQuitIndex() == 1)
			{
				SetLeftKey = true;
			}
			
			LastMousePauseChange = Now;
			PauseMenuHorizontalChanged = true;
		}
		else
		{
			LastMouseHorizontalPos = GetMouseHorizontalPosition();
		}

		// Reset mouse position to avoid edges
		if (GetMouseHorizontalPosition() > 0x15F || GetMouseHorizontalPosition() < 0x10)
		{
			*GetMouseHorizontalPositionPointer() = 0xFC;
			LastMouseHorizontalPos = 0xFC;
		}
		if (GetMouseVerticalPosition() > 0x1D0 || GetMouseVerticalPosition() < 0x10)
		{
			*GetMouseVerticalPositionPointer() = 0xF0;
			LastMouseVerticalPos = 0xF0;
		}
	}
}

void InputTweaks::TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData)
{
	// For keyboard
	if (ProxyInterface == KeyboardInterfaceAddress)
	{
		auto Now = std::chrono::system_clock::now();
		auto DeltaMsWeaponSwap = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastWeaponSwap);

		// Save Keyboard Data
		KeyboardData = (BYTE*)lpvData;
		
		// Ignore Alt + Enter combo
		if ((IsKeyPressed(DIK_LMENU) || IsKeyPressed(DIK_RMENU)) && IsKeyPressed(DIK_RETURN) && 
			DynamicResolution && ScreenMode != EXCLUSIVE_FULLSCREEN)
		{
			ClearKey(DIK_LMENU);
			ClearKey(DIK_RMENU);
			ClearKey(DIK_RETURN);
			Logging::LogDebug() << __FUNCTION__ << " Ignoring ALT + ENTER...";
		}

		// Ignore Ctrl + D combo
		if (IsKeyPressed(DIK_LCONTROL) && IsKeyPressed(DIK_D) && EnableDebugOverlay)
		{
			ClearKey(DIK_LCONTROL);
			ClearKey(DIK_D);
			Logging::LogDebug() << __FUNCTION__ << " Ignoring CTRL + D...";
		}

		// Ignore Ctrl + I combo
		if (IsKeyPressed(DIK_LCONTROL) && IsKeyPressed(DIK_I) && EnableInfoOverlay)
		{
			ClearKey(DIK_LCONTROL);
			ClearKey(DIK_I);
			Logging::LogDebug() << __FUNCTION__ << " Ignoring CTRL + I...";
		}

		// Inject Key Presses
		if (GetEventIndex() != EVENT_MAP && GetEventIndex() != EVENT_INVENTORY && GetEventIndex() != EVENT_OPTION_FMV)
		{
			if (SetLeftMouseButton)
				SetKey(GetActionKeyBind());
			if (SetRightMouseButton)
			{
				// If in game
				if (GetEventIndex() == EVENT_IN_GAME)
					SetKey(GetAimKeyBind());
				else
					SetKey(GetCancelKeyBind());
			}
		}

		if (SetUpKey)
		{
			SetKey(GetWalkForwardKeyBind());
			SetUpKey = false;
		}

		if (SetDownKey)
		{
			SetKey(GetWalkBackwardsKeyBind());
			SetDownKey = false;
		}

		if (SetLeftKey)
		{
			SetKey(GetTurnLeftKeyBind());
			SetLeftKey = false;
		}

		if (SetRightKey)
		{
			SetKey(GetTurnRightKeyBind());
			SetRightKey = false;
		}

		if (MouseWheel != 0 && DeltaMsWeaponSwap.count() > InputDebounce && GetEventIndex() == EVENT_IN_GAME)
		{
			if (MouseWheel > 0)
				SetKey(GetNextWeaponKeyBind());
			else 
				SetKey(GetPreviousWeaponKeyBind());

			LastWeaponSwap = Now;
			MouseWheel = 0;
		}

		if (GetEventIndex() == EVENT_MEMO_LIST)
		{
			if (IsKeyPressed(DIK_UP))
				SetKey(GetWalkForwardKeyBind());
			if (IsKeyPressed(DIK_DOWN))
				SetKey(GetWalkBackwardsKeyBind());
			if (IsKeyPressed(DIK_RETURN))
				SetKey(GetActionKeyBind());
		}

		// Clear Keyboard Data pointer
		KeyboardData = nullptr;
	}

	if (!once)
	{
		once = true;

		orgGetControllerLXAxis.fun = injector::MakeCALL(GetLeftAnalogXFunctionPointer(), GetControllerLXAxis_Hook, true).get();
		orgGetControllerLYAxis.fun = injector::MakeCALL(GetLeftAnalogYFunctionPointer(), GetControllerLYAxis_Hook, true).get();
		orgGetControllerRXAxis.fun = injector::MakeCALL(GetRightAnalogXFunctionPointer(), GetControllerRXAxis_Hook, true).get();
		orgGetControllerRYAxis.fun = injector::MakeCALL(GetRightAnalogYFunctionPointer(), GetControllerRYAxis_Hook, true).get();

		orgUpdateMousePosition.fun = injector::MakeCALL(GetUpdateMousePositionFunctionPointer(), UpdateMousePosition_Hook, true).get();
	}

}

void InputTweaks::TweakGetDeviceData(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	// For mouse
	if (ProxyInterface == MouseInterfaceAddress)
	{
		// Save Mouse Data
		MouseData = rgdod;
		MouseDataSize = *pdwInOut;

		MouseXAxis = GetMouseRelXChange();
		ReadMouseButtons();

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
	if (KeyIndex > 0x100 || !KeyboardData)
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

void InputTweaks::RemoveAddr(LPDIRECTINPUTDEVICE8A ProxyInterface)
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

	for (int i = 0; i < MouseDataSize; i++)
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

	for (int i = 0; i < MouseDataSize; i++)
		if (MouseData[i].dwOfs == DIMOFS_Y)
			AxisSum += (int32_t)MouseData[i].dwData;

	return AxisSum;
}

void InputTweaks::ReadMouseButtons()
{
	if (!MouseData) return;

	for (int i = 0; i < MouseDataSize; i++)
		switch (MouseData[i].dwOfs)
		{
		case DIMOFS_BUTTON0:
		{
			SetLeftMouseButton = MouseData[i].dwData == KEY_SET;
			break;
		}
		case DIMOFS_BUTTON1:
		{
			SetRightMouseButton = MouseData[i].dwData == KEY_SET;
			break;
		}
		case DIMOFS_Z:
		{
			MouseWheel = MouseData[i].dwData;
		}

		default:
		{
			break;
		}

		}
		
}