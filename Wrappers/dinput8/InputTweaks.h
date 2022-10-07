#pragma once

#include "dinput8wrapper.h"
#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\hooking.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "Patches\Patches.h"
#include <chrono>
#include <bitset>

#define EVENT_LOAD_SCR        0x00
#define EVENT_LOAD_ROOM       0x01
#define EVENT_MAIN_MENU       0x02
#define EVENT_IN_GAME         0x04
#define EVENT_MAP             0x05
#define EVENT_INVENTORY       0x06
#define EVENT_OPTION_FMV      0x07
#define EVENT_MEMO_LIST       0x08
#define EVENT_SAVE_SCREEN     0x09
#define EVENT_GAME_RESULT_ONE 0x0A
#define EVENT_GAME_RESULT_TWO 0x0B
#define EVENT_COMING_SOON     0x0C
#define EVENT_GAME_OVER       0x0D
#define EVENT_FMV             0x0F
#define EVENT_PAUSE_MENU      0x10

#define KEY_SET   0x80
#define KEY_CLEAR 0x00

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

class InputTweaks
{
private:
	LPDIRECTINPUTDEVICE8A KeyboardInterfaceAddress = nullptr;
	LPDIRECTINPUTDEVICE8A MouseInterfaceAddress = nullptr;

	BYTE* KeyboardData = nullptr;
	LPDIDEVICEOBJECTDATA MouseData = nullptr;
	DWORD MouseDataSize = 0;

	bool IsKeyPressed(int KeyIndex);
	void ReadMouseButtons();
	void ClearKey(int KeyIndex);
	void SetKey(int KeyIndex);
	int32_t GetMouseRelXChange();
	int32_t GetMouseRelYChange();

public:
	void SetKeyboardInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	void SetMouseInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	void RemoveAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	
	void TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData);
	void TweakGetDeviceData(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
};

extern InputTweaks InputTweaksRef;