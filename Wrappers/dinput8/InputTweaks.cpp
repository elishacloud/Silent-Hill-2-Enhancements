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

const int AnalogThreshold = 30;
const int  StickFlipBack = 30;
const int MousePauseDebounce = 300;

BYTE* KeyboardData = nullptr;
LPDIDEVICEOBJECTDATA MouseData = nullptr;
DWORD MouseDataSize = NULL;

bool once = false;
auto LastMouseXRead = std::chrono::system_clock::now();
auto LastMouseYRead = std::chrono::system_clock::now();

auto LastMousePauseChange = std::chrono::system_clock::now();
int LastMouseVerticalPos = 0xF0;
int LastMouseHorizontalPos = 0xFC;

int MouseXAxis = 0;
int MouseYAxis = 0;
int AnalogX = 0;

bool LeftMouseButtonState = false;
bool RightMouseButtonState = false;
bool LeftPressState = false;
bool PauseVerticalChanged = false;
bool PauseHorizontalChanged = false;

injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLYAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRYAxis;
injector::hook_back<void(__cdecl*)(void)> orgUpdateMousePosition;

int8_t GetControllerLXAxis_Hook(DWORD* arg)
{
	int RetValue = MouseXAxis;
	MouseXAxis = 0;
	//TODO check james speed while mouse turning
	// Fix for James spinning after Alt+ Tab
	if (GetForegroundWindow() != GameWindowHandle || GetEnableInput() != 0xFFFFFFFF)
	{
		return orgGetControllerLXAxis.fun(arg);
	}

	if (RetValue > 0)
	{
		AnalogX = RetValue > AnalogThreshold ? 126 : 80;
		return RetValue > AnalogThreshold ? 126 : 80;
	}
	else if (RetValue < 0)
	{
		AnalogX = RetValue < -AnalogThreshold ? -126 : -80;
		return RetValue < -AnalogThreshold ? -126 : -80;
	}
	
	// If no mouse input is detected this frame
	if (AnalogX > 0)
	{
		AnalogX -= StickFlipBack;
		if (AnalogX < 0)
			AnalogX = 0;

		return AnalogX;
	} else if (AnalogX > 0)
	{
		AnalogX -= StickFlipBack;
		if (AnalogX < 0)
			AnalogX = 0;

		return AnalogX;
	}
	else 
	{
		return orgGetControllerRXAxis.fun(arg);
	}
	
}

int8_t GetControllerLYAxis_Hook(DWORD* arg)
{
	return orgGetControllerLYAxis.fun(arg);
}

int8_t GetControllerRXAxis_Hook(DWORD* arg)
{
	return orgGetControllerRXAxis.fun(arg);
}

int8_t GetControllerRYAxis_Hook(DWORD* arg)
{
	return orgGetControllerRYAxis.fun(arg);
}

void UpdateMousePosition_Hook()
{
	orgUpdateMousePosition.fun();

	// In the pause menu
	if (GetEventIndex() == 0x10)
	{
		auto Now = std::chrono::system_clock::now();
		auto DeltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(Now - LastMousePauseChange);

		if (PauseVerticalChanged && DeltaMs.count() > MousePauseDebounce)
		{
			LastMouseVerticalPos = GetMouseVerticalPosition();

			PauseVerticalChanged = false;
		}

		if (PauseHorizontalChanged && DeltaMs.count() > MousePauseDebounce)
		{
			LastMouseHorizontalPos = GetMouseHorizontalPosition();

			PauseHorizontalChanged = false;
		}

		if (LastMouseVerticalPos != GetMouseVerticalPosition() && !PauseVerticalChanged)
		{
			if (GetMouseVerticalPosition() > LastMouseVerticalPos)
			{
				switch (GetPauseMenuButtonIndex())
				{
				case 0:
				case 1:
				case 2:
				case 3:
					*GetPauseMenuButtonIndexPointer() += 1;
					break;
				case 4:
					*GetPauseMenuButtonIndexPointer() = 0;
					break;
				}
			}
			else
			{
				switch (GetPauseMenuButtonIndex())
				{
				case 0:
					*GetPauseMenuButtonIndexPointer() = 4;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					*GetPauseMenuButtonIndexPointer() -= 1;
					break;
				}
			}

			LastMousePauseChange = Now;
			PauseVerticalChanged = true;

		}

		if (LastMouseHorizontalPos != GetMouseHorizontalPosition() && !PauseHorizontalChanged)
		{
			LeftPressState = true;

			LastMousePauseChange = Now;
			PauseHorizontalChanged = true;
		}

		if (GetMouseHorizontalPosition() > 0x15F || GetMouseHorizontalPosition() < 0x10)
			*GetMouseHorizontalPositionPointer() = 0xFC;
		if (GetMouseVerticalPosition() > 0x1D0 || GetMouseVerticalPosition() < 0x10)
			*GetMouseVerticalPositionPointer() = 0xF0;
	}
}

void InputTweaks::TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData)
{
	// For keyboard
	if (ProxyInterface == KeyboardInterfaceAddress)
	{
		KeyboardData = (BYTE*)lpvData;
		
		// Ignore Alt + Enter combo
		if ((IsKeyPressed(DIK_LMENU) || IsKeyPressed(DIK_RMENU)) && IsKeyPressed(DIK_RETURN) && 
			DynamicResolution && ScreenMode != 3)
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

		if (LeftMouseButtonState)
			SetKey(GetActionButton());
		if (RightMouseButtonState)
			SetKey(GetAimButton());
		if (LeftPressState)
		{
			SetKey(GetTurnLeftButton());
			LeftPressState = false;
		}

		// Clear 
		KeyboardData = nullptr;
	}

	if (!once)
	{
		once = true;

		//TODO make exe agnostic
		auto pattern = hook::pattern("e8 36 16 04 00");
		orgGetControllerLXAxis.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetControllerLXAxis_Hook, true).get();
		pattern = hook::pattern("e8 26 16 04 00");
		orgGetControllerLYAxis.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetControllerLYAxis_Hook, true).get();
		pattern = hook::pattern("e8 13 16 04 00");
		orgGetControllerRXAxis.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetControllerRXAxis_Hook, true).get();
		pattern = hook::pattern("e8 03 16 04 00");
		orgGetControllerRYAxis.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetControllerRYAxis_Hook, true).get();
		pattern = hook::pattern("e8 aa 1b 00 00");
		orgUpdateMousePosition.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), UpdateMousePosition_Hook, true).get();
	}

}

void InputTweaks::TweakGetDeviceData(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	// For mouse
	if (ProxyInterface == MouseInterfaceAddress)
	{
		MouseData = rgdod;
		MouseDataSize = *pdwInOut;

		MouseXAxis = GetMouseRelXChange();
		ReadMouseButtons();

		MouseData = nullptr;
		MouseDataSize = NULL;
	}
}

void InputTweaks::ClearKey(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData)
		return;

	KeyboardData[KeyIndex] = 0x00;
}

void InputTweaks::SetKey(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData)
		return;

	KeyboardData[KeyIndex] = 0x80;
}

bool InputTweaks::IsKeyPressed(int KeyIndex)
{
	if (KeyIndex > 0x100 || !KeyboardData)
		return false;

	return KeyboardData[KeyIndex] == 0x80;
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
		if (MouseData->dwOfs == DIMOFS_X)
			AxisSum += (int32_t) MouseData->dwData;

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
		if (MouseData->dwOfs == DIMOFS_Y)
			AxisSum += (int32_t)MouseData->dwData;

	return AxisSum;
}

void InputTweaks::ReadMouseButtons()
{
	if (!MouseData) return;

	for (int i = 0; i < MouseDataSize; i++)
		switch (MouseData->dwOfs)
		{
		case DIMOFS_BUTTON0:
		{
			LeftMouseButtonState = MouseData->dwData == 0x80;
			break;
		}
		case DIMOFS_BUTTON1:
		{
			RightMouseButtonState = MouseData->dwData == 0x80;
			break;
		}

		default:
		{
			break;
		}

		}
		
}

static int8_t ClampByteValue(int Value)
{
	if (Value >= 126)
		return 126;
	else if (Value <= -126)
		return -126;
	else
		return Value;
}