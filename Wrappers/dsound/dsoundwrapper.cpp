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
#include "Wrappers\wrapper.h"
#include "Common\Utils.h"

AddressLookupTableDsound<void> ProxyAddressLookupTableDsound = AddressLookupTableDsound<void>();

DirectSoundCreate8Proc m_pDirectSoundCreate8 = nullptr;

// Hook dsound API
void HookDirectSoundCreate8()
{
	// Get DirectSoundCreate8 address
	constexpr BYTE SearchBytes[]{ 0x53, 0x33, 0xDB, 0x3B, 0xC3, 0x56, 0x57, 0x0F, 0x85 };
	DWORD Address = SearchAndGetAddresses(0x00514F15, 0x00515245, 0x00514B65, SearchBytes, sizeof(SearchBytes), 0x30, __FUNCTION__);

	// Checking address pointer
	if (!Address)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get 'DirectSoundCreate8' function address
	m_pDirectSoundCreate8 = (DirectSoundCreate8Proc)(Address + 5 + *(DWORD*)(Address + 1));

	// Write to memory
	WriteCalltoMemory((BYTE*)Address, *DirectSoundCreate8Wrapper, 5);
}

// Dsound DirectSoundCreate8 API wrapper
HRESULT WINAPI DirectSoundCreate8Wrapper(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
	LOG_LIMIT(3, "Redirecting 'DirectSoundCreate8' ...");

	// Try using dsoal with OpenAL-Soft
	HRESULT hr = DSOAL_DirectSoundCreate8(pcGuidDevice, ppDS8, pUnkOuter);

	// Failover to local dsound.dll
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " DSOAL 'DirectSoundCreate8' Failed! Error: " << (DSERR)hr << " Atepmting to use local dsound.dll.";
		hr = m_pDirectSoundCreate8(pcGuidDevice, ppDS8, pUnkOuter);
	}

	// Failover to System32 dsound.dll
	if (FAILED(hr))
	{
		// Get System32 path
		wchar_t systemFolderPath[MAX_PATH];
		if (GetSystemDirectoryW(systemFolderPath, MAX_PATH))
		{
			std::wstring dllPath = std::wstring(systemFolderPath) + L"\\dsound.dll";

			// Load dsound.dll from System32
			HMODULE dsoundDLL = LoadLibrary(dllPath.c_str());
			if (dsoundDLL)
			{
				DirectSoundCreate8Proc pDirectSoundCreate8 = (DirectSoundCreate8Proc)GetProcAddress(dsoundDLL, "DirectSoundCreate8");

				// Call DirectSoundCreate8 from System32
				if (pDirectSoundCreate8)
				{
					Logging::Log() << __FUNCTION__ << " local 'DirectSoundCreate8' Failed! Error: " << (DSERR)hr << " Atepmting to use system dsound.dll.";
					hr = pDirectSoundCreate8(pcGuidDevice, ppDS8, pUnkOuter);
				}
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppDS8 = new m_IDirectSound8(*ppDS8);
	}
	else
	{
		Logging::Log() << "'DirectSoundCreate8' Failed! Error: " << (DSERR)hr;
	}

	RunDelayedOneTimeItems();

	return hr;
}
