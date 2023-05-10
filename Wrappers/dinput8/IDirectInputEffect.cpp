/**
* Copyright (C) 2020 Adrian Zdanowicz
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

// We need to target older Windows, else Win8+ only xinput1_4 will be used
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

#include "dinput8wrapper.h"
#include "Patches\patches.h"
#include "Common\Utils.h"
#include <dinput.h>
#include <Xinput.h>

typedef DWORD *(WINAPI *XInputSetStateProc)(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);
XInputSetStateProc pXInputSetState = nullptr;

BYTE *IntensityAddr = nullptr;		// IntensityAddr (0 = Off, 1 = Soft, 2 = Normal, 3 = Hard)
m_IDirectInputEffect StubXInputEffect(nullptr);

HRESULT m_IDirectInputEffect::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IDirectInputEffect || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	if (IsStatic || !ProxyInterface)
	{
		return E_NOINTERFACE;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectInputEffect::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (IsStatic || !ProxyInterface)
	{
		return 1;
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectInputEffect::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (IsStatic || !ProxyInterface)
	{
		return 0;
	}

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IDirectInputEffect::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface != nullptr ? ProxyInterface->Initialize(hinst, dwVersion, rguid) : DI_OK;
}

HRESULT m_IDirectInputEffect::GetEffectGuid(LPGUID pguid)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface != nullptr ? ProxyInterface->GetEffectGuid(pguid) : DI_OK;
}

HRESULT m_IDirectInputEffect::GetParameters(LPDIEFFECT peff, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface != nullptr ? ProxyInterface->GetParameters(peff, dwFlags) : DI_OK;
}

HRESULT m_IDirectInputEffect::SetParameters(LPCDIEFFECT peff, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface != nullptr ? ProxyInterface->SetParameters(peff, dwFlags) : DI_OK;
}

HRESULT m_IDirectInputEffect::Start(DWORD dwIterations, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (RestoreVibration && !m_vibrating)
	{
		WORD XIntensity = MaxVibrationIntensity;
		WORD DIntensity = DI_FFNOMINALMAX;
		if (IntensityAddr && *IntensityAddr < 3)
		{
			XIntensity = (WORD)(MaxVibrationIntensity * (*IntensityAddr * (100.0f / 3.0f)));
			DIntensity = (WORD)(DI_FFNOMINALMAX * (*IntensityAddr * (100.0f / 3.0f)));
		}

		// xinput
		if (pXInputSetState)
		{
			XINPUT_VIBRATION vib = { XIntensity, XIntensity };
			pXInputSetState(PadNumber, &vib);
		}

		// dinput
		DIEFFECT diEffect;
		ZeroMemory(&diEffect, sizeof(DIEFFECT));
		diEffect.dwSize = sizeof(DIEFFECT);
		diEffect.dwGain = DIntensity;
		SetParameters(&diEffect, DIEP_GAIN);

		m_vibrating = true;
	}

	return ProxyInterface != nullptr ? ProxyInterface->Start(dwIterations, dwFlags) : DI_OK;
}

HRESULT m_IDirectInputEffect::Stop()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (RestoreVibration && m_vibrating)
	{
		if (pXInputSetState)
		{
			XINPUT_VIBRATION vib = { 0, 0 };
			pXInputSetState(PadNumber, &vib);
		}
		m_vibrating = false;
	}

	return ProxyInterface != nullptr ? ProxyInterface->Stop() : DI_OK;
}

HRESULT m_IDirectInputEffect::GetEffectStatus(LPDWORD pdwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface != nullptr ? ProxyInterface->GetEffectStatus(pdwFlags) : DI_OK;
}

HRESULT m_IDirectInputEffect::Download()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface != nullptr ? ProxyInterface->Download() : DI_OK;
}

HRESULT m_IDirectInputEffect::Unload()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface != nullptr ? ProxyInterface->Unload() : DI_OK;
}

HRESULT m_IDirectInputEffect::Escape(LPDIEFFESCAPE pesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface != nullptr ? ProxyInterface->Escape(pesc) : DI_OK;
}


//**************************
// Other fucntions
//**************************

int32_t* DInputGamepadType;
LPDIRECTINPUTEFFECT* directInputEffect;

void(*orgCreateDirectInputGamepad)();
void CreateDirectInputGamepad_Hook()
{
	orgCreateDirectInputGamepad();

	if (*DInputGamepadType != 0)
	{
		// If game already created an effect, store it so we can submit data to both DInput and XInput
		StubXInputEffect.SetProxyInterface(*directInputEffect);
		*directInputEffect = &StubXInputEffect;
		*DInputGamepadType = 2;
	}
}

void PatchXInputVibration()
{
	constexpr BYTE XIVibrationSearchBytes[]{ 0x57, 0x33, 0xC0, 0xB9, 0x9F, 0x01, 0x00, 0x00 };
	const DWORD CreateDIGamepadAddr = SearchAndGetAddresses(0x458B30, 0x458D90, 0x458D90, XIVibrationSearchBytes, sizeof(XIVibrationSearchBytes), 0x24, __FUNCTION__);
	if (!CreateDIGamepadAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Read out a jump from under this address and replace it with a custom one
	int32_t jmpAddress = 0;
	memcpy(&jmpAddress, (void*)(CreateDIGamepadAddr + 1), sizeof(jmpAddress));

	constexpr BYTE SetVibrationParametersSearchBytes[]{ 0x83, 0xEC, 0x38, 0x83, 0xF8, 0x02, 0x75, 0x39 };
	const DWORD SetVibrationParametersAddr = SearchAndGetAddresses(0x458005, 0x458265, 0x458265, SetVibrationParametersSearchBytes, sizeof(SetVibrationParametersSearchBytes), -0x5, __FUNCTION__);
	if (!SetVibrationParametersAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Get address for the main menu function that enables rumble
	constexpr BYTE MenuRumbleSearchBytes[]{ 0x53, 0x53, 0x53, 0x68, 0x10, 0x27, 0x00, 0x00, 0xC7, 0x05 };
	DWORD MenuRumbleAddr = SearchAndGetAddresses(0x00497DD3, 0x00498083, 0x00497713, MenuRumbleSearchBytes, sizeof(MenuRumbleSearchBytes), -0x62, __FUNCTION__);
	if (!MenuRumbleAddr)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Load xinput module
	for (auto dllname : { L"xinput1_4.dll" , L"xinput9_1_0.dll",  L"xinput1_3.dll",  L"xinput1_2.dll",  L"xinput1_1.dll" })
	{
		HMODULE dll = LoadLibrary(dllname);
		if (dll)
		{
			pXInputSetState = (XInputSetStateProc)GetProcAddress(dll, "XInputSetState");
			if (pXInputSetState)
			{
				break;
			}
		}
	}

	// Update SH2 code
	Logging::Log() << "Enabling XInput Vibration Fix...";
	orgCreateDirectInputGamepad = decltype(orgCreateDirectInputGamepad)(jmpAddress + CreateDIGamepadAddr + 5);
	WriteJMPtoMemory((BYTE*)CreateDIGamepadAddr, CreateDirectInputGamepad_Hook);

	DInputGamepadType = *(int32_t**)(SetVibrationParametersAddr + 1);
	directInputEffect = *(LPDIRECTINPUTEFFECT**)(SetVibrationParametersAddr + 0x34 + 1);

	// Fix infinite rumble
	BYTE NOP[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
	UpdateMemoryAddress((void*)MenuRumbleAddr, &NOP, sizeof(NOP));
}

void RunInfiniteRumbleFix()
{
	// Get vibration intensity
	if (!IntensityAddr)
	{
		RUNONCE();

		constexpr BYTE IntensitySearchBytes[]{ 0x6A, 0x60, 0x75, 0x04, 0x6A, 0x3F, 0xEB, 0x02, 0x6A, 0x1F, 0x6A, 0x3F, 0x6A, 0x3F, 0xE8 };
		IntensityAddr = (BYTE*)ReadSearchedAddresses(0x00461735, 0x00461995, 0x00461995, IntensitySearchBytes, sizeof(IntensitySearchBytes), -0x04, __FUNCTION__);
		if (!IntensityAddr)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
	}

	// Get rumble address
	static BYTE *RumbleAddress = nullptr;
	if (!RumbleAddress)
	{
		RUNONCE();

		// Get address for the rumble flag
		constexpr BYTE RumbleSearchBytes[]{ 0x68, 0x10, 0x01, 0x00, 0x00, 0x50, 0xFF, 0x52, 0x24, 0x33, 0xD2, 0xB9 };
		RumbleAddress = (BYTE*)ReadSearchedAddresses(0x004589C1, 0x00458C21, 0x00458C21, RumbleSearchBytes, sizeof(RumbleSearchBytes), 0x0C, __FUNCTION__);
		if (!RumbleAddress)
		{
			Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
			return;
		}
		RumbleAddress -= 0x02;
	}

	// Get pause menu address
	static BYTE *EventIndex = GetEventIndexPointer();

	// Get transition state address
	static DWORD *TransitionState = GetTransitionStatePointer();

	// Checking address pointer
	if (!EventIndex || !TransitionState)
	{
		RUNONCE();

		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Disable rumble
	static bool ValueSet = false;
	if (*EventIndex != 0x04 || GetRoomID() == 0x00 || *TransitionState == 0x01 || *TransitionState == 0x02 || *TransitionState == 0x03 || LostWindowFocus)
	{
		if (!ValueSet)
		{
			BYTE Value = 0x00;
			UpdateMemoryAddress((void*)RumbleAddress, &Value, sizeof(BYTE));
			ValueSet = true;
			if (*TransitionState == 0x01 || *TransitionState == 0x02 || *TransitionState == 0x03 || LostWindowFocus)
			{
				StubXInputEffect.Stop();
			}
		}
	}
	else if (ValueSet)
	{
		BYTE Value = 0x01;
		UpdateMemoryAddress((void*)RumbleAddress, &Value, sizeof(BYTE));
		ValueSet = false;
	}
}
