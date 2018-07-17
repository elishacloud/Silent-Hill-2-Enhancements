/**
* Copyright (C) 2018 Elisha Riedlinger
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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include "LoadModule.h"
#include "External\MemoryModule\MemoryModule.h"
#include "Common\Utils.h"
#include "Common\Logging.h"

// Memory modules
struct MMODULE
{
	HMEMORYMODULE handle;		// Module handle
	DWORD ResID;				// Resource ID
};
std::vector<MMODULE> HMModules;

// Unload resource memory modules
void UnloadResourceModules()
{
	for (MMODULE it : HMModules)
	{
		MemoryFreeLibrary(it.handle);
	}
}

// Load memory module from resource
void LoadModuleFromResource(HMODULE hModule, DWORD ResID, LPCWSTR lpName)
{
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(ResID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource(hLoadedResource);
			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource(hModule, hResource);
				if (dwResourceSize != 0)
				{
					Log() << "Loading the " << lpName << " module...";
					HMEMORYMODULE hMModule = MemoryLoadLibrary((const void*)pLockedResource, dwResourceSize);
					if (hMModule)
					{
						MMODULE MMItem = { hMModule , ResID };
						HMModules.push_back(MMItem);
					}
					else
					{
						Log() << __FUNCTION__ << " Error: " << lpName << " module could not be loaded!";
					}
				}
			}
		}
	}
}

// Load memory module from resource
void LoadModuleFromResourceToFile(HMODULE hModule, DWORD ResID, LPCWSTR lpName, LPCWSTR lpFilepath)
{
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(ResID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource(hLoadedResource);
			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource(hModule, hResource);
				if (dwResourceSize != 0)
				{
					Log() << "Loading the " << lpName << " module...";
					std::fstream fsModule;
					fsModule.open(lpFilepath, std::ios_base::out | std::ios_base::binary);
					if (fsModule.is_open())
					{
						fsModule.write((char*)pLockedResource, dwResourceSize);
						fsModule.close();
						HMODULE h_Module = LoadLibrary(lpFilepath);
						if (h_Module)
						{
							AddHandleToVector(h_Module);
						}
						else
						{
							Log() << __FUNCTION__ << " Error: " << lpName << " module could not be loaded!";
						}
					}
					else
					{
						Log() << __FUNCTION__ << " Error: " << lpName << " module could not be written!";
					}
				}
			}
		}
	}
}
