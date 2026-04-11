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

#include "dsoundwrapper.h"
#include <fstream>
#include <shlwapi.h>
#include "Common\ScopeGuard.h"
#include "Patches\Patches.h"

namespace {
	CRITICAL_SECTION dscs = {};
	constexpr DWORD MaxBuffers = 4;		// 0 = Save; 1 = Load; 2 = Flashlight; 3 = Footsteps/ClosetCutscene/UnusedLauraLetter
	m_IDirectSound8* pCurrentDirectSound = nullptr;
	m_IDirectSoundBuffer8* pDirectSoundWavBuffer[MaxBuffers] = {};
}

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

	EnterCriticalSection(&dscs);

	ULONG count = ProxyInterface->Release();

	// Release all buffers before destroying device
	if (count == 1)
	{
		for (DWORD x = 0; x < MaxBuffers; x++)
		{
			ReleaseWavSoundBuffer(x);
		}
		count = ProxyInterface->Release();	// Release extra ref count
	}

	LeaveCriticalSection(&dscs);

	if (count == 0)
	{
		delete this;
	}

	return count;
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
HRESULT m_IDirectSound8::VerifyCertification(LPDWORD pdwCertified)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->VerifyCertification(pdwCertified);
}

// Helper functions
void m_IDirectSound8::InitDevice()
{
	pCurrentDirectSound = this;

	ProxyInterface->AddRef();	// Add extra ref count for tracking extra sound buffers created

	InitializeCriticalSection(&dscs);
}

void m_IDirectSound8::ReleaseDevice()
{
	EnterCriticalSection(&dscs);
	pCurrentDirectSound = nullptr;
	for (DWORD x = 0; x < MaxBuffers; x++)
	{
		pDirectSoundWavBuffer[x] = nullptr;
	}
	LeaveCriticalSection(&dscs);

	DeleteCriticalSection(&dscs);
	dscs = {};
}

HRESULT m_IDirectSound8::CreateWavSoundBuffer(DWORD BufferID, DSBUFFERDESC& dsbd, m_IDirectSoundBuffer8** ppDSBuffer)
{
	// Check buffer
	if (!ppDSBuffer)
	{
		Logging::Log() << __FUNCTION__ << " Error: bad buffer address!";
		return DSERR_INVALIDPARAM;
	}
	*ppDSBuffer = nullptr;

	// Check buffer ID
	if (BufferID + 1 > MaxBuffers)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid buffer ID! " << BufferID;
		return DSERR_INVALIDPARAM;
	}

	// Release all buffers that are not playing
	for (DWORD x = 0; x < MaxBuffers; x++)
	{
		if (pDirectSoundWavBuffer[x])
		{
			DWORD Status = 0;
			if (SUCCEEDED(pDirectSoundWavBuffer[x]->GetStatus(&Status)) && !(Status & DSBSTATUS_PLAYING))
			{
				ReleaseWavSoundBuffer(x);
			}
		}
	}

	// Release current buffer
	ReleaseWavSoundBuffer(BufferID);

	// Create a sound buffer
	LPDIRECTSOUNDBUFFER pBuffer = nullptr;
	HRESULT hr = ProxyInterface->CreateSoundBuffer(&dsbd, &pBuffer, nullptr);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create buffer! ID: " << BufferID << " hr: " << (DSERR)hr;
		return hr;
	}
	pDirectSoundWavBuffer[BufferID] = new m_IDirectSoundBuffer8((IDirectSoundBuffer8*)pBuffer);

	*ppDSBuffer = pDirectSoundWavBuffer[BufferID];

	return hr;
}

void m_IDirectSound8::ReleaseWavSoundBuffer(DWORD BufferID)
{
	if (pDirectSoundWavBuffer[BufferID])
	{
		// Mute volume
		pDirectSoundWavBuffer[BufferID]->SetVolume(0);

		// Release buffer
		UINT ref = pDirectSoundWavBuffer[BufferID]->Release();
		if (ref != 0)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to the sound buffer " << ref;
		}
		pDirectSoundWavBuffer[BufferID] = nullptr;
	}
}

HRESULT m_IDirectSound8::ParseWavFile(const char* filePath, DSBUFFERDESC& dsbd, WAVEFORMATEX& waveFormat, std::vector<char>& AudioBuffer)
{
	// Check for nullptr
	if (!filePath)
	{
		Logging::Log() << __FUNCTION__ << " Error: bad file name!";
		return DSERR_INVALIDPARAM;
	}

	// Check if WAV file exists
	if (!PathFileExistsA(filePath))
	{
		Logging::Log() << __FUNCTION__ << "Error: '" << filePath << "' not found!";
		return DSERR_INVALIDPARAM;
	}

	// Open the WAV file
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to open the file: " << filePath;
		return DSERR_GENERIC;
	}

	// Handle reading the file and loading it into the buffer
	HRESULT hr = DSERR_GENERIC;
	do {

		// Read the RIFF header
		char riffHeader[4] = {};
		file.read(riffHeader, sizeof(riffHeader));
		if (strncmp(riffHeader, "RIFF", 4) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: WAV file missing 'RIFF' chunk: " << filePath;
			break;
		}

		// Read the file size (skip it for now)
		file.seekg(4, std::ios::cur);

		// Read the WAVE header
		char waveHeader[4] = {};
		file.read(waveHeader, sizeof(waveHeader));
		if (strncmp(waveHeader, "WAVE", 4) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: WAV file missing 'WAVE' chunk: " << filePath;
			break;
		}

		// Read the 'fmt ' chunk
		char fmtChunk[4] = {};
		file.read(fmtChunk, sizeof(fmtChunk));
		if (strncmp(fmtChunk, "fmt ", 4) != S_OK)
		{
			Logging::Log() << __FUNCTION__ << " Error: WAV file missing 'fmt ' chunk: " << filePath;
			break;
		}

		// Read the size of the 'fmt ' chunk
		DWORD fmtSize = 0;
		file.read(reinterpret_cast<char*>(&fmtSize), sizeof(fmtSize));

		// Read the 'fmt ' data
		waveFormat = {};
		file.read(reinterpret_cast<char*>(&waveFormat), min(fmtSize, sizeof(WAVEFORMATEX)));

		// Find the 'data' chunk
		char chunkHeader[4] = {};
		file.read(chunkHeader, sizeof(chunkHeader));
		while (strncmp(chunkHeader, "data", 4) != S_OK && !file.eof())
		{
			if (chunkHeader[1] == 'd' && chunkHeader[2] == 'a' && chunkHeader[3] == 't')
			{
				chunkHeader[0] = 'd';
				chunkHeader[1] = 'a';
				chunkHeader[2] = 't';
				file.read(&chunkHeader[3], 1);
			}
			else if (chunkHeader[2] == 'd' && chunkHeader[3] == 'a')
			{
				chunkHeader[0] = 'd';
				chunkHeader[1] = 'a';
				file.read(&chunkHeader[2], 2);
			}
			else if (chunkHeader[3] == 'd')
			{
				chunkHeader[0] = 'd';
				file.read(&chunkHeader[1], 3);
			}
			else
			{
				file.read(chunkHeader, sizeof(chunkHeader));
			}
		}
		if (file.eof())
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to find 'data' chunk: " << filePath;
			break;
		}

		// Read the size of the 'data' chunk
		DWORD dataSize = 0;
		file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

		// Allocate a buffer for the audio data
		AudioBuffer.resize(dataSize);

		// Read the audio data
		file.read(AudioBuffer.data(), dataSize);

		// Check how much data was actually read
		std::streamsize ActualBytesRead = file.gcount();
		if (ActualBytesRead < dataSize)
		{
			Logging::Log() << __FUNCTION__ << " Error: Failed to read full buffer! size: " << dataSize << " read: " << ActualBytesRead << " file: " << filePath;
			break;
		}

		// Set the buffer desc
		dsbd.dwBufferBytes = dataSize;
		dsbd.lpwfxFormat = &waveFormat;

		// Set hr to success
		hr = DS_OK;

	} while (false);

	// Close and exit
	file.close();
	return hr;
}

HRESULT m_IDirectSound8::PlayWavFile(const char* filePath, DWORD BufferID, bool useSfxVolume)
{
	if (!filePath)
	{
		Logging::Log() << __FUNCTION__ << " Error: bad file name!";
		return DSERR_INVALIDPARAM;
	}

	ScopedCriticalSection ThreatLock(&dscs);

	if (!pCurrentDirectSound)
	{
		Logging::Log() << __FUNCTION__ << " Error: current DriectSound8 not setup!";
		return DSERR_DS8_REQUIRED;
	}

	// Setting the buffer desc
	DSBUFFERDESC dsbd = {};
	dsbd = {};
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLPOSITIONNOTIFY;

	// Buffer variables
	WAVEFORMATEX waveFormat = {};
	std::vector<char> AudioBuffer;

	// Parse WAV files
	HRESULT hr = ParseWavFile(filePath, dsbd, waveFormat, AudioBuffer);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to load WAV! file: " << filePath << " hr: " << (DSERR)hr;
		return hr;
	}

	// Create sound buffer
	m_IDirectSoundBuffer8* pBuffer = nullptr;
	hr = pCurrentDirectSound->CreateWavSoundBuffer(BufferID, dsbd, &pBuffer);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create buffer! file: " << filePath << " hr: " << (DSERR)hr;
		return hr;
	}

	// Lock the buffer and copy the audio data
	LPVOID pData = nullptr;
	DWORD dataSize = dsbd.dwBufferBytes;

	hr = pBuffer->Lock(0, dataSize, &pData, &dataSize, nullptr, nullptr, DSBLOCK_ENTIREBUFFER);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to lock buffer! file: " << filePath << " hr: " << (DSERR)hr;
		return hr;
	}

	memcpy(pData, AudioBuffer.data(), dataSize);

	pBuffer->Unlock(pData, dataSize, nullptr, 0);

	// Get the volume level
	LONG Volume = DSBVOLUME_MAX;
	if (useSfxVolume)
	{
		BYTE SFXVolumeLevel = GetSFXVolume();
		if (SFXVolumeLevel < 16)
		{
			Volume = VolumeArray[SFXVolumeLevel];
		}
	}

	// Set the volume
	pBuffer->SetVolume(Volume);

	// Play the loaded WAV file
	hr = pBuffer->Play(0, 0, 0);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Play failed! file: " << filePath << " hr: " << (DSERR)hr;
		return hr;
	}

	Logging::LogDebug() << __FUNCTION__ << " Playing sound: " << filePath;

	return hr;
}

HRESULT m_IDirectSound8::PlayWavMemory(const WAVEFORMATEX* waveFormat, const BYTE* audioData, DWORD audioSize, DWORD BufferID, bool useSfxVolume)
{
	if (!waveFormat || !audioData || audioSize == 0)
	{
		Logging::Log() << __FUNCTION__ << " Error: bad audio parameters!";
		return DSERR_INVALIDPARAM;
	}

	ScopedCriticalSection ThreatLock(&dscs);

	if (!pCurrentDirectSound)
	{
		Logging::Log() << __FUNCTION__ << " Error: current DirectSound8 not setup!";
		return DSERR_DS8_REQUIRED;
	}

	// Setting the buffer desc
	DSBUFFERDESC dsbd = {};
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLPOSITIONNOTIFY;
	dsbd.dwBufferBytes = audioSize;
	dsbd.lpwfxFormat = const_cast<WAVEFORMATEX*>(waveFormat);

	// Create sound buffer
	m_IDirectSoundBuffer8* pBuffer = nullptr;
	HRESULT hr = pCurrentDirectSound->CreateWavSoundBuffer(BufferID, dsbd, &pBuffer);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create buffer! ID: " << BufferID << " hr: " << (DSERR)hr;
		return hr;
	}

	// Lock the buffer and copy the audio data
	LPVOID pData1 = nullptr;
	DWORD dataSize1 = 0;
	LPVOID pData2 = nullptr;
	DWORD dataSize2 = 0;

	hr = pBuffer->Lock(0, audioSize, &pData1, &dataSize1, &pData2, &dataSize2, DSBLOCK_ENTIREBUFFER);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Lock failed! ID: " << BufferID << " hr: " << (DSERR)hr;
		return hr;
	}

	if (pData1 && dataSize1)
	{
		memcpy(pData1, audioData, dataSize1);
	}
	if (pData2 && dataSize2)
	{
		memcpy(pData2, audioData + dataSize1, dataSize2);
	}

	pBuffer->Unlock(pData1, dataSize1, pData2, dataSize2);

	// Get the volume level
	LONG Volume = DSBVOLUME_MAX;
	if (useSfxVolume)
	{
		BYTE SFXVolumeLevel = GetSFXVolume();
		if (SFXVolumeLevel < 16)
		{
			Volume = VolumeArray[SFXVolumeLevel];
		}
	}

	// Set the volume
	pBuffer->SetVolume(Volume);

	// Play the loaded WAV file
	hr = pBuffer->Play(0, 0, 0);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Play failed! ID: " << BufferID << " hr: " << (DSERR)hr;
		return hr;
	}

	Logging::LogDebug() << __FUNCTION__ << " Playing in-memory WAV buffer. ID: " << BufferID;

	return hr;
}

HRESULT m_IDirectSound8::StopWavSoundBuffer(DWORD BufferID)
{
	if (BufferID + 1 > MaxBuffers)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid buffer ID!";
		return DSERR_INVALIDPARAM;
	}

	ScopedCriticalSection ThreatLock(&dscs);

	if (!pCurrentDirectSound)
	{
		Logging::Log() << __FUNCTION__ << " Error: current DriectSound8 not setup!";
		return DSERR_DS8_REQUIRED;
	}

	if (pDirectSoundWavBuffer[BufferID])
	{
		pCurrentDirectSound->ReleaseWavSoundBuffer(BufferID);
	}

	return DS_OK;
}
