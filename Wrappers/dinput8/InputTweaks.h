#pragma once

#include "dinput8wrapper.h"
#include <bitset>

class InputTweaks
{
private:
	LPDIRECTINPUTDEVICE8A KeyboardInterfaceAddress = nullptr;
	LPDIRECTINPUTDEVICE8A MouseInterfaceAddress = nullptr;

	bool IsKeyPressed(int keyIndex);
	void ClearKey(int keyIndex);

public:
	void SetKeyboardInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	void SetMouseInterfaceAddr(LPDIRECTINPUTDEVICE8A ProxyInterface);
	
	void TweakGetDeviceState(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbData, LPVOID lpvData);
	void TweakGetDeviceData(LPDIRECTINPUTDEVICE8A ProxyInterface, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
};
