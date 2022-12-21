/**
* Copyright (C) 2022 Elisha Riedlinger
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
#include <Shldisp.h>
#include <shellapi.h>
#include <string>
#include <iostream>
#include <filesystem>
#include "Utils.h"
#include "Patches\Patches.h"
#include "Wrappers\d3d8\d3d8wrapper.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "Unicode.h"

#ifndef _DPI_AWARENESS_CONTEXTS_

#define _DPI_AWARENESS_CONTEXTS_

DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);

typedef enum DPI_AWARENESS {
	DPI_AWARENESS_INVALID = -1,
	DPI_AWARENESS_UNAWARE = 0,
	DPI_AWARENESS_SYSTEM_AWARE = 1,
	DPI_AWARENESS_PER_MONITOR_AWARE = 2
} DPI_AWARENESS;

#define DPI_AWARENESS_CONTEXT_UNAWARE               ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE          ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE     ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((DPI_AWARENESS_CONTEXT)-4)
#define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED     ((DPI_AWARENESS_CONTEXT)-5)

typedef enum DPI_HOSTING_BEHAVIOR {
	DPI_HOSTING_BEHAVIOR_INVALID = -1,
	DPI_HOSTING_BEHAVIOR_DEFAULT = 0,
	DPI_HOSTING_BEHAVIOR_MIXED = 1
} DPI_HOSTING_BEHAVIOR;

#endif

#define STAP_ALLOW_NONCLIENT (1UL << 0)

#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20

typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef HRESULT(WINAPI *SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS value);
typedef BOOL(WINAPI *SetProcessDPIAwareProc)();
typedef BOOL(WINAPI *SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT value);

std::vector<HMODULE> custom_dll;		// Used for custom dll's and asi plugins

// Search memory for byte array
void *GetAddressOfData(const void *data, size_t len, DWORD step)
{
	return GetAddressOfData(data, len, step, 0);
}

// Search memory for byte array
void *GetAddressOfData(const void *data, size_t len, DWORD step, DWORD start, DWORD distance)
{
	HANDLE hProcess = GetCurrentProcess();
	if (hProcess)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);

		MEMORY_BASIC_INFORMATION info;
		std::string chunk;
		BYTE* p = (BYTE*)start;
		while (p < si.lpMaximumApplicationAddress && (DWORD)p < start + distance)
		{
			if (VirtualQueryEx(hProcess, p, &info, sizeof(info)) == sizeof(info))
			{
				p = (BYTE*)info.BaseAddress;
				chunk.resize(info.RegionSize);
				SIZE_T bytesRead;
				if (ReadProcessMemory(hProcess, p, &chunk[0], info.RegionSize, &bytesRead))
				{
					for (size_t i = 0; i < (bytesRead - len); i += step)
					{
						if ((DWORD)p + i > start)
						{
							if (memcmp(data, &chunk[i], len) == 0)
							{
								return (BYTE*)p + i;
							}
						}
						if ((DWORD)p > start + distance)
						{
							return nullptr;
						}
					}
				}
				p += info.RegionSize;
			}
			else
			{
				return nullptr;
			}
		}
	}
	return nullptr;
}

// Checks the value of two data segments
bool CheckMemoryAddress(void *dataAddr, void *dataBytes, size_t dataSize, bool WriteLog)
{
	if (!dataAddr || !dataBytes || !dataSize)
	{
		return false;
	}

	// VirtualProtect first to make sure patch_address is readable
	DWORD dwPrevProtect;
	if (!VirtualProtect(dataAddr, dataSize, PAGE_READONLY, &dwPrevProtect))
	{
		Logging::Log() << __FUNCTION__ << " Error: could not read memory address";
		return false;
	}

	bool flag = (memcmp(dataAddr, dataBytes, dataSize) == 0);

	// Restore protection
	VirtualProtect(dataAddr, dataSize, dwPrevProtect, &dwPrevProtect);

	if (!flag && WriteLog)
	{
		Logging::Log() << __FUNCTION__ << " Error: memory address not found!";
	}

	// Return results
	return flag;
}

// Checks mulitple memory addresses
void *CheckMultiMemoryAddress(void *dataAddr10, void *dataAddr11, void *dataAddrDC, void *dataBytes, size_t dataSize)
{
	void *MemAddress = nullptr;
	// v1.0
	if (!MemAddress && (GameVersion == SH2V_10 || GameVersion == SH2V_UNKNOWN))
	{
		MemAddress = (CheckMemoryAddress(dataAddr10, dataBytes, dataSize)) ? dataAddr10 : nullptr;
	}
	// v1.1
	if (!MemAddress && (GameVersion == SH2V_11 || GameVersion == SH2V_UNKNOWN))
	{
		MemAddress = (CheckMemoryAddress(dataAddr11, dataBytes, dataSize)) ? dataAddr11 : nullptr;
	}
	// vDC
	if (!MemAddress && (GameVersion == SH2V_DC || GameVersion == SH2V_UNKNOWN))
	{
		MemAddress = (CheckMemoryAddress(dataAddrDC, dataBytes, dataSize)) ? dataAddrDC : nullptr;
	}
	// Return address
	return MemAddress;
}

// Search for memory addresses
DWORD SearchAndGetAddresses(DWORD dataAddr10, DWORD dataAddr11, DWORD dataAddrDC, const BYTE *dataBytes, size_t dataSize, int ByteDelta)
{
	// Get address
	DWORD MemoryAddr = (DWORD)CheckMultiMemoryAddress((void*)dataAddr10, (void*)dataAddr11, (void*)dataAddrDC, (void*)dataBytes, dataSize);

	// Search for address
	if (!MemoryAddr)
	{
		DWORD SearchAddr = (GameVersion == SH2V_10) ? dataAddr10 : (GameVersion == SH2V_11) ? dataAddr11 : (GameVersion == SH2V_DC) ? dataAddrDC : dataAddr10;
		MemoryAddr = (DWORD)GetAddressOfData(dataBytes, dataSize, 1, SearchAddr - 0x800, 2600);
		Logging::Log() << __FUNCTION__ << " searching for memory address! Found = " << (void*)MemoryAddr;
	}

	// Checking address pointer
	if (!MemoryAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return NULL;
	}
	MemoryAddr = MemoryAddr + ByteDelta;

	// Return address found
	return MemoryAddr;
}

// Search for memory addresses
DWORD ReadSearchedAddresses(DWORD dataAddr10, DWORD dataAddr11, DWORD dataAddrDC, const BYTE *dataBytes, size_t dataSize, int ByteDelta)
{
	// Search for address
	DWORD MemoryAddr = SearchAndGetAddresses(dataAddr10, dataAddr11, dataAddrDC, dataBytes, dataSize, ByteDelta);

	// If address exists then read memory and return address
	if (MemoryAddr)
	{
		DWORD Address;
		memcpy(&Address, (void*)MemoryAddr, sizeof(DWORD));
		return Address;
	}

	// Return NULL
	return NULL;
}

// Search and log address
void SearchAndLogAddress(DWORD FindAddress)
{
	void *Address = (void*)0x00410000;
	for (int x = -3; x < 4; x++)
	{
		do {
			DWORD SearchAddress = FindAddress + x;
			Address = GetAddressOfData(&SearchAddress, sizeof(DWORD), 1, (DWORD)Address, 0x005F0000 - (DWORD)Address);
			Logging::Log() << "Address found: " << Address;
		} while (Address);
	}
}

// Update memory
bool UpdateMemoryAddress(void *dataAddr, const void *dataBytes, size_t dataSize)
{
	if (!dataAddr || !dataBytes || !dataSize)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid memory data";
		return false;
	}

	// VirtualProtect first to make sure patch_address is readable
	DWORD dwPrevProtect;
	if (!VirtualProtect(dataAddr, dataSize, PAGE_READWRITE, &dwPrevProtect))
	{
		Logging::Log() << __FUNCTION__ << " Error: could not write to memory address";
		return false;
	}

	// Update memory
	memcpy(dataAddr, dataBytes, dataSize);

	// Restore protection
	VirtualProtect(dataAddr, dataSize, dwPrevProtect, &dwPrevProtect);

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), dataAddr, dataSize);

	// Return
	return true;
}

// Write a address to memory
bool WriteAddresstoMemory(BYTE *dataAddr, const void *JMPAddr, DWORD count, BYTE command)
{
	if (!dataAddr || !JMPAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid memory data";
		return false;
	}

	if (count < 5)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid count";
		return false;
	}

	// VirtualProtect first to make sure patch_address is readable
	DWORD dwPrevProtect;
	if (!VirtualProtect(dataAddr, count, PAGE_READWRITE, &dwPrevProtect))
	{
		Logging::Log() << __FUNCTION__ << " Error: could not read memory address";
		return false; // access denied
	}

	// command (4-byte relative)
	*dataAddr = command;
	// relative jmp address
	*((DWORD*)(dataAddr + 1)) = (DWORD)JMPAddr - (DWORD)dataAddr - 5;

	for (DWORD x = 5; x < count; x++)
	{
		*((BYTE*)(dataAddr + x)) = 0x90;
	}

	// Restore protection
	VirtualProtect(dataAddr, count, dwPrevProtect, &dwPrevProtect);

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), dataAddr, count);

	// Return
	return true;
}

// Write a call to memory
bool WriteCalltoMemory(BYTE *dataAddr, const void *JMPAddr, DWORD count)
{
	// 0xE8 call (4-byte relative)
	return WriteAddresstoMemory(dataAddr, JMPAddr, count, 0xE8);
}

// Write a jmp to memory
bool WriteJMPtoMemory(BYTE *dataAddr, const void *JMPAddr, DWORD count)
{
	// 0xE9 jmp (4-byte relative)
	return WriteAddresstoMemory(dataAddr, JMPAddr, count, 0xE9);
}

// Replace memory
DWORD ReplaceMemoryBytes(void *dataSrc, void *dataDest, size_t size, DWORD start, DWORD distance, DWORD count)
{
	DWORD counter = 0;
	DWORD StartAddr = start;
	DWORD EndAddr = start + distance;

	// Update memory
	while (StartAddr < EndAddr)
	{
		// Get next address
		void *NextAddr = GetAddressOfData(dataSrc, size, 1, start, EndAddr - StartAddr);
		if (!NextAddr)
		{
			return counter;
		}
		StartAddr = (DWORD)NextAddr + size;

		// Write to memory
		UpdateMemoryAddress(NextAddr, dataDest, size);
		counter++;
		if (count && count == counter)
		{
			return counter;
		}
	}
	return counter;
}

bool GetCoreCount(DWORD& pBits, DWORD& sBits)
{
	pBits = 0;
	sBits = 0;

	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	if (GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
	{
		for (UINT x = 0; x < sizeof(DWORD_PTR); x++)
		{
			if (ProcessAffinityMask & 0x00000001)
			{
				pBits++;
			}
			ProcessAffinityMask >>= 1;
		}
		for (UINT x = 0; x < sizeof(DWORD_PTR); x++)
		{
			if (SystemAffinityMask & 0x00000001)
			{
				sBits++;
			}
			SystemAffinityMask >>= 1;
		}
		if (pBits && sBits)
		{
			return true;
		}
	}
	return false;
}

// Log process affinity
void LogAffinity()
{
	DWORD pBits, sBits;
	if (GetCoreCount(pBits, sBits))
	{
		Logging::Log() << __FUNCTION__ << " System has '" << sBits << "' cores and Silent Hill 2 is using '" << pBits << "' cores.";
	}
}

// Get processor mask
DWORD_PTR GetProcessMask()
{
	static DWORD_PTR nMask = 0;
	if (nMask)
	{
		return nMask;
	}

	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	if (GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
	{
		DWORD_PTR AffinityLow = 1;
		while (AffinityLow && (AffinityLow & SystemAffinityMask) == 0)
		{
			AffinityLow <<= 1;
		}
		if (AffinityLow)
		{
			DWORD_PTR Base = (SingleCoreAffinityLegacy) ? SingleCoreAffinityLegacy : ((AffinityLow << 2) & SystemAffinityMask) ? 3 : 2;
			nMask = ((AffinityLow << (Base - 1)) & SystemAffinityMask) ? (AffinityLow << (Base - 1)) : AffinityLow;
		}
	}

	Logging::Log() << __FUNCTION__ << " Using CPU mask: " << Logging::hex(nMask);
	return nMask;
}

// Set Single Core Affinity
void SetSingleCoreAffinity()
{
	Logging::Log() << "Setting SingleCoreAffinity...";
	SetProcessAffinityMask(GetCurrentProcess(), GetProcessMask());
}

// Set Multi Core Affinity
void SetMultiCoreAffinity()
{
	DWORD pBits, sBits;
	if (GetCoreCount(pBits, sBits) && pBits != sBits)
	{
		DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
		if (GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask))
		{
			Logging::Log() << __FUNCTION__ << " Setting Multi CPU mask: " << Logging::hex(SystemAffinityMask);
			SetProcessAffinityMask(GetCurrentProcess(), SystemAffinityMask);
		}
	}
}

// Sets application DPI aware which disables DPI virtulization/High DPI scaling for this process
void SetDPIAware()
{
	Logging::Log() << "Disabling High DPI Scaling...";

	BOOL setDpiAware = FALSE;
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");

	if (!setDpiAware)
	{
		SetProcessDpiAwarenessContextProc setProcessDpiAwarenessContext = (SetProcessDpiAwarenessContextProc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");

		if (setProcessDpiAwarenessContext)
		{
			setDpiAware |= setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
		}
	}
	if (!setDpiAware)
	{
		HMODULE hShcore = LoadLibrary(L"shcore.dll");
		SetProcessDpiAwarenessProc setProcessDpiAwareness = (SetProcessDpiAwarenessProc)GetProcAddress(hShcore, "SetProcessDpiAwareness");

		if (setProcessDpiAwareness)
		{
			setDpiAware |= SUCCEEDED(setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE));
		}
	}
	if (!setDpiAware)
	{
		SetProcessDPIAwareProc setProcessDPIAware = (SetProcessDPIAwareProc)GetProcAddress(hUser32, "SetProcessDPIAware");

		if (setProcessDPIAware)
		{
			setDpiAware |= setProcessDPIAware();
		}
	}

	if (!setDpiAware)
	{
		Logging::Log() << "Failed to disable High DPI Scaling!";
	}
}

// Allow for dark mode theme
HMODULE SetAppTheme()
{
	LOG_ONCE("Setting GUI theme...");

	static HMODULE hUxtheme = LoadLibrary(L"uxtheme.dll");

	using SetThemeAppPropertiesProc = void (WINAPI *)(_In_ DWORD dwFlags);
	static SetThemeAppPropertiesProc SetThemeAppProperties = (SetThemeAppPropertiesProc)GetProcAddress(hUxtheme, "SetThemeAppProperties");

	if (SetThemeAppProperties)
	{
		LOG_ONCE("Setting theme properties...");
		SetThemeAppProperties(STAP_ALLOW_NONCLIENT);
	}

	return hUxtheme;
}

// Allow for dark mode theme
void SetWindowTheme(HWND hWnd)
{
	HMODULE hUxtheme = SetAppTheme();

	using SetWindowThemeProc = void (WINAPI *)(_In_ HWND hwnd, _In_opt_ LPCWSTR pszSubAppName, _In_opt_ LPCWSTR pszSubIdList);
	static SetWindowThemeProc SetWindowTheme = (SetWindowThemeProc)GetProcAddress(hUxtheme, "SetWindowTheme");

	if (SetWindowTheme && hWnd)
	{
		LOG_ONCE("Setting Window theme...");
		SetWindowTheme(hWnd, L"Explorer", NULL);
	}

	static HMODULE hDwmapi = LoadLibrary(L"dwmapi.dll");

	using DwmSetWindowAttributeProc = HRESULT(WINAPI *)(HWND hwnd, DWORD dwAttribute, _In_reads_bytes_(cbAttribute) LPCVOID pvAttribute, DWORD cbAttribute);
	static DwmSetWindowAttributeProc DwmSetWindowAttribute = (DwmSetWindowAttributeProc)GetProcAddress(hDwmapi, "DwmSetWindowAttribute");

	if (DwmSetWindowAttribute && hWnd)
	{
		LOG_ONCE("Setting Window Attributes...");
		BOOL fBool = (GetAppsLightMode() == 0);
		DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &fBool, sizeof(BOOL));
	}
}

// Add HMODULE to vector
void AddHandleToVector(HMODULE dll)
{
	if (dll)
	{
		custom_dll.push_back(dll);
	}
}

// Unload standard modules
void UnloadAllModules()
{
	for (HMODULE it : custom_dll)
	{
		if (it)
		{
			FreeLibrary(it);
		}
	}
}

DWORD ConvertFloat(float num)
{
	return *((DWORD*)&num);
}

void GetFileDate(std::filesystem::file_time_type filetime, char *strOutput, size_t size)
{
	if (!strOutput)
	{
		return;
	}

	time_t rawtime = std::chrono::duration_cast<std::chrono::seconds>(filetime.time_since_epoch()).count() - 11644473600;
	struct tm ts;

	// Format time, "mm/dd/yyyy hh:mm AM"
	if (localtime_s(&ts, &rawtime) == 0)
	{
		strftime(strOutput, size, "%m/%d/%Y  %I:%M %p", &ts);
	}
}

void GetFileSize(uintmax_t fsize, char *strOutput, size_t size)
{
	if (!strOutput)
	{
		return;
	}

	// Get file size
	std::string strSize;
	for (int x = 1; x < 18; x++)
	{
		if (fsize && x % 4 == 0)
		{
			strSize.insert(0, ",");
		}
		else if (!fsize)
		{
			strSize.insert(0, " ");
		}
		else
		{
			char num[2] = { '\0' };
			num[0] = (fsize % 10) + 48;
			strSize.insert(0, num);
			fsize /= 10;
		}
	}
	strcpy_s(strOutput, size, strSize.c_str());
}

HMEMORYMODULE LoadResourceToMemory(DWORD ResID)
{
	HRSRC hResource = FindResource(m_hModule, MAKEINTRESOURCE(ResID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource(m_hModule, hResource);
		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource(hLoadedResource);
			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource(m_hModule, hResource);
				if (dwResourceSize != 0)
				{
					return MemoryLoadLibrary(pLockedResource, dwResourceSize);
				}
			}
		}
	}
	return nullptr;
}

template <typename T>
void ExtractFileFromResourceT(DWORD ResID, T *lpFilepath)
{
	do {
		HRSRC hResource = FindResource(m_hModule, MAKEINTRESOURCE(ResID), RT_RCDATA);
		if (hResource)
		{
			HGLOBAL hLoadedResource = LoadResource(m_hModule, hResource);
			if (hLoadedResource)
			{
				LPVOID pLockedResource = LockResource(hLoadedResource);
				if (pLockedResource)
				{
					DWORD dwResourceSize = SizeofResource(m_hModule, hResource);
					if (dwResourceSize != 0)
					{
						Logging::Log() << "Extracting the " << lpFilepath << " file...";

						std::fstream fsModule;
						fsModule.open(lpFilepath, std::ios_base::out | std::ios_base::binary);
						if (fsModule.is_open())
						{
							// Write file to disk
							fsModule.write((char*)pLockedResource, dwResourceSize);
							fsModule.close();
							// Return
							return;
						}
					}
				}
			}
		}
	} while (false);

	Logging::Log() << __FUNCTION__ << " Error: could not extract the " << lpFilepath << " file!";
}

void ExtractFileFromResource(DWORD ResID, char* lpFilepath)
{
	return ExtractFileFromResourceT<char>(ResID, lpFilepath);
}
void ExtractFileFromResource(DWORD ResID, wchar_t* lpFilepath)
{
	return ExtractFileFromResourceT<wchar_t>(ResID, lpFilepath);
}

HRESULT UnZipFile(BSTR sourceZip, BSTR destFolder)
{
	HRESULT hr = E_FAIL;
	IShellDispatch *pISD = nullptr;
	Folder *pToFolder = nullptr;
	Folder *pFromFolder = nullptr;

	do
	{
		if (FAILED(CoInitialize(NULL)))
		{
			Logging::Log() << __FUNCTION__ " Failed to CoInitialize!";
			break;
		}

		if (FAILED(CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD)) || !pISD)
		{
			Logging::Log() << __FUNCTION__ " Failed to CoCreateInstance!";
			break;
		}

		VARIANT vFile;
		VariantInit(&vFile);
		vFile.vt = VT_BSTR;
		vFile.bstrVal = sourceZip;

		if (FAILED(pISD->NameSpace(vFile, &pFromFolder)))
		{
			Logging::Log() << __FUNCTION__ " Failed to get source NameSpace! " << sourceZip;
			break;
		}

		VARIANT vDir;
		VariantInit(&vDir);
		vDir.vt = VT_BSTR;
		vDir.bstrVal = destFolder;
		if (!PathFileExists(destFolder))
		{
			CreateDirectory(destFolder, nullptr);
		}

		// Destination is our zip file
		if (FAILED(pISD->NameSpace(vDir, &pToFolder)) || !pToFolder)
		{
			Logging::Log() << __FUNCTION__ " Failed to get destination NameSpace! " << destFolder;
			break;
		}

		FolderItems *fi = nullptr;
		if (FAILED(pFromFolder->Items(&fi)))
		{
			Logging::Log() << __FUNCTION__ " Failed to get file list from zip file!";
			break;
		}

		VARIANT vOpt;
		VariantInit(&vOpt);
		vOpt.vt = VT_I4;
		vOpt.lVal = FOF_NO_UI;

		VARIANT newV;
		VariantInit(&newV);
		newV.vt = VT_DISPATCH;
		newV.pdispVal = fi;

		if (FAILED(pToFolder->CopyHere(newV, vOpt)))
		{
			Logging::Log() << __FUNCTION__ " Failed to extract files out of zip file!";
			break;
		}

		hr = S_OK;
	} while (false);

	if (pToFolder)
	{
		pToFolder->Release();
	}

	if (pFromFolder)
	{
		pFromFolder->Release();
	}

	if (pISD)
	{
		pISD->Release();
	}

	CoUninitialize();

	return hr;
}

DWORD WINAPI GetModuleFileNameT(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	DWORD val = GetModuleFileNameA(hModule, lpFilename, nSize);
	if (hModule == m_hModule)
	{
		// Get SH2 path
		char sh2path[MAX_PATH] = {};
		bool ret = (GetModuleFileNameA(nullptr, sh2path, MAX_PATH) != 0);
		char* pdest = strrchr(sh2path, '\\');
		if (ret && pdest)
		{
			*(pdest + 1) = '\0';
		}

		// Get module path
		char modpath[MAX_PATH] = {};
		ret = (strcpy_s(modpath, MAX_PATH, lpFilename) == 0);
		pdest = strrchr(modpath, '\\');
		if (ret && pdest)
		{
			*(pdest + 1) = '\0';
		}

		// Check if should return static d3d8.dll path
		if (!PathFileExistsA(lpFilename) || _stricmp(modpath, sh2path) != 0)
		{
			ret = (strcat_s(sh2path, MAX_PATH, "d3d8.dll") == 0);
			pdest = strrchr(sh2path, '\\');
			if (ret && pdest)
			{
				strcpy_s(lpFilename, nSize, sh2path);
				return strlen(lpFilename);
			}
		}
	}
	return val;
}

DWORD WINAPI GetModuleFileNameT(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	DWORD val = GetModuleFileNameW(hModule, lpFilename, nSize);
	if (hModule == m_hModule)
	{
		// Get SH2 path
		wchar_t sh2path[MAX_PATH] = {};
		bool ret = (GetModuleFileNameW(nullptr, sh2path, MAX_PATH) != 0);
		wchar_t* pdest = wcsrchr(sh2path, '\\');
		if (ret && pdest)
		{
			*(pdest + 1) = '\0';
		}

		// Get module path
		wchar_t modpath[MAX_PATH] = {};
		ret = (wcscpy_s(modpath, MAX_PATH, lpFilename) == 0);
		pdest = wcsrchr(modpath, '\\');
		if (ret && pdest)
		{
			*(pdest + 1) = '\0';
		}

		// Check if should return static d3d8.dll path
		if (!PathFileExistsW(lpFilename) || _wcsicmp(modpath, sh2path) != 0)
		{
			ret = (wcscat_s(sh2path, MAX_PATH, L"d3d8.dll") == 0);
			pdest = wcsrchr(sh2path, '\\');
			if (ret && pdest)
			{
				wcscpy_s(lpFilename, nSize, sh2path);
				return wcslen(lpFilename);
			}
		}
	}
	return val;
}

bool GetModulePath(char *path, rsize_t size)
{
	static char modpath[MAX_PATH] = {};

	static bool ret = (GetModuleFileNameT(m_hModule, modpath, MAX_PATH) != 0);

	return (ret && strcpy_s(path, size, modpath) == 0);
}

bool GetModulePath(wchar_t *path, rsize_t size)
{
	static wchar_t modpath[MAX_PATH] = {};

	static bool ret = (GetModuleFileNameT(m_hModule, modpath, MAX_PATH) != 0);

	return (ret && wcscpy_s(path, size, modpath) == 0);
}

bool GetSH2FolderPath(char *path, rsize_t size)
{
	static char sh2path[MAX_PATH] = {};

	static bool ret = (GetModuleFileName(nullptr, sh2path, MAX_PATH) != 0);

	return (ret && strcpy_s(path, size, sh2path) == 0);
}

bool GetSH2FolderPath(wchar_t *path, rsize_t size)
{
	static wchar_t sh2path[MAX_PATH] = {};

	static bool ret = (GetModuleFileName(nullptr, sh2path, MAX_PATH) != 0);

	return (ret && wcscpy_s(path, size, sh2path) == 0);
}

template <typename T>
bool GetConfigNameLowerT(T* ConfigName, rsize_t size)
{
	bool ret = GetModulePath(ConfigName, size);
	T* pdest = strrchr(ConfigName, '\\');
	if (ret && pdest)
	{
		T name[MAX_PATH] = {};
		strcpy_s(name, MAX_PATH, pdest + 1);

		pdest = strrchr(name, '.');
		if (pdest)
		{
			*pdest = '\0';
		}

		pdest = strrchr(ConfigName, '\\');
		if (pdest)
		{
			*(pdest + 1) = '\0';
		}
		strcat_s(ConfigName, size, TransformLower(name).c_str());

		return true;
	}
	return false;
}

bool GetConfigName(char* ConfigName, rsize_t size, char* ext)
{
	static char modpath[MAX_PATH] = {};

	static bool ret = GetConfigNameLowerT<char>(modpath, size);

	bool rt = (ret && strcpy_s(ConfigName, size, modpath) == 0);

	return (ret && rt && strcat_s(ConfigName, size, ext) == 0);
}

bool GetConfigName(wchar_t* ConfigName, rsize_t size, wchar_t* ext)
{
	static wchar_t modpath[MAX_PATH] = {};

	static bool ret = GetConfigNameLowerT<wchar_t>(modpath, size);

	bool rt = (ret && wcscpy_s(ConfigName, size, modpath) == 0);

	return (ret && rt && wcscat_s(ConfigName, size, ext) == 0);
}

bool CheckPathNameMatch(LPCSTR lpFileName1, LPCSTR lpFileName2)
{
	char *pfile1 = nullptr;
	for (int x = 0; x < MAX_PATH; x++)
	{
		if (lpFileName1[x] == '\0')
		{
			break;
		}
		else if (lpFileName1[x] == '\\' || lpFileName1[x] == '/')
		{
			pfile1 = (char*)(lpFileName1 + x + 1);
		}
	}
	char *pfile2 = nullptr;
	for (int x = 0; x < MAX_PATH; x++)
	{
		if (lpFileName2[x] == '\0')
		{
			break;
		}
		else if (lpFileName2[x] == '\\' || lpFileName2[x] == '/')
		{
			pfile2 = (char*)(lpFileName2 + x + 1);
		}
	}
	if (pfile1 && pfile2 && strcmp(pfile1, pfile2) == 0)
	{
		return true;
	}
	return false;
}

void CopyReplaceSlash(char* DestStr, size_t Size, LPCSTR SrcStr)
{
	for (UINT x = 0; x < Size; x++)
	{
		if (SrcStr[x] == '/')
		{
			DestStr[x] = '\\';
		}
		else if (SrcStr[x] == '\0')
		{
			DestStr[x] = SrcStr[x];
			break;
		}
		else
		{
			DestStr[x] = SrcStr[x];
		}
	}
}

BOOL GetAppsLightMode()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Failed to open registry key!");
		return NULL;
	}

	BOOL bFlag = NULL;
	DWORD Size = sizeof(BOOL);
	if (RegQueryValueEx(hKey, L"AppsUseLightTheme", 0, 0, (BYTE*)&bFlag, &Size) != ERROR_SUCCESS)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Failed to read registry value!");
	}

	RegCloseKey(hKey);

	return bFlag;
}

HMONITOR GetMonitorHandle()
{
	return MonitorFromWindow(IsWindow(DeviceWindow) ? DeviceWindow : GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
}

void GetDesktopRes(LONG &screenWidth, LONG &screenHeight)
{
	MONITORINFO info = {};
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(GetMonitorHandle(), &info);
	screenWidth = info.rcMonitor.right - info.rcMonitor.left;
	screenHeight = info.rcMonitor.bottom - info.rcMonitor.top;
}

void GetDesktopRect(RECT &screenRect)
{
	MONITORINFO info = {};
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(GetMonitorHandle(), &info);
	screenRect.left = info.rcMonitor.left;
	screenRect.top = info.rcMonitor.top;
	screenRect.right = info.rcMonitor.right;
	screenRect.bottom = info.rcMonitor.bottom;
}

bool ReadRegistryStruct(const std::wstring& lpzSection, const std::wstring& lpzKey, const LPVOID& lpStruct, UINT uSizeStruct)
{
	HKEY hKey;
	std::wstring SubKey(L"SOFTWARE\\" + lpzSection);

	HRESULT hr = RegOpenKey(HKEY_CURRENT_USER, SubKey.c_str(), &hKey);

	if (FAILED(hr))
	{
		return false;
	}

	DWORD DataSize = 0;
	hr = RegQueryValueEx(hKey, lpzKey.c_str(), 0, 0, 0, &DataSize);
	if (DataSize > uSizeStruct)
	{
		return false;
	}

	hr = RegQueryValueEx(hKey, lpzKey.c_str(), 0, 0, (BYTE*)lpStruct, &DataSize);

	if (FAILED(hr) || !DataSize)
	{
		return false;
	}

	RegCloseKey(hKey);

	return true;
}

bool WriteRegistryStruct(const std::wstring& lpzSection, const std::wstring& lpzKey, DWORD dwType, const LPVOID lpStruct, UINT uSizeStruct)
{
	HKEY hKey;
	std::wstring SubKey(L"SOFTWARE\\" + lpzSection);

	HRESULT hr = RegCreateKey(HKEY_CURRENT_USER, SubKey.c_str(), &hKey);

	if (FAILED(hr))
	{
		return false;
	}

	hr = RegSetValueEx(hKey, lpzKey.c_str(), 0, dwType, (BYTE*)lpStruct, uSizeStruct);

	if (FAILED(hr))
	{
		return false;
	}

	RegCloseKey(hKey);

	return true;
}

struct CFGDATA
{
	DWORD Width = 0;
	DWORD Height = 0;
};

HRESULT GetResolutionFromConfig(DWORD &Width, DWORD &Height)
{
	wchar_t ConfigName[MAX_PATH] = {};
	if (!GetConfigName(ConfigName, MAX_PATH, L".cfg") || !PathFileExists(ConfigName))
	{
		return E_FAIL;
	}

	HANDLE hFile;
	const DWORD BytesToRead = sizeof(CFGDATA);
	DWORD dwBytesRead;

	CFGDATA ConfigData;
	hFile = CreateFile(ConfigName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to open file: '" << ConfigName << "'");
		return E_FAIL;
	}

	BOOL hRet = ReadFile(hFile, (void*)&ConfigData, BytesToRead, &dwBytesRead, nullptr);
	if (dwBytesRead != BytesToRead || hRet == FALSE)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	if (ConfigData.Width && ConfigData.Height)
	{
		Width = ConfigData.Width;
		Height = ConfigData.Height;

		CloseHandle(hFile);
		return S_OK;
	}

	CloseHandle(hFile);
	return E_FAIL;
}

void MigrateRegistry()
{
	DWORD Width = 0, Height = 0;

	// Check if registry key exists
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Konami\\Silent Hill 2\\sh2e", 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
	{
		return;
	}

	// Get resolution from registry
	DWORD Size = sizeof(DWORD);
	RegQueryValueEx(hKey, L"Width", 0, 0, (BYTE*)&Width, &Size);
	Size = sizeof(DWORD);
	RegQueryValueEx(hKey, L"Height", 0, 0, (BYTE*)&Height, &Size);

	// Remove resolution from registry
	RegDeleteValueW(hKey, L"Width");
	RegDeleteValueW(hKey, L"Height");

	RegCloseKey(hKey);

	if (Width && Height)
	{
		DWORD TestWidth = 0, TestHeight = 0;

		if (FAILED(GetResolutionFromConfig(TestWidth, TestHeight)) || !TestWidth || !TestHeight)
		{
			if (FAILED(SaveResolution(Width, Height)))
			{
				return;
			}
		}
	}

	return;
}

HRESULT GetSavedResolution(DWORD &Width, DWORD &Height)
{
	// Migrate old registry keys to config file 
	MigrateRegistry();

	// Check for config file
	return GetResolutionFromConfig(Width, Height);
}

HRESULT SaveResolution(DWORD Width, DWORD Height)
{
	wchar_t ConfigName[MAX_PATH] = {};
	if (!GetConfigName(ConfigName, MAX_PATH, L".cfg"))
	{
		return E_FAIL;
	}

	HANDLE hFile;
	const DWORD BytesToRead = sizeof(CFGDATA);
	DWORD dwBytesWritten;

	CFGDATA ConfigData = { Width, Height };
	hFile = CreateFile(ConfigName, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to open file: '" << ConfigName << "'");
		return E_FAIL;
	}

	BOOL bRet = WriteFile(hFile, (void*)&ConfigData, BytesToRead, &dwBytesWritten, nullptr);

	CloseHandle(hFile);

	if (bRet)
	{
		return S_OK;
	}

	return E_FAIL;
}

void LogDirectory()
{
	wchar_t path[MAX_PATH];
	bool ret = GetSH2FolderPath(path, MAX_PATH);
	wchar_t* pdest = wcsrchr(path, '\\');
	if (ret && pdest)
	{
		*pdest = '\0';
	}

	Logging::Log() << "|- ";
	Logging::Log() << "|-  Directory of " << path;
	Logging::Log() << "|- ";

	// Directories
	for (const auto & entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			// Get file date
			char filetime[80] = { '\0' };
			GetFileDate(entry.last_write_time(), filetime, sizeof(filetime));

			// Get folder name
			std::wstring filename(entry.path().filename());

			Logging::Log() << "|- " << filetime << "    <DIR>          " << filename;
		}
	}
	// Files
	for (const auto & entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_directory())
		{
			// Get file date
			char filetime[80] = { '\0' };
			GetFileDate(entry.last_write_time(), filetime, sizeof(filetime));

			// Get file size
			char filesize[80] = { '\0' };
			GetFileSize(entry.file_size(), filesize, sizeof(filesize));

			// Get file name
			std::wstring filename(entry.path().filename());

			Logging::Log() << "|- " << filetime << " " << filesize << " " << filename;
		}
	}

	Logging::Log() << "|--------------------------------";
}

void LogAllModules()
{
	HMODULE hMods[1024];
	HANDLE hProcess = GetCurrentProcess();
	DWORD cbNeeded;

	Logging::Log() << "|-------- MODULES LOADED --------";

	// Get a list of all the modules in this process
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			wchar_t szModName[MAX_PATH];

			// Get the full path to the module's file
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t)))
			{
				// Print the module name and handle value
				Logging::Log() << "|- " << szModName;
			}
		}
	}

	// Release the handle to the process
	CloseHandle(hProcess);

	Logging::Log() << "|--------------------------------";
}

void RunDelayedOneTimeItems()
{
	RUNCODEONCE(

		// Set Multi Core Affinity
		if (EnableCriWareReimplementation && !SingleCoreAffinityLegacy)
		{
			SetMultiCoreAffinity();
		}

		// Log number of cores used
		LogAffinity();

		// Log all attached modules
		LogAllModules();

	);
}
