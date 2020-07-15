#pragma once

struct ENUMDEVICEA
{
	bool Stopped = false;
	LPVOID pvRef;
	LPDIENUMDEVICESCALLBACKA lpCallback;
};

class m_IDirectInputEnumDevice
{
public:
	m_IDirectInputEnumDevice() {}
	~m_IDirectInputEnumDevice() {}

	static BOOL CALLBACK EnumDeviceCallbackA(LPCDIDEVICEINSTANCEA, LPVOID);
};

struct ENUMDEVICESEMA
{
	LPVOID pvRef;
	LPDIENUMDEVICESBYSEMANTICSCBA lpCallback;
};

class m_IDirectInputEnumDeviceSemantics
{
public:
	m_IDirectInputEnumDeviceSemantics() {}
	~m_IDirectInputEnumDeviceSemantics() {}

	static BOOL CALLBACK EnumDeviceCallbackA(LPCDIDEVICEINSTANCEA, LPDIRECTINPUTDEVICE8A, DWORD, DWORD, LPVOID);
};

struct ENUMEFFECT
{
	LPVOID pvRef;
	LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback;
};

class m_IDirectInputEnumEffect
{
public:
	m_IDirectInputEnumEffect() {}
	~m_IDirectInputEnumEffect() {}

	static BOOL CALLBACK EnumEffectCallback(LPDIRECTINPUTEFFECT, LPVOID);
};
