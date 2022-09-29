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

BYTE* KeyboardData = nullptr;
LPDIDEVICEOBJECTDATA MouseData = nullptr;
DWORD MouseDataSize = NULL;

bool once = false;

injector::hook_back<int32_t(__cdecl*)(DWORD*)> orgGetControllerLXAxis;
injector::hook_back<int32_t(__cdecl*)(DWORD*)> orgGetControllerLYAxis;
injector::hook_back<int32_t(__cdecl*)(DWORD*)> orgGetControllerRXAxis;
injector::hook_back<int32_t(__cdecl*)(DWORD*)> orgGetControllerRYAxis;

int32_t GetControllerLXAxis(DWORD* arg)
{

	return orgGetControllerRXAxis.fun(arg);
}

int32_t GetControllerLYAxis(DWORD* arg)
{


	return orgGetControllerLYAxis.fun(arg);
}

int32_t GetControllerRXAxis(DWORD* arg)
{

	return orgGetControllerRXAxis.fun(arg);
}

int32_t GetControllerRYAxis(DWORD* arg)
{

	return orgGetControllerRYAxis.fun(arg);
}

/*
	Animation ids:
		Standing Still:
			Idling = 1
			Rotating left = 6
			Rotating right = 7
		Walking:
			Backwards = 8
			Forwards = 9
		Running:
			Low Stamina = 13
			Full Speed = 14
	*/

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

		//int32_t* AnalogFlag = (int32_t*)0x01FB806A;
		//float* TurnAmount = (float*)0x01fb8054;

		//*enableMovementHook = 0x1;
		//*RotValue = 1.;


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
	int32_t AxisSum = 0;

	if (!MouseData || MouseDataSize == 0)
		return AxisSum;

	for (int i = 0; i < MouseDataSize; i++)
		if (MouseData->dwOfs == DIMOFS_Y)
			AxisSum += (int32_t)MouseData->dwData;

	return AxisSum;
}

