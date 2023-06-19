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
#include "Patches\Patches.h"

std::vector<m_IDirectSoundBuffer8*> BufferList;

HRESULT m_IDirectSoundBuffer8::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if ((riid == IID_IDirectSoundBuffer || riid == IID_IDirectSoundBuffer8 || riid == IID_IUnknown) && ppvObj)
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

ULONG m_IDirectSoundBuffer8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->AddRef();
}

ULONG m_IDirectSoundBuffer8::Release()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Check if audio needs to be stopped
	if (IsStopSet)
	{
		ProxyInterface->Stop();
	}

	ULONG x = ProxyInterface->Release();

	if (x == 0)
	{
		delete this;
	}

	return x;
}

HRESULT m_IDirectSoundBuffer8::GetCaps(LPDSBCAPS pDSBufferCaps)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->GetCaps(pDSBufferCaps);
}

HRESULT m_IDirectSoundBuffer8::GetCurrentPosition(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	// Disable sound when in Game Results
	if (CheckGameResults())
	{
		Stop();
	}

	return ProxyInterface->GetCurrentPosition(pdwCurrentPlayCursor, pdwCurrentWriteCursor);
}

HRESULT m_IDirectSoundBuffer8::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);
}

HRESULT m_IDirectSoundBuffer8::GetVolume(LPLONG plVolume)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	if (!plVolume)
	{
		return DSERR_INVALIDPARAM;
	}

	// Set last volume
	if (IsMasterVolumeSet || IsStopSet)
	{
		*plVolume = RequestedVolume;
		return DS_OK;
	}

	return ProxyInterface->GetVolume(plVolume);
}

HRESULT m_IDirectSoundBuffer8::GetPan(LPLONG plPan)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->GetPan(plPan);
}

HRESULT m_IDirectSoundBuffer8::GetFrequency(LPDWORD pdwFrequency)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->GetFrequency(pdwFrequency);
}

HRESULT m_IDirectSoundBuffer8::GetStatus(LPDWORD pdwStatus)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	// Disable sound when in Game Results
	if (CheckGameResults())
	{
		Stop();
	}

	return ProxyInterface->GetStatus(pdwStatus);
}

HRESULT m_IDirectSoundBuffer8::Initialize(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (pDirectSound)
	{
		pDirectSound = static_cast<m_IDirectSound8 *>(pDirectSound)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(pDirectSound, pcDSBufferDesc);
}

HRESULT m_IDirectSoundBuffer8::Lock(DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	// Check if audio needs to be stopped
	if (IsStopSet)
	{
		ProxyInterface->Stop();
	}

	return ProxyInterface->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
}

HRESULT m_IDirectSoundBuffer8::Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	// Disable sound when in Game Results
	if (CheckGameResults())
	{
		return DS_OK;
	}

	if (IsStopSet)
	{
		// Reset volume
		ProxyInterface->SetVolume(CurrentVolume);

		// Reset flag
		IsStopSet = false;
	}

	if (EnableMasterVolume)
	{
		if (!IsMasterVolumeSet)
		{
			GetVolume(&RequestedVolume);
		}
		SetVolume(RequestedVolume);
	}

	return ProxyInterface->Play(dwReserved1, dwPriority, dwFlags);
}

HRESULT m_IDirectSoundBuffer8::SetCurrentPosition(DWORD dwNewPosition)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	// Check if audio needs to be stopped
	if (IsStopSet)
	{
		ProxyInterface->Stop();
	}

	return ProxyInterface->SetCurrentPosition(dwNewPosition);
}

HRESULT m_IDirectSoundBuffer8::SetFormat(LPCWAVEFORMATEX pcfxFormat)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->SetFormat(pcfxFormat);
}

HRESULT m_IDirectSoundBuffer8::SetVolume(LONG lVolume)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	if (lVolume < DSBVOLUME_MIN || lVolume > DSBVOLUME_MAX)
	{
		return DSERR_INVALIDPARAM;
	}

	LONG NewVolume = lVolume;

	if (EnableMasterVolume)
	{
		float VolumePercentage = (ConfigData.VolumeLevel < 16) ? (10000.0f + (float)VolumeArray[ConfigData.VolumeLevel]) / 10000.0f : 1.0f;

		lVolume = (LONG)((lVolume - DSBVOLUME_MIN) * VolumePercentage) + DSBVOLUME_MIN;
	}

	HRESULT hr;

	// Assign new volume
	if (IsStopSet)
	{
		hr = DS_OK;
	}
	else
	{
		hr = ProxyInterface->SetVolume(lVolume);
	}

	if (SUCCEEDED(hr))
	{
		RequestedVolume = NewVolume;
		CurrentVolume = lVolume;
		if (EnableMasterVolume)
		{
			LastVolumeSet = ConfigData.VolumeLevel;
			if (!IsMasterVolumeSet)
			{
				IsMasterVolumeSet = true;
				SetNewVolume();
			}
		}
	}

	return hr;
}

HRESULT m_IDirectSoundBuffer8::SetPan(LONG lPan)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->SetPan(lPan);
}

HRESULT m_IDirectSoundBuffer8::SetFrequency(DWORD dwFrequency)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->SetFrequency(dwFrequency);
}

HRESULT m_IDirectSoundBuffer8::Stop()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	if (AudioClipDetection)
	{
		if (IsStopSet)
		{
			SetStopped();
			return DS_OK;
		}

		// Lower volume
		ProxyInterface->SetVolume(DSBVOLUME_MIN);

		// Get current time
		static FILETIME SystemTimeAsFileTime;
		GetSystemTimeAsFileTime(&SystemTimeAsFileTime);
		LastStopTime.HighPart = SystemTimeAsFileTime.dwHighDateTime;
		LastStopTime.LowPart = SystemTimeAsFileTime.dwLowDateTime;

		// Set pending stop
		IsStopSet = true;

		// Return
		return DS_OK;
	}

	// Return
	return ProxyInterface->Stop();
}

HRESULT m_IDirectSoundBuffer8::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	HRESULT hr = ProxyInterface->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);

	SetStopped();

	// Disable sound when in Game Results
	if (CheckGameResults())
	{
		Stop();
	}

	return hr;
}

HRESULT m_IDirectSoundBuffer8::Restore()
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	if (EnableMasterVolume)
	{
		SetVolume(RequestedVolume);
	}

	return ProxyInterface->Restore();
}

// IDirectSoundBuffer8 methods
HRESULT m_IDirectSoundBuffer8::SetFX(DWORD dwEffectsCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->SetFX(dwEffectsCount, pDSFXDesc, pdwResultCodes);
}

HRESULT m_IDirectSoundBuffer8::AcquireResources(DWORD dwFlags, DWORD dwEffectsCount, LPDWORD pdwResultCodes)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	return ProxyInterface->AcquireResources(dwFlags, dwEffectsCount, pdwResultCodes);
}

HRESULT m_IDirectSoundBuffer8::GetObjectInPath(REFGUID rguidObject, DWORD dwIndex, REFGUID rguidInterface, LPVOID *ppObject)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	SetStopped();

	HRESULT hr = ProxyInterface->GetObjectInPath(rguidObject, dwIndex, rguidInterface, ppObject);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(rguidInterface, ppObject);
	}

	return hr;
}

// Helper functions
bool m_IDirectSoundBuffer8::CheckGameResults()
{
	if (IsInGameResults && GetEventIndex() == 2)
	{
		IsInGameResults = false;
	}

	return (IsInGameResults && (GetEventIndex() == 10 || GetEventIndex() == 11));
}

void m_IDirectSoundBuffer8::CheckMasterVolume()
{
	if (EnableMasterVolume && LastVolumeSet != ConfigData.VolumeLevel)
	{
		DWORD Status = 0;
		if (SUCCEEDED(ProxyInterface->GetStatus(&Status)) && (Status & DSBSTATUS_PLAYING))
		{
			SetVolume(RequestedVolume);
		}
	}
}

void m_IDirectSoundBuffer8::SetStopped()
{
	if (IsStopSet)
	{
		// Get current time
		FILETIME SystemTimeAsFileTime;
		GetSystemTimeAsFileTime(&SystemTimeAsFileTime);
		ULARGE_INTEGER CurrentTime = { SystemTimeAsFileTime.dwLowDateTime, SystemTimeAsFileTime.dwHighDateTime };

		// Check if time delay is up
		if (CurrentTime.QuadPart > LastStopTime.QuadPart + (10000ULL * ((AudioFadeOutDelayMS) ? AudioFadeOutDelayMS : 5)))
		{
			// Stop audio
			ProxyInterface->Stop();

			// Reset volume
			ProxyInterface->SetVolume(CurrentVolume);

			// Reset flag
			IsStopSet = false;
		}
	}
}

void SetNewVolume()
{
	for (auto pBuffer : BufferList)
	{
		pBuffer->CheckMasterVolume();
	}
}
