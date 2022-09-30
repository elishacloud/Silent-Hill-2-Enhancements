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
#include "External\injector\include\injector\injector.hpp"
#include "External/injector/include/injector/hooking.hpp"
#include "External/injector/include/injector/utility.hpp"
#include "External/Hooking.Patterns/Hooking.Patterns.h"
#include <chrono>
#include "Patches\Patches.h"

BYTE* KeyboardData = nullptr;
LPDIDEVICEOBJECTDATA MouseData = nullptr;
DWORD MouseDataSize = NULL;

bool once = false;
auto LastMouseXRead = std::chrono::system_clock::now();
int MouseXAxis = 0;
int AnalogX = 0;
bool LeftMouseButtonState = false;
bool RightMouseButtonState = false;

long int frame = 0;

injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerLYAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRXAxis;
injector::hook_back<int8_t(__cdecl*)(DWORD*)> orgGetControllerRYAxis;

int8_t GetControllerLXAxis(DWORD* arg)
{
	int  FlipBack = 30;
	int RetValue = MouseXAxis;
	MouseXAxis = 0;

	if (RetValue > 0)
	{
		AnalogX = RetValue > 50 ? 126 : 80;
		return RetValue > 50 ? 126 : 80;
	}
	else if (RetValue < 0)
	{
		AnalogX = RetValue < -50 ? -126 : -80;
		return RetValue < -50 ? -126 : -80;
	}
	else
	{
		if (AnalogX > 0)
		{
			AnalogX -= FlipBack;
			if (AnalogX < 0)
				AnalogX = 0;

			return AnalogX;
		} else if (AnalogX > 0)
		{
			AnalogX -= FlipBack;
			if (AnalogX < 0)
				AnalogX = 0;

			return AnalogX;
		}
		else 
		{
			return orgGetControllerRXAxis.fun(arg);
		}
	}
	
}

int8_t GetControllerLYAxis(DWORD* arg)
{
	return orgGetControllerLYAxis.fun(arg);
}

int8_t GetControllerRXAxis(DWORD* arg)
{
	return orgGetControllerRXAxis.fun(arg);
}

int8_t GetControllerRYAxis(DWORD* arg)
{
	return orgGetControllerRYAxis.fun(arg);
}

void InputTweaks::TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData)
{
	// For keyboard
	if (ProxyInterface == KeyboardInterfaceAddress)
	{
		KeyboardData = (BYTE*)lpvData;
		
		// Ignore Alt + Enter combo
		if ((IsKeyPressed(DIK_LMENU) || IsKeyPressed(DIK_RMENU) && IsKeyPressed(DIK_RETURN)) && 
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

		//TODO exe agnostic
		int8_t RunButton = *(int8_t*)0x01DB8450;
		int8_t AimButton = *(int8_t*)0x01DB8480;
		int8_t ActionButton = *(int8_t*)0x01DB8438;

		AdditionalDebugInfo = "\rLeft Button: ";
		AdditionalDebugInfo.append(LeftMouseButtonState ? "True" : "False");

		if (LeftMouseButtonState)
			SetKey(ActionButton);
		if (RightMouseButtonState)
			SetKey(AimButton);

		AdditionalDebugInfo.append("\rRight Button: ");
		AdditionalDebugInfo.append(RightMouseButtonState ? "True" : "False");

		// Clear 
		KeyboardData = nullptr;
	}

	if (!once)
	{
		once = true;

		//TODO make exe agnostic
		auto pattern = hook::pattern("e8 36 16 04 00");
		orgGetControllerLXAxis.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetControllerLXAxis, true).get();
		pattern = hook::pattern("e8 26 16 04 00");
		orgGetControllerLYAxis.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetControllerLYAxis, true).get();
		pattern = hook::pattern("e8 13 16 04 00");
		orgGetControllerRXAxis.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetControllerRXAxis, true).get();
		pattern = hook::pattern("e8 03 16 04 00");
		orgGetControllerRYAxis.fun = injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(0), GetControllerRYAxis, true).get();
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
	/*
	if (AxisSum != 0)
		Logging::LogDebug() << __FUNCTION__ << " X Axis: " << AxisSum;
	*/
	return AxisSum;
}

int32_t InputTweaks::GetMouseRelYChange()
{
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