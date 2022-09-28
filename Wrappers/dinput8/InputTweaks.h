#pragma once

#include "dinput8wrapper.h"
#include <bitset>

class InputTweaks
{
private:
	LPDIRECTINPUTDEVICE8A KeyboardInterfaceAddress = nullptr;
	LPDIRECTINPUTDEVICE8A MouseInterfaceAddress = nullptr;

	BYTE* KeyboardData = nullptr;
	LPDIDEVICEOBJECTDATA MouseData = nullptr;
	DWORD MouseDataSize = 0;

	bool IsKeyPressed(int KeyIndex);
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
