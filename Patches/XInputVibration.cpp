/**
* Copyright (C) 2019 Adrian Zdanowicz
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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dxguid.lib")

#include <Xinput.h>
#pragma comment(lib, "Xinput9_1_0.lib")

static LPDIRECTINPUTEFFECT originalDInputEffect = nullptr;

// XInput -> DirectInput vibration effect wrapper

// NOTE: This wrapper is very primitive, mostly because it's been tailored specifically for the game
// Silent Hill 2 uses vibration in a very simple manner - submits a periodic wave at 100% force until stopped by the game
// For this use case, we just need to implement Start and Stop
class XInputEffect final : public IDirectInputEffect
{
public:
	virtual HRESULT WINAPI QueryInterface(REFIID, LPVOID*) override
	{
		// Deliberately left unimplemented
		return E_NOTIMPL;
	}

	virtual ULONG WINAPI AddRef(void) override
	{
		// Deliberately left unimplemented
		return 0;
	}

	virtual ULONG WINAPI Release(void) override
	{
		// Deliberately left unimplemented
		return 0;
	}

	virtual HRESULT WINAPI Initialize(HINSTANCE, DWORD, REFGUID) override
	{
		// Deliberately left unimplemented
		return E_NOTIMPL;
	}

	virtual HRESULT WINAPI GetEffectGuid(LPGUID) override
	{
		// Deliberately left unimplemented
		return E_NOTIMPL;
	}

	virtual HRESULT WINAPI GetParameters(LPDIEFFECT, DWORD) override
	{
		// Deliberately left unimplemented
		return E_NOTIMPL;
	}

	virtual HRESULT WINAPI SetParameters(LPCDIEFFECT, DWORD) override
	{
		// Deliberately left empty, SH2 only submits 100% force to SetParameters
		return DI_OK;
	}

	virtual HRESULT WINAPI Start(DWORD dwIterations, DWORD dwFlags) override
	{
		if ( !m_vibrating )
		{
			XINPUT_VIBRATION vib = { FixedVibrationIntensity, 0 };
			XInputSetState( PadNumber, &vib );
			m_vibrating = true;
		}
		return originalDInputEffect != nullptr ? originalDInputEffect->Start( dwIterations, dwFlags ) : DI_OK;
	}

	virtual HRESULT WINAPI Stop(void) override
	{
		if ( m_vibrating )
		{
			XINPUT_VIBRATION vib = { 0, 0 };
			XInputSetState( PadNumber, &vib );
			m_vibrating = false;
		}

		return originalDInputEffect != nullptr ? originalDInputEffect->Stop() : DI_OK;
	}

	virtual HRESULT WINAPI GetEffectStatus(LPDWORD) override
	{
		// Deliberately left unimplemented
		return E_NOTIMPL;
	}

	virtual HRESULT WINAPI Download(void) override
	{
		// Deliberately left unimplemented
		return E_NOTIMPL;
	}

	virtual HRESULT WINAPI Unload(void) override
	{
		// Deliberately left unimplemented
		return E_NOTIMPL;
	}

	virtual HRESULT WINAPI Escape(LPDIEFFESCAPE) override
	{
		// Deliberately left unimplemented
		return E_NOTIMPL;
	}

private:
	static constexpr WORD FixedVibrationIntensity = 65535;
	bool m_vibrating = false;
} StubXInputEffect;


int32_t* DInputGamepadType;
LPDIRECTINPUTEFFECT* directInputEffect;

void (*orgCreateDirectInputGamepad)();
void CreateDirectInputGamepad_Hook()
{
	orgCreateDirectInputGamepad();

	if ( *DInputGamepadType != 0 )
	{
		// If game already created an effect, store it so we can submit data to both DInput and XInput
		originalDInputEffect = *directInputEffect;
		*directInputEffect = &StubXInputEffect;
		*DInputGamepadType = 2;
	}
}

void UpdateXInputVibration()
{
	constexpr BYTE XIVibrationSearchBytes[] { 0x57, 0x33, 0xC0, 0xB9, 0x9F, 0x01, 0x00, 0x00 };
	const DWORD CreateDIGamepadAddr = SearchAndGetAddresses(0x458B30, 0x458D90, 0x458D90, XIVibrationSearchBytes, sizeof(XIVibrationSearchBytes), 0x24);
	if (CreateDIGamepadAddr == 0)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Read out a jump from under this address and replace it with a custom one
	int32_t jmpAddress = 0;
	memcpy( &jmpAddress, (void*)(CreateDIGamepadAddr + 1), sizeof(jmpAddress) );

	constexpr BYTE SetVibrationParametersSearchBytes[] { 0x83, 0xEC, 0x38, 0x83, 0xF8, 0x02, 0x75, 0x39 };
	const DWORD SetVibrationParametersAddr = SearchAndGetAddresses(0x458005, 0x458265, 0x458265, SetVibrationParametersSearchBytes, sizeof(SetVibrationParametersSearchBytes), -0x5);
	if (SetVibrationParametersAddr == 0)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}

	// Update SH2 code
	Logging::Log() << "Enabling XInput Vibration Fix...";
	orgCreateDirectInputGamepad = decltype(orgCreateDirectInputGamepad)(jmpAddress + CreateDIGamepadAddr + 5);
	WriteJMPtoMemory((BYTE*)CreateDIGamepadAddr, CreateDirectInputGamepad_Hook);

	DInputGamepadType = *(int32_t**)(SetVibrationParametersAddr + 1);
	directInputEffect = *(LPDIRECTINPUTEFFECT**)(SetVibrationParametersAddr + 0x34 + 1);
}
