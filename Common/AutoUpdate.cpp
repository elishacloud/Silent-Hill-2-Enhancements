/**
* Copyright (C) 2020 Elisha Riedlinger
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
#include <urlmon.h>
#include <sstream>
#include <iostream>
#include <string>
#include <regex>
#include "Common\LoadModules.h"
#include "Logging\Logging.h"
#include "Resources\BuildNo.rc"

typedef HRESULT(WINAPI *URLOpenBlockingStreamProc)(LPUNKNOWN pCaller, LPCSTR szURL, LPSTREAM *ppStream, _Reserved_ DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB);
typedef HRESULT(WINAPI *URLDownloadToFileProc)(LPUNKNOWN pCaller, LPCWSTR szURL, LPCWSTR szFileName, _Reserved_ DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB);

extern HMODULE m_hModule;

namespace {
	const char removechars[] = " {}:\"\t\n\r";
	inline void trim(std::string &str, const char chars[] = removechars)
	{
		str.erase(0, str.find_first_not_of(chars));
		str.erase(str.find_last_not_of(chars) + 1);
	}
	inline std::string trim(const std::string &str, const char chars[] = removechars)
	{
		std::string res(str);
		trim(res, chars);
		return res;
	}
}

HMODULE GetUrlDll()
{
	static HMODULE urlmondll = LoadLibraryA("urlmon.dll");
	return urlmondll;
}

HRESULT URLOpenBlockingStreamHandler(LPUNKNOWN pCaller, LPCSTR szURL, LPSTREAM *ppStream, _Reserved_ DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB)
{
	HMODULE urlmondll = GetUrlDll();
	if (!urlmondll)
	{
		Logging::Log() << __FUNCTION__ << " Error: Cannnot open urlmon.dll!";
		return E_FAIL;
	}

	static URLOpenBlockingStreamProc pURLOpenBlockingStream = (URLOpenBlockingStreamProc)GetProcAddress(urlmondll, "URLOpenBlockingStreamA");
	if (!pURLOpenBlockingStream)
	{
		Logging::Log() << __FUNCTION__ << " Error: Cannnot find 'URLOpenBlockingStreamA' in urlmon.dll!";
		return E_FAIL;
	}

	return pURLOpenBlockingStream(pCaller, szURL, ppStream, dwReserved, lpfnCB);
}

HRESULT URLDownloadToFileHandler(LPUNKNOWN pCaller, LPCWSTR szURL, LPCWSTR szFileName, _Reserved_ DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB)
{
	HMODULE urlmondll = GetUrlDll();
	if (!urlmondll)
	{
		Logging::Log() << __FUNCTION__ << " Error: Cannnot open urlmon.dll!";
		return E_FAIL;
	}

	static URLDownloadToFileProc pURLDownloadToFile = (URLDownloadToFileProc)GetProcAddress(urlmondll, "URLDownloadToFileW");
	if (!pURLDownloadToFile)
	{
		Logging::Log() << __FUNCTION__ << " Error: Cannnot find 'URLOpenBlockingStreamA' in urlmon.dll!";
		return E_FAIL;
	}

	return pURLDownloadToFile(pCaller, szURL, szFileName, dwReserved, lpfnCB);
}

bool GetURLString(char*URL, std::string &data)
{
	IStream* stream;
	if (FAILED(URLOpenBlockingStreamHandler(nullptr, URL, &stream, 0, nullptr)))
	{
		Logging::Log() << __FUNCTION__ " Warning: Unable to check for update!";
		return false;
	}

	constexpr DWORD BufferSize = 1024;
	char buffer[BufferSize];
	DWORD bytesRead;
	data.clear();

	while (true)
	{
		stream->Read(buffer, BufferSize, &bytesRead);

		if (0U == bytesRead)
		{
			break;
		}

		data.append(buffer, bytesRead);
	};

	stream->Release();

	return true;
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

		VARIANT vDir;
		VariantInit(&vDir);
		vDir.vt = VT_BSTR;
		vDir.bstrVal = destFolder;

		// Destination is our zip file
		if (FAILED(pISD->NameSpace(vDir, &pToFolder)) || !pToFolder)
		{
			Logging::Log() << __FUNCTION__ " Failed to get source NameSpace!";
			break;
		}

		VARIANT vFile;
		VariantInit(&vFile);
		vFile.vt = VT_BSTR;
		vFile.bstrVal = sourceZip;

		if (FAILED(pISD->NameSpace(vFile, &pFromFolder)))
		{
			Logging::Log() << __FUNCTION__ " Failed to get destination NameSpace!";
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

		Sleep(0);
		hr = S_OK;
	}
	while (false);

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

void CheckForUpdate()
{
	// Get GitHub url for the latest released build
	std::string data;
	if (!GetURLString("https://api.github.com/repos/elishacloud/Silent-Hill-2-Enhancements/releases", data))
	{
		return;
	}

	// Break string into lines and parce each line
	data = std::regex_replace(data, std::regex(","), "\n");
	std::string line, url;
	std::istringstream s_data(data.c_str());
	while (std::getline(s_data, line))
	{
		if (line.find("html_url") != std::string::npos)
		{
			url = std::regex_replace(line, std::regex("html_url"), "");
			trim(url);
			url = std::regex_replace(url, std::regex("github.com"), "raw.githubusercontent.com");
			url = std::regex_replace(url, std::regex("/releases/tag/"), "/");
			url.append("/Resources/BuildNo.rc");
			break;
		}
	}

	// Get latest release build number from repository
	if (!GetURLString((char*)url.c_str(), data))
	{
		return;
	}

	// Get build number from data stream
	data = std::regex_replace(data, std::regex("#define BUILD_NUMBER"), "");
	trim(data);
	DWORD ReleaseBuildNo = std::strtoul(data.c_str(), nullptr, 10);

	// Check build number
	if (!ReleaseBuildNo)
	{
		Logging::Log() << __FUNCTION__ " Warning: Failed to get release build number!";
		return;
	}
	Logging::Log() << __FUNCTION__ " Latest release build found: " << ReleaseBuildNo;

	// Check if newer build is available
	if (BUILD_NUMBER == ReleaseBuildNo)
	{
		Logging::Log() << __FUNCTION__ " Using release build version!";
		return;
	}
	else if (BUILD_NUMBER > ReleaseBuildNo)
	{
		Logging::Log() << __FUNCTION__ " Using a build newer than the release build!";
		return;
	}
	Logging::Log() << __FUNCTION__ " Using an older build!";

	// *********************************************************************************
	// ToDo: prompt user for download
	// *********************************************************************************

	// Get zip file download url
	while (std::getline(s_data, line))
	{
		if (line.find("browser_download_url") != std::string::npos)
		{
			url = std::regex_replace(line, std::regex("browser_download_url"), "");
			trim(url);
			break;
		}
	}

	// Get Silent Hill folder path
	wchar_t path[MAX_PATH] = {}, name[MAX_PATH] = {};
	GetModuleFileName(m_hModule, path, MAX_PATH);
	wcscpy_s(name, MAX_PATH - wcslen(path) - 1, wcsrchr(path, '\\') + 1);
	wcscpy_s(wcsrchr(name, '.'), MAX_PATH - wcslen(name), L"\0");
	wcscpy_s(wcsrchr(path, '\\'), MAX_PATH - wcslen(path), L"\0");

	// Download the updated release build
	std::wstring downloadPath(std::wstring(path) + L"\\" + name + L".zip");
	if (FAILED(URLDownloadToFileHandler(nullptr, std::wstring(url.begin(), url.end()).c_str(), downloadPath.c_str(), 0, nullptr)))
	{
		Logging::Log() << __FUNCTION__ " Failed to download updated build: " << url;
		return;
	}
	Sleep(0);

	// Get update folder
	std::wstring updatePath(std::wstring(path) + L"\\~update");
	DeleteAllfiles(updatePath.c_str());
	RemoveDirectoryW(updatePath.c_str());
	CreateDirectoryW(updatePath.c_str(), nullptr);

	// Unzip downloaded
	if ((UnZipFile((BSTR)downloadPath.c_str(), (BSTR)updatePath.c_str())))
	{
		Logging::Log() << __FUNCTION__ " Failed to download updated build: " << url;

		// Remove temp files
		DeleteAllfiles(updatePath.c_str());
		RemoveDirectoryW(updatePath.c_str());
		DeleteFile(downloadPath.c_str());

		// Quit update
		return;
	}

	// Delete downloaded zip file
	DeleteFile(downloadPath.c_str());

	// *********************************************************************************
	// ToDo: update esiting d3d8.dll, d3d8.ini and d3d8.res files
	// *********************************************************************************

	// Remove temp files
	DeleteAllfiles(updatePath.c_str());
	RemoveDirectoryW(updatePath.c_str());
}
