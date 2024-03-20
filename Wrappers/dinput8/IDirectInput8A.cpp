/**
* Copyright (C) 2024 Elisha Riedlinger
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

#include "dinput8wrapper.h"
#include "Common\FileSystemHooks.h"

HRESULT m_IDirectInput8A::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IDirectInput8A || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectInput8A::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectInput8A::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		delete this;
	}

	return ref;
}

HRESULT m_IDirectInput8A::CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8A *lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (rguid != GUID_SysMouse && rguid != GUID_SysKeyboard)
	{
		IsControllerConnected = true;
		Logging::Log() << "Using gamepad device GUID: " << rguid;
	}

	HRESULT hr = ProxyInterface->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);

	if (SUCCEEDED(hr) && lplpDirectInputDevice)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Created device!");

		*lplpDirectInputDevice = new m_IDirectInputDevice8A(*lplpDirectInputDevice);
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Failed! Error: " << (DIERR)hr;
	}

	return hr;
}

HRESULT m_IDirectInput8A::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	static bool IsUsingXidi = false;

	// Log attached gamepads
	static bool RunOnceFlag = true;
	if (RunOnceFlag)
	{
		RunOnceFlag = false;

		struct EnumDevice
		{
			static BOOL CALLBACK EnumDeviceCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
			{
				UNREFERENCED_PARAMETER(pvRef);

				Logging::Log() << "|- Name: '" << lpddi->tszProductName << "' GUID: " << lpddi->guidInstance;

				if (RemoveForceFeedbackFilter == AUTO_REMOVE_FORCEFEEDBACK && isInString<const char*>(lpddi->tszProductName, "Xidi Virtual Controller", strlen(lpddi->tszProductName)))
				{
					IsUsingXidi = true;
				}

				return DIENUM_CONTINUE;
			}
		};

		Logging::Log() << "|----------- GAMEPADS -----------";
		ProxyInterface->EnumDevices(dwDevType, EnumDevice::EnumDeviceCallback, nullptr, DIEDFL_ATTACHEDONLY);
		Logging::Log() << "|--------------------------------";
	}

	if ((RemoveForceFeedbackFilter == REMOVE_FORCEFEEDBACK || IsUsingXidi) && (dwFlags & DIEDFL_FORCEFEEDBACK))
	{
		Logging::Log() << "Removing Force Feedback filter!";
		dwFlags &= ~DIEDFL_FORCEFEEDBACK;
	}

	return ProxyInterface->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

HRESULT m_IDirectInput8A::GetDeviceStatus(REFGUID rguidInstance)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetDeviceStatus(rguidInstance);
}

HRESULT m_IDirectInput8A::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->RunControlPanel(hwndOwner, dwFlags);
}

HRESULT m_IDirectInput8A::Initialize(HINSTANCE hinst, DWORD dwVersion)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Initialize(hinst, dwVersion);
}

HRESULT m_IDirectInput8A::FindDevice(REFGUID rguidClass, LPCSTR ptszName, LPGUID pguidInstance)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->FindDevice(rguidClass, ptszName, pguidInstance);
}

HRESULT m_IDirectInput8A::EnumDevicesBySemantics(LPCSTR ptszUserName, LPDIACTIONFORMATA lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCBA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (!lpCallback)
	{
		return E_INVALIDARG;
	}

	struct EnumDevice
	{
		LPVOID pvRef;
		LPDIENUMDEVICESBYSEMANTICSCBA lpCallback;

		static BOOL CALLBACK EnumDeviceCallback(LPCDIDEVICEINSTANCEA lpddi, LPDIRECTINPUTDEVICE8A lpdid, DWORD dwFlags, DWORD dwRemaining, LPVOID pvRef)
		{
			EnumDevice *self = (EnumDevice*)pvRef;

			if (lpdid)
			{
				lpdid = ProxyAddressLookupTableDinput8.FindAddress<m_IDirectInputDevice8A>(lpdid);
			}

			return self->lpCallback(lpddi, lpdid, dwFlags, dwRemaining, self->pvRef);
		}
	} CallbackContext;
	CallbackContext.pvRef = pvRef;
	CallbackContext.lpCallback = lpCallback;

	return ProxyInterface->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, EnumDevice::EnumDeviceCallback, &CallbackContext, dwFlags);
}

HRESULT m_IDirectInput8A::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMSA lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}
