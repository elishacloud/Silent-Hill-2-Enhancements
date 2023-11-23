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
#include <fstream>
#include <shlwapi.h>
#include "Patches\Patches.h"

CRITICAL_SECTION dscs = {};
constexpr DWORD MaxBuffers = 3;
m_IDirectSound8* pCurrentDirectSound = nullptr;
m_IDirectSoundBuffer8* pDirectSoundWavBuffer[MaxBuffers] = {};

HRESULT ParseWavFile(const char* filePath, DSBUFFERDESC& dsbd, WAVEFORMATEX& waveFormat, std::vector<char>& AudioBuffer);
void ReleaseSoundBuffer(DWORD BifferID);

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

	ULONG x = ProxyInterface->Release();

	LeaveCriticalSection(&dscs);

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
HRESULT m_IDirectSound8::VerifyCertification(LPDWORD pdwCertified)
{
	Logging::LogDebug() << __FUNCTION__ << " (" << this << ")";

	return ProxyInterface->VerifyCertification(pdwCertified);
}

// Helper functions
void m_IDirectSound8::InitDevice()
{
	pCurrentDirectSound = this;

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

HRESULT m_IDirectSound8::CreateWAVSoundBuffer(const char* filePath, m_IDirectSoundBuffer8** ppDSBuffer)
{
	// Check buffer
	if (!ppDSBuffer)
	{
		Logging::Log() << __FUNCTION__ << " Error: bad buffer address!";
		return DSERR_INVALIDPARAM;
	}

	// Buffer variables
	DSBUFFERDESC dsbd = {};
	WAVEFORMATEX waveFormat = {};
	std::vector<char> AudioBuffer;

	// Parse WAV files
	HRESULT hr = ParseWavFile(filePath, dsbd, waveFormat, AudioBuffer);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to load WAV! file: " << filePath << " hr: " << (DSERR)hr;
		return hr;
	}

	// Create a sound buffer
	LPDIRECTSOUNDBUFFER pBuffer = nullptr;
	hr = ProxyInterface->CreateSoundBuffer(&dsbd, &pBuffer, nullptr);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create buffer! file: " << filePath << " hr: " << (DSERR)hr;
		return hr;
	}
	*ppDSBuffer = new m_IDirectSoundBuffer8((IDirectSoundBuffer8*)pBuffer);

	// Lock the buffer and copy the audio data
	LPVOID pData = nullptr;
	DWORD dataSize = dsbd.dwBufferBytes;
	hr = (*ppDSBuffer)->Lock(0, dataSize, &pData, &dataSize, nullptr, nullptr, DSBLOCK_ENTIREBUFFER);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to lock buffer! file: " << filePath << " hr: " << (DSERR)hr;
		return hr;
	}
	memcpy(pData, AudioBuffer.data(), dataSize);
	(*ppDSBuffer)->Unlock(pData, dataSize, nullptr, 0);

	return hr;
}

HRESULT ParseWavFile(const char* filePath, DSBUFFERDESC& dsbd, WAVEFORMATEX& waveFormat, std::vector<char>& AudioBuffer)
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

		// Settings the buffer desc
		dsbd = {};
		dsbd.dwSize = sizeof(DSBUFFERDESC);
		dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLPOSITIONNOTIFY;
		dsbd.dwBufferBytes = dataSize;
		dsbd.lpwfxFormat = &waveFormat;

		// Set hr to success
		hr = DS_OK;
	} while (false);

	// Close and exit
	file.close();
	return hr;
}

HRESULT PlayWavFile(const char* filePath, DWORD BifferID)
{
	if (!filePath)
	{
		Logging::Log() << __FUNCTION__ << " Error: bad file name!";
		return DSERR_INVALIDPARAM;
	}

	if (BifferID + 1 > MaxBuffers)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid buffer ID!";
		return DSERR_INVALIDPARAM;
	}

	if (!pCurrentDirectSound)
	{
		Logging::Log() << __FUNCTION__ << " Error: current DriectSound8 not setup!";
		return DSERR_DS8_REQUIRED;
	}

	EnterCriticalSection(&dscs);

	// Release all buffers that are not playing
	for (DWORD x = 0; x < MaxBuffers; x++)
	{
		if (pDirectSoundWavBuffer[x])
		{
			DWORD Status = 0;
			if (SUCCEEDED(pDirectSoundWavBuffer[x]->GetStatus(&Status)) && !(Status & DSBSTATUS_PLAYING))
			{
				ReleaseSoundBuffer(x);
			}
		}
	}

	// Release current buffer
	ReleaseSoundBuffer(BifferID);

	HRESULT hr = pCurrentDirectSound->CreateWAVSoundBuffer(filePath, &pDirectSoundWavBuffer[BifferID]);

	if (SUCCEEDED(hr))
	{
		// Get the volume level
		LONG SFXVolume = DSBVOLUME_MAX;
		BYTE SFXVolumeLevel = GetSFXVolume();
		if (SFXVolumeLevel < 16)
		{
			SFXVolume = VolumeArray[SFXVolumeLevel];
		}

		// Set the volume
		pDirectSoundWavBuffer[BifferID]->SetVolume(SFXVolume);

		// Play the loaded WAV file
		pDirectSoundWavBuffer[BifferID]->Play(0, 0, 0);

		Logging::Log() << __FUNCTION__ << " Playing sound: " << filePath;
	}

	LeaveCriticalSection(&dscs);

	return hr;
}

void ReleaseSoundBuffer(DWORD BifferID)
{
	if (pDirectSoundWavBuffer[BifferID])
	{
		UINT ref = pDirectSoundWavBuffer[BifferID]->Release();
		if (ref != 0)
		{
			Logging::Log() << __FUNCTION__ << " Error: there is still a reference to the sound buffer " << ref;
		}
		pDirectSoundWavBuffer[BifferID] = nullptr;
	}
}
