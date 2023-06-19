/**
* Copyright (C) 2023 Elisha Riedlinger
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

#include "dsoundwrapper.h"

HRESULT m_IDirectSound8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IDirectSound || riid == IID_IDirectSound8 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return DS_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj);
	}

	return hr;
}

ULONG m_IDirectSound8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectSound8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

// IDirectSound methods
HRESULT m_IDirectSound8::CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((EnableMasterVolume || AudioClipDetection) && pcDSBufferDesc)
	{
		DSBUFFERDESC *DSBufferDesc = (DSBUFFERDESC*)pcDSBufferDesc;
		DSBufferDesc->dwFlags |= DSBCAPS_CTRLVOLUME;
	}

	HRESULT hr;

	DWORD x = 0;
	do {
		hr = ProxyInterface->CreateSoundBuffer(pcDSBufferDesc, ppDSBuffer, pUnkOuter);

		if (FAILED(hr))
		{
			Sleep(100);
		}
	} while (FAILED(hr) && ++x < 100);

	if (SUCCEEDED(hr) && ppDSBuffer)
	{
		LOG_LIMIT(1, __FUNCTION__ << " Created buffer!");

		*ppDSBuffer = new m_IDirectSoundBuffer8((IDirectSoundBuffer8*)*ppDSBuffer);
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Failed! Error: " << (DSERR)hr;
	}

	return hr;
}

HRESULT m_IDirectSound8::GetCaps(LPDSCAPS pDSCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->GetCaps(pDSCaps);
}

HRESULT m_IDirectSound8::DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pDSBufferOriginal)
	{
		pDSBufferOriginal = static_cast<m_IDirectSoundBuffer8 *>(pDSBufferOriginal)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->DuplicateSoundBuffer(pDSBufferOriginal, ppDSBufferDuplicate);

	if (SUCCEEDED(hr) && ppDSBufferDuplicate)
	{
		*ppDSBufferDuplicate = new m_IDirectSoundBuffer8((IDirectSoundBuffer8*)*ppDSBufferDuplicate);
	}

	return hr;
}

HRESULT m_IDirectSound8::SetCooperativeLevel(HWND hwnd, DWORD dwLevel)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (dwLevel == DSSCL_EXCLUSIVE)
	{
		dwLevel = DSSCL_PRIORITY;
	}

	return ProxyInterface->SetCooperativeLevel(hwnd, dwLevel);
}

HRESULT m_IDirectSound8::Compact()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Compact();
}

HRESULT m_IDirectSound8::GetSpeakerConfig(LPDWORD pdwSpeakerConfig)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->GetSpeakerConfig(pdwSpeakerConfig);

	if (pdwSpeakerConfig)
	{
		*pdwSpeakerConfig = (*pdwSpeakerConfig == DSSPEAKER_7POINT1_SURROUND) ? DSSPEAKER_7POINT1 :
			(*pdwSpeakerConfig == DSSPEAKER_5POINT1_SURROUND) ? DSSPEAKER_5POINT1 : *pdwSpeakerConfig;
	}

	return hr;
}

HRESULT m_IDirectSound8::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->SetSpeakerConfig(dwSpeakerConfig);
}

HRESULT m_IDirectSound8::Initialize(LPCGUID pcGuidDevice)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->Initialize(pcGuidDevice);
}

// IDirectSound8 methods
HRESULT  m_IDirectSound8::VerifyCertification(LPDWORD pdwCertified)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->VerifyCertification(pdwCertified);
}
