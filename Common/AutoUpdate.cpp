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
#include <shellapi.h>
#include <urlmon.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include "Common\AutoUpdate.h"
#include "Common\Utils.h"
#include "Common\LoadModules.h"
#include "Wrappers\d3d8\d3d8wrapper.h"
#include "Logging\Logging.h"
#include "Resource.h"
#include "External\csvparser\src\rapidcsv.h"
#include "Unicode.h"

// Should be updated if we move the .csv
#define SH2EE_UPDATE_URL "http://www.enhanced.townofsilenthill.com/SH2/files/_sh2ee.csv"
#define SH2EE_SETUP_EXE_FILE "SH2EEsetup.exe"
#define SH2EE_SETUP_DATA_FILE "SH2EEsetup.dat"

typedef HRESULT(WINAPI *URLOpenBlockingStreamProc)(LPUNKNOWN pCaller, LPCSTR szURL, LPSTREAM *ppStream, _Reserved_ DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB);
typedef HRESULT(WINAPI *URLDownloadToFileProc)(LPUNKNOWN pCaller, LPCWSTR szURL, LPCWSTR szFileName, _Reserved_ DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB);

bool IsSetupToolUpdateAvailable = false;
bool IsProjectUpdateAvailable = false;

namespace
{
	const char removechars[] = " \t\n\r";
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

std::string ReadFileContents(std::wstring &path)
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

bool NewModuleReleaseBuildAvailable(std::string &urlDownload)
{
	// Get GitHub url for the latest released build
	std::string data;
	if (!GetURLString("https://api.github.com/repos/elishacloud/Silent-Hill-2-Enhancements/releases", data))
	{
		return false;
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
			trim(urlBuildNo, " {}:\"\t\n\r");
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
		return false;
	}

	// Get build number from data stream
	data = std::regex_replace(data, std::regex("#define BUILD_NUMBER"), "");
	trim(data);
	DWORD ReleaseBuildNo = std::strtoul(data.c_str(), nullptr, 10);

	// Check build number
	if (!ReleaseBuildNo)
	{
		Logging::Log() << __FUNCTION__ " Warning: Failed to get release build number!";
		return false;
	}
	Logging::Log() << "Latest release build found: " << ReleaseBuildNo;

	// Check if newer build is available
	if (BUILD_NUMBER == ReleaseBuildNo)
	{
		Logging::Log() << "Using release build version!";
		return false;
	}
	else if (BUILD_NUMBER > ReleaseBuildNo)
	{
		Logging::Log() << "Using a build newer than the release build!";
		return false;
	}

	Logging::Log() << __FUNCTION__ " Using an older build!";
	return true;
}

bool NewProjectReleaseAvailable(std::string &path_str)
{
	// Parse local CSV file
	rapidcsv::Document localcsv(path_str + "\\" + SH2EE_SETUP_DATA_FILE, rapidcsv::LabelParams(), rapidcsv::SeparatorParams(),
		rapidcsv::ConverterParams(),
		rapidcsv::LineReaderParams(true /* pSkipCommentLines */, '#' /* pCommentPrefix */));

	// Error checking
	if (localcsv.GetColumnIdx("id") < 0 || localcsv.GetColumnIdx("isInstalled") < 0 || localcsv.GetColumnIdx("version") < 0)
	{
		return false;
	}

	std::vector<std::string> localcsv_id = localcsv.GetColumn<std::string>("id");
	std::vector<std::string> localcsv_isInstalled = localcsv.GetColumn<std::string>("isInstalled");
	std::vector<std::string> localcsv_version = localcsv.GetColumn<std::string>("version");

	// Get and parse web CSV
	std::string webcsv;
	if (!GetURLString(SH2EE_UPDATE_URL, webcsv))
	{
		return false;
	}

	std::stringstream webcsv_sstream(webcsv);
	rapidcsv::Document doc(webcsv_sstream, rapidcsv::LabelParams(), rapidcsv::SeparatorParams(),
		rapidcsv::ConverterParams(),
		rapidcsv::LineReaderParams(true /* pSkipCommentLines */, '#' /* pCommentPrefix */));

	// Error checking
	if (doc.GetColumnIdx("id") < 0 || doc.GetColumnIdx("version") < 0)
	{
		return false;
	}

	std::vector<std::string> webcsv_id = doc.GetColumn<std::string>("id");
	std::vector<std::string> webcsv_version = doc.GetColumn<std::string>("version");

	// Check if there is an update available for the Setup Tool
	if (localcsv_version[0] != webcsv_version[0])
	{
		Logging::Log() << "Setup Tool update found. Current version: " << localcsv_version[0] << ", New version: " << webcsv_version[0];

		IsSetupToolUpdateAvailable = true;
		return true;
	}

	// Check if there is an update available for all the components
	if (std::size(localcsv_id) == std::size(webcsv_id))
	{
		for (std::size_t i{}; i != std::size(localcsv_id); ++i)
		{
			if (localcsv_isInstalled[i] != "false")
			{
				if (localcsv_version[i] != webcsv_version[i])
				{
					Logging::Log() << "\"" << localcsv_id[i] << "\"" << " update found. Current version: " << localcsv_version[i] << ", New version: " << webcsv_version[i];
					IsProjectUpdateAvailable = true;
				}
			}
		}

		if (IsProjectUpdateAvailable)
		{
			return true;
		}
	}

	return false;
}

template<typename T, typename D>
void GetSH2Path(T &path, T &name)
{
	D t_path[MAX_PATH] = {}, t_name[MAX_PATH] = {};
	bool ret = GetModulePath(t_path, MAX_PATH);
	D* pdest = strrchr(t_path, '\\');
	if (ret && pdest)
	{
		strcpy_s(t_name, MAX_PATH - strlen(t_path), pdest + 1);
		pdest = strrchr(t_name, '.');
		if (pdest)
		{
			*pdest = '\0';
		}
		pdest = strrchr(t_path, '\\');
		if (pdest)
		{
			*pdest = '\0';
		}
		path.assign(t_path);
		name.assign(t_name);
		std::transform(name.begin(), name.end(), name.begin(), [](D c) { return (D)towlower(c); });
	}
}

HRESULT UpdatedllFile(std::wstring &currentDll, std::wstring &tempDll, std::wstring &updatePath)
{
	// Read configuration file
	std::wstring newdll(updatePath + L"\\d3d8.dll");

	// Move current dll to temp
	if (!MoveFile(currentDll.c_str(), tempDll.c_str()))
	{
		Logging::Log() << __FUNCTION__ " Error: Failed to rename current dll!";
		return E_FAIL;
	}

	// Move updated dll to primary
	if (!MoveFile(newdll.c_str(), currentDll.c_str()))
	{
		// If failed then restore current dll
		Logging::Log() << __FUNCTION__ " Error: Failed to rename updated dll!";
		if (!MoveFile(tempDll.c_str(), currentDll.c_str()))
		{
			Logging::Log() << __FUNCTION__ " Error: Failed to restore current dll!";
		}
		return E_FAIL;
	}

	return S_OK;
}

std::string MergeiniFile(std::stringstream &s_currentini, std::stringstream &s_ini, bool OverWriteCurrent)
{
	// Merge current settings with new ini file
	std::string newini, line, tmpline;
	std::vector<DWORD> linesadded;
	while (std::getline(s_ini, line))
	{
		trim(line);
		if (line[0] == ';' || line.find("=") == std::string::npos)
		{
			newini.append(line + "\n");
		}
		else
		{
			bool found = false;
			DWORD counter = 0;
			DWORD loc = min(line.find_first_of(" "), line.find_first_of("="));
			s_currentini.clear();
			s_currentini.seekg(0, std::ios::beg);
			while (std::getline(s_currentini, tmpline))
			{
				counter++;
				trim(tmpline);
				DWORD tloc = min(tmpline.find_first_of(" "), tmpline.find_first_of("="));
				if (loc == tloc && MatchCount(line, tmpline) >= loc)
				{
					found = true;
					linesadded.push_back(counter);
					if (OverWriteCurrent)
					{
						newini.append(line + "\n");
					}
					else
					{
						newini.append(tmpline + "\n");
					}
					break;
				}
			}
			if (!found)
			{
				newini.append(line + "\n");
			}
		}
	}

	// Find extra settings that don't exist in the new ini file
	DWORD counter = 0;
	std::string extraini, tmpphrase;
	s_currentini.clear();
	s_currentini.seekg(0, std::ios::beg);
	while (std::getline(s_currentini, line))
	{
		counter++;
		trim(line);
		if (std::find(linesadded.begin(), linesadded.end(), counter) != linesadded.end())
		{
			// Match found
			tmpphrase.clear();
		}
		else
		{
			tmpphrase.append(line + "\n");
			if (!line.size() || line[0] == '[')
			{
				tmpphrase.clear();
			}
			else if (tmpphrase.size() && line[0] != ';' && line.find("=") != std::string::npos)
			{
				extraini.append(tmpphrase + "\n");
				tmpphrase.clear();
			}
		}
	}

	// Add extra settings to the new ini file
	if (extraini.size())
	{
		size_t pos = newini.find("\n[Extra]");
		if (pos != std::string::npos)
		{
			size_t newpos = newini.find("\n[", pos + 8);
			if (pos != std::string::npos)
			{
				newini.insert(newpos, extraini);
			}
			else
			{
				newini.append(extraini);
			}
		}
		else
		{
			extraini.insert(0, "[Extra]\n");
			pos = newini.find("\n[");
			if (pos != std::string::npos)
			{
				newini.insert(pos + 1, extraini);
			}
			else
			{
				newini.append(extraini);
			}
		}
	}

	return newini;
}

HRESULT UpdateiniFile(std::wstring &path, std::wstring &name, std::wstring &updatePath)
{
	// Read configuration file
	std::wstring configPath(path + L"\\" + name + L".ini");
	std::stringstream s_currentini(ReadFileContents(configPath));

	// Read new ini file
	std::wstring iniPath(updatePath + L"\\d3d8.ini");
	std::stringstream s_ini(ReadFileContents(iniPath));

	// Check config files
	if (s_currentini.str().empty() || s_ini.str().empty())
	{
		Logging::Log() << __FUNCTION__ " Failed to read ini file!";
		return E_FAIL;
	}

	// Merge ini files
	std::string newini = MergeiniFile(s_currentini, s_ini);

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

HRESULT UpdateAllFiles(std::wstring &path, std::wstring &name, std::wstring &updatePath)
{
	// Find the first file in the directory.
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(std::wstring(updatePath + L"\\*").c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;

	// Copy all the files in update directory
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::wstring filename(ffd.cFileName);
			filename = std::regex_replace(filename, std::wregex(L"d3d8."), name + L".");

			if (!CopyFile(std::wstring(updatePath + L"\\" + ffd.cFileName).c_str(), std::wstring(path + L"\\" + filename).c_str(), FALSE))
			{
				hr = E_FAIL;
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	DWORD err = GetLastError();
	if (err != ERROR_NO_MORE_FILES)
	{
		Logging::Log() << "Error updating accessory files!";
		hr = err;
	}

	FindClose(hFind);

	return hr;
}

void RestoreMainWindow()
{
	if (IsWindow(DeviceWindow) && IsIconic(DeviceWindow))
	{
		ShowWindow(DeviceWindow, SW_RESTORE);
	}
}

LRESULT WINAPI ChangeCaptionButtons(int nCode, WPARAM wParam, LPARAM)
{
	if (nCode == HCBT_ACTIVATE)
	{
		SetWindowText(GetDlgItem((HWND)wParam, IDOK), L"Confirm");
	}
	return 0;
}

DWORD WINAPI CheckForUpdate(LPVOID)
{
	// Get Silent Hill 2 folder paths
	std::wstring path, name;
	GetSH2Path<std::wstring, wchar_t>(path, name);
	if (m_StopThreadFlag || path.empty() || name.empty())
	{
		Logging::Log() << __FUNCTION__ " Failed to get module path or name!";
		return S_OK;
	}
	std::wstring downloadPath(path + L"\\" + name + L".zip");
	std::wstring updatePath(path + L"\\~update");
	std::wstring currentDll(path + L"\\" + name + L".dll");
	std::wstring tempDll(path + L"\\~" + name + L".dll");
	std::wstring SH2EEsetupExePath(path + L"\\" + TEXT(SH2EE_SETUP_EXE_FILE));

	// Delete old module if it exists
	DeleteFile(tempDll.c_str());

	// Check if file exists in the path
	std::string urlDownload;
	std::string path_str, name_str;
	GetSH2Path<std::string, char>(path_str, name_str);
	if (PathFileExistsA(std::string(path_str + "\\" + SH2EE_SETUP_DATA_FILE).c_str()) && PathFileExistsA(std::string(path_str + "\\" + SH2EE_SETUP_EXE_FILE).c_str()))
	{
		Logging::Log() << __FUNCTION__ " " << SH2EE_SETUP_DATA_FILE << " exists, using CSV for update checking...";
		// Check if there is a new project update
		if (m_StopThreadFlag || !NewProjectReleaseAvailable(path_str))
		{
			return S_OK;
		}
	}
	else
	{
		// Check if there is a newer module update
		if (m_StopThreadFlag || !NewModuleReleaseBuildAvailable(urlDownload))
		{
			return S_OK;
		}
	}

	// Wait for main window handle
	std::string app_name(APP_NAME);
	std::wstring MsgTitle(app_name.begin(), app_name.end());
	while (!m_StopThreadFlag && !IsWindow(DeviceWindow))
	{
		Sleep(1000);
	}

	// Prompt user for download
	if (!m_StopThreadFlag)
	{
		if (IsProjectUpdateAvailable || IsSetupToolUpdateAvailable)
		{
			// Update SH2EE project
			IsUpdating = true;
			std::wstring param;

			// Ask user for update
			int Response = MessageBox(DeviceWindow, L"There is an update for Silent Hill 2: Enhanced Edition. Would you like to close the game and launch the updater?", MsgTitle.c_str(), MB_YESNO | MB_ICONINFORMATION | MB_SYSTEMMODAL);
			if (Response == IDYES)
			{
				// Decide which parameter to use
				if (IsProjectUpdateAvailable)
				{
					param = L"-update";
				}
				else if (IsSetupToolUpdateAvailable)
				{
					param = L"-selfUpdate";
				}

				// Run SH2EEsetup.exe
				if (ShellExecute(nullptr, L"open", SH2EEsetupExePath.data(), param.data(), nullptr, SW_SHOWDEFAULT) > (HINSTANCE)32)
				{
					exit(0);
					return S_OK;
				}
			}
			else
			{
				Logging::Log() << __FUNCTION__ " User chose not to update the project!";
				IsUpdating = false;
				RestoreMainWindow();
				return S_OK;
			}
		}
		else
		{
			// Update SH2E module only
			IsUpdating = true;

			// Ask user for update
			int Response = MessageBox(DeviceWindow, L"There is an update for the SH2 Enhancements module. Would you like to update?", MsgTitle.c_str(), MB_YESNO | MB_ICONINFORMATION | MB_SYSTEMMODAL);
			if (Response == IDNO)
			{
				Logging::Log() << __FUNCTION__ " User chose not to update the build!";
				IsUpdating = false;
				RestoreMainWindow();
				return S_OK;
			}

			// Notify user to download other packages
			HHOOK hook = SetWindowsHookEx(WH_CBT, ChangeCaptionButtons, GetModuleHandle(nullptr), GetCurrentThreadId());
			MessageBox(DeviceWindow, L"Note: This only updates the SH2 Enhancements module. You must manually download and update other enhancement packages from the project's website.", MsgTitle.c_str(), MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
			UnhookWindowsHookEx(hook);
		}
	}

	HRESULT hr = S_OK;

	// Update module and accessory files
	do
	{
		// Download the updated release build
		if (m_StopThreadFlag || FAILED(URLDownloadToFileHandler(nullptr, std::wstring(urlDownload.begin(), urlDownload.end()).c_str(), downloadPath.c_str(), 0, nullptr)))
		{
			Logging::Log() << __FUNCTION__ " Failed to download updated build: " << urlDownload;
			hr = E_FAIL;
			break;
		}

		// Unzip downloaded file
		if (m_StopThreadFlag || FAILED(UnZipFile((BSTR)downloadPath.c_str(), (BSTR)updatePath.c_str())))
		{
			Logging::Log() << __FUNCTION__ " Failed to unzip release download!";
			hr = E_FAIL;
			break;
		}

		// Update dll file
		if (m_StopThreadFlag || FAILED(UpdatedllFile(currentDll, tempDll, updatePath)))
		{
			Logging::Log() << __FUNCTION__ " Failed to update dll file!";
			hr = E_FAIL;
			break;
		}

		// Update ini configuration file
		if (m_StopThreadFlag || FAILED(UpdateiniFile(path, name, updatePath)))
		{
			Logging::Log() << __FUNCTION__ " Failed to update ini file!";
			hr = E_FAIL;
			break;
		}

		// Update accessory files
		if (m_StopThreadFlag || FAILED(UpdateAllFiles(path, name, updatePath)))
		{
			Logging::Log() << __FUNCTION__ " Failed to update accessory files!";
			hr = E_FAIL;
			break;
		}
	} while (false);

	// Remove temp files
	DeleteFile(downloadPath.c_str());
	DeleteAllfiles(updatePath.c_str());
	RemoveDirectory(updatePath.c_str());

	// Prompt for results
	if (!m_StopThreadFlag)
	{
		// Update succeeded
		if (SUCCEEDED(hr))
		{
			Logging::Log() << __FUNCTION__ " Successfully updated module!";

			int Response = MessageBox(DeviceWindow, L"Update complete! You must restart the game for the update to take effect. Would you like to restart the game now?", MsgTitle.c_str(), MB_YESNO | MB_ICONINFORMATION | MB_SYSTEMMODAL);
			if (Response == IDYES)
			{
				// Get Silent Hill 2 file path and restart
				wchar_t sh2path[MAX_PATH];
				if (GetSH2FolderPath(sh2path, MAX_PATH) && ShellExecute(nullptr, L"open", sh2path, nullptr, nullptr, SW_SHOWDEFAULT) > (HINSTANCE)32)
				{
					exit(0);
					return S_OK;
				}
			}
		}
		// Update failed
		else
		{
			Logging::Log() << __FUNCTION__ " Update Failed!";
			
			if (!IsProjectUpdateAvailable)
			{
				MessageBox(DeviceWindow, L"Update FAILED! You will need to manually update the SH2 Enhancements module!", MsgTitle.c_str(), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			}
			else
			{
				MessageBox(DeviceWindow, std::wstring(L"Failed to launch the updater! Please try to manually run " + std::wstring(TEXT(SH2EE_SETUP_EXE_FILE)) + L" and update from there.").c_str(), MsgTitle.c_str(), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			}
		}
	}

	IsUpdating = false;

	RestoreMainWindow();

	return hr;
}
