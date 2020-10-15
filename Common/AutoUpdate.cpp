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
#include <shlwapi.h>
#include <shellapi.h>
#include <urlmon.h>
#include <sstream>
#include <iostream>
#include <fstream>
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
		Logging::Log() << __FUNCTION__ << " Error: Cannnot find 'URLDownloadToFileW' in urlmon.dll!";
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
			CreateDirectoryW(destFolder, nullptr);
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

DWORD MatchCount(std::string &path1, std::string &path2)
{
	DWORD size = min(path1.size(), path2.size());
	for (DWORD x = 0; x < size; x++)
	{
		if (path1[x] != path2[x])
		{
			return x;
		}
	}
	return size;
}

std::string ReadFile(std::wstring &path)
{
	std::wifstream in(path.c_str());
	if (!in)
	{
		Logging::Log() << __FUNCTION__ " Failed to open file: " << path.c_str();
		return "";
	}
	std::wostringstream ss;
	ss << in.rdbuf();
	std::wstring str(ss.str());
	in.close();
	return std::string(str.begin(), str.end());
}

HRESULT GetReleaseBuild(std::string &urlDownload)
{
	// Get GitHub url for the latest released build
	std::string data;
	if (!GetURLString("https://api.github.com/repos/elishacloud/Silent-Hill-2-Enhancements/releases", data))
	{
		return E_FAIL;
	}

	// Break string into lines and parce each line
	data = std::regex_replace(data, std::regex(","), "\n");
	std::string line, urlBuildNo;
	std::istringstream s_data(data.c_str());
	while (std::getline(s_data, line))
	{
		if (line.find("html_url") != std::string::npos)
		{
			// Get url from line
			urlBuildNo = std::regex_replace(line, std::regex("html_url"), "");
			trim(urlBuildNo);
			urlDownload.assign(urlBuildNo);

			// Get url for build number
			urlBuildNo = std::regex_replace(urlBuildNo, std::regex("github.com"), "raw.githubusercontent.com");
			urlBuildNo = std::regex_replace(urlBuildNo, std::regex("/releases/tag/"), "/");
			urlBuildNo.append("/Resources/BuildNo.rc");

			// Get download url
			urlDownload = std::regex_replace(urlDownload, std::regex("/releases/tag/"), "/releases/download/");
			urlDownload.append("/d3d8.zip");
			break;
		}
	}

	// Get latest release build number from repository
	if (!GetURLString((char*)urlBuildNo.c_str(), data))
	{
		Logging::Log() << __FUNCTION__ " Failed to download release build number: " << urlBuildNo;
		return E_FAIL;
	}

	// Get build number from data stream
	data = std::regex_replace(data, std::regex("#define BUILD_NUMBER"), "");
	trim(data);
	DWORD ReleaseBuildNo = std::strtoul(data.c_str(), nullptr, 10);

	// Check build number
	if (!ReleaseBuildNo)
	{
		Logging::Log() << __FUNCTION__ " Warning: Failed to get release build number!";
		return E_FAIL;
	}
	Logging::Log() << __FUNCTION__ " Latest release build found: " << ReleaseBuildNo;

	// Check if newer build is available
	if (BUILD_NUMBER == ReleaseBuildNo)
	{
		Logging::Log() << __FUNCTION__ " Using release build version!";
		return E_FAIL;
	}
	else if (BUILD_NUMBER > ReleaseBuildNo)
	{
		Logging::Log() << __FUNCTION__ " Using a build newer than the release build!";
		return E_FAIL;
	}

	Logging::Log() << __FUNCTION__ " Using an older build!";
	return S_OK;
}

void GetSH2Path(std::wstring &path, std::wstring &name)
{
	wchar_t t_path[MAX_PATH] = {}, t_name[MAX_PATH] = {};
	if (GetModuleFileName(m_hModule, t_path, MAX_PATH) && wcsrchr(t_path, '\\'))
	{
		wcscpy_s(t_name, MAX_PATH - wcslen(t_path) - 1, wcsrchr(t_path, '\\') + 1);
		if (wcsrchr(t_path, '.'))
		{
			wcscpy_s(wcsrchr(t_name, '.'), MAX_PATH - wcslen(t_name), L"\0");
		}
		wcscpy_s(wcsrchr(t_path, '\\'), MAX_PATH - wcslen(t_path), L"\0");
		path.assign(t_path);
		name.assign(t_name);
	}
}

HRESULT UpdateiniFile(std::wstring &path, std::wstring &name, std::wstring &updatePath)
{
	// Read configuration file
	std::wstring configPath(path + L"\\" + name + L".ini");
	std::istringstream s_currentini(ReadFile(configPath).c_str());

	// Read new ini file
	std::wstring iniPath(updatePath + L"\\d3d8.ini");
	std::istringstream s_ini(ReadFile(iniPath).c_str());

	// Check config files
	if (s_currentini.str().empty() || s_ini.str().empty())
	{
		Logging::Log() << __FUNCTION__ " Failed to read ini file!";
		return E_FAIL;
	}

	// Merge current settings with new ini file
	std::string newini, line, tmpline;
	while (std::getline(s_ini, line))
	{
		trim(line, " \t\n\r");
		if (line[0] == ';' || line.find("=") == std::string::npos)
		{
			newini.append(line + "\n");
		}
		else
		{
			bool found = false;
			size_t size = line.find_first_of(" =");
			s_currentini.clear();
			s_currentini.seekg(0, std::ios::beg);
			while (std::getline(s_currentini, tmpline))
			{
				trim(tmpline, " \t\n\r");
				if (MatchCount(line, tmpline) >= size)
				{
					found = true;
					newini.append(tmpline + "\n");
					break;
				}
			}
			if (!found)
			{
				newini.append(line + "\n");
			}
		}
	}

	// Write updated ini file
	std::ofstream out(configPath);
	if (!out)
	{
		Logging::Log() << __FUNCTION__ " Failed to save ini file!";
		return E_FAIL;
	}
	out << newini;
	out.close();
	DeleteFile(iniPath.c_str());

	return S_OK;
}

void CheckForUpdate()
{
	// Check if there is a newer update
	std::string urlDownload;
	if (FAILED(GetReleaseBuild(urlDownload)))
	{
		return;
	}

	// *********************************************************************************
	// ToDo: prompt user for download
	// *********************************************************************************

	// Get Silent Hill 2 folder path
	std::wstring path, name;
	GetSH2Path(path, name);
	if (path.empty() || name.empty())
	{
		Logging::Log() << __FUNCTION__ " Failed to module path or name!";
		return;
	}
	std::wstring downloadPath(path + L"\\" + name + L".zip");
	std::wstring updatePath(path + L"\\~update");

	// Update module and accessory files
	do
	{
		// Download the updated release build
		if (FAILED(URLDownloadToFileHandler(nullptr, std::wstring(urlDownload.begin(), urlDownload.end()).c_str(), downloadPath.c_str(), 0, nullptr)))
		{
			Logging::Log() << __FUNCTION__ " Failed to download updated build: " << urlDownload;
			break;
		}

		// Unzip downloaded file
		if ((UnZipFile((BSTR)downloadPath.c_str(), (BSTR)updatePath.c_str())))
		{
			Logging::Log() << __FUNCTION__ " Failed to unzip release download!";
			break;
		}

		// Update ini configuration file
		if (FAILED(UpdateiniFile(path, name, updatePath)))
		{
			Logging::Log() << __FUNCTION__ " Failed to update ini file!";
			break;
		}

		// *********************************************************************************
		// ToDo: update existing d3d8.dll and d3d8.res files
		// *********************************************************************************

	} while (false);

	// Remove temp files
	DeleteFile(downloadPath.c_str());
	DeleteAllfiles(updatePath.c_str());
	RemoveDirectoryW(updatePath.c_str());
}
