/**
* Copyright (C) 2017 Elisha Riedlinger
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
*
* Code in EraseCppComments, Read, Parse and ParseCallback functions taken from source code found in Aqrit's ddwrapper
* http://bitpatch.com/ddwrapper.html
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "..\Common\Logging.h"

// Declare constants
static constexpr LONG MinWindowWidth = 320;			// Minimum window width for valid window check
static constexpr LONG MinWindowHeight = 240;		// Minimum window height for valid window check
static constexpr LONG WindowDelta = 40;				// Delta between window size and screensize for fullscreen check
static constexpr DWORD TerminationCount = 10;		// Minimum number of loops to check for termination
static constexpr DWORD TerminationWaitTime = 2000;	// Minimum time to wait for termination (LoopSleepTime * NumberOfLoops)

// Declare structures
struct screen_res
{
	LONG Width = 0;
	LONG Height = 0;

	screen_res& operator=(const screen_res& a)
	{
		Width = a.Width;
		Height = a.Height;
		return *this;
	}

	bool operator==(const screen_res& a) const
	{
		return (Width == a.Width && Height == a.Height);
	}

	bool operator!=(const screen_res& a) const
	{
		return (Width != a.Width || Height != a.Height);
	}
};

struct window_layer
{
	HWND hwnd = nullptr;
	bool IsMain = false;
	bool IsFullScreen = false;
};

struct handle_data
{
	DWORD process_id = 0;
	HWND best_handle = nullptr;
	DWORD LayerNumber = 0;
	window_layer Windows[256];
	bool AutoDetect = true;
	bool Debug = false;
};

// Global vars
screen_res m_Current_ScreenRes;

bool IsMainWindow(HWND hwnd)
{
	return GetWindow(hwnd, GW_OWNER) == (HWND)0 && IsWindowVisible(hwnd);
}

bool IsWindowTooSmall(screen_res WindowSize)
{
	return WindowSize.Width < MinWindowWidth || WindowSize.Height < MinWindowHeight;
}

bool IsWindowFullScreen(screen_res WindowSize, screen_res ScreenSize)
{
	return abs(ScreenSize.Width - WindowSize.Width) <= WindowDelta ||		// Window width matches screen width
		abs(ScreenSize.Height - WindowSize.Height) <= WindowDelta;			// Window height matches screen height
}

// Gets the window size from a handle
void GetWindowSize(HWND& hwnd, screen_res& Res, RECT& rect)
{
	GetWindowRect(hwnd, &rect);
	Res.Width = abs(rect.right - rect.left);
	Res.Height = abs(rect.bottom - rect.top);
}

// Gets the screen size from a wnd handle
void GetScreenSize(HWND& hwnd, screen_res& Res, MONITORINFO& mi)
{
	GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
	Res.Width = mi.rcMonitor.right - mi.rcMonitor.left;
	Res.Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
}

// Enums all windows and returns the handle to the active window
BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam)
{
	// Get varables from call back
	handle_data& data = *(handle_data*)lParam;

	// Skip windows that are from a different process ID
	DWORD process_id;
	GetWindowThreadProcessId(hwnd, &process_id);
	if (data.process_id != process_id)
	{
		return true;
	}

	// Skip compatibility class windows
	char class_name[80] = "";
	GetClassNameA(hwnd, class_name, sizeof(class_name));
	if (strcmp(class_name, "CompatWindowDesktopReplacement") == 0)			// Compatibility class windows
	{
		return true;
	}

	// Skip windows of zero size
	RECT rect = { sizeof(rect) };
	screen_res WindowSize;
	GetWindowSize(hwnd, WindowSize, rect);
	if (WindowSize.Height == 0 && WindowSize.Width == 0)
	{
		return true;
	}

	// Declare vars
	MONITORINFO mi = { sizeof(mi) };
	screen_res ScreenSize;

	// Get window and monitor information
	GetScreenSize(hwnd, ScreenSize, mi);

	// Store window layer information
	++data.LayerNumber;
	data.Windows[data.LayerNumber].hwnd = hwnd;
	data.Windows[data.LayerNumber].IsFullScreen = IsWindowFullScreen(WindowSize, ScreenSize);
	data.Windows[data.LayerNumber].IsMain = IsMainWindow(hwnd);

	// Check if the window is the best window
	if (data.Windows[data.LayerNumber].IsFullScreen && data.Windows[data.LayerNumber].IsMain)
	{
		// Match found returning value
		data.best_handle = hwnd;
		return false;
	}

	// Return to loop again
	return true;
}

// Finds the active window
HWND FindMainWindow(DWORD process_id, bool AutoDetect)
{
	// Set varables
	HWND WindowsHandle = nullptr;
	handle_data data;
	data.best_handle = nullptr;
	data.process_id = process_id;
	data.AutoDetect = AutoDetect;
	data.LayerNumber = 0;

	// Gets all window layers and looks for a main window that is fullscreen
	EnumWindows(EnumWindowsCallback, (LPARAM)&data);
	WindowsHandle = data.best_handle;

	// If no main fullscreen window found then check for other windows
	if (!WindowsHandle)
	{
		for (DWORD x = 1; x <= data.LayerNumber; x++)
		{
			// Return the first fullscreen window layer
			if (data.Windows[x].IsFullScreen)
			{
				return data.Windows[x].hwnd;
			}
			// If no fullscreen layer then return the first 'main' window
			if (!WindowsHandle && data.Windows[x].IsMain)
			{
				WindowsHandle = data.Windows[x].hwnd;
			}
		}
	}

	// Get first window handle if no handle has been found yet
	if (!WindowsHandle && data.LayerNumber > 0)
	{
		WindowsHandle = data.Windows[1].hwnd;
	}

	// Return the best handle
	return WindowsHandle;
}

// Checks is the current screen res is smaller and returns the smaller screen res
void CheckCurrentScreenRes()
{
	// Declare vars
	HWND hwnd;
	MONITORINFO mi = { sizeof(mi) };
	screen_res ScreenSize;

	// Get window handle
	hwnd = FindMainWindow(GetCurrentProcessId(), true);

	// Get monitor size
	GetScreenSize(hwnd, ScreenSize, mi);

	// Check if screen resolution is too small
	if (!IsWindowTooSmall(ScreenSize))
	{
		// Save screen resolution
		InterlockedExchange(&m_Current_ScreenRes.Width, ScreenSize.Width);
		InterlockedExchange(&m_Current_ScreenRes.Height, ScreenSize.Height);
	}
}

// Check with resolution is best
LONG GetBestResolution(screen_res& ScreenRes, LONG xWidth, LONG xHeight)
{
	//Set vars
	DEVMODE dm = { 0 };
	dm.dmSize = sizeof(dm);
	LONG diff = 40000;
	LONG NewDiff = 0;
	ScreenRes.Width = 0;
	ScreenRes.Height = 0;

	// Get closest resolution
	for (DWORD iModeNum = 0; EnumDisplaySettings(nullptr, iModeNum, &dm) != 0; iModeNum++)
	{
		NewDiff = abs((LONG)dm.dmPelsWidth - xWidth) + abs((LONG)dm.dmPelsHeight - xHeight);
		if (NewDiff < diff)
		{
			diff = NewDiff;
			ScreenRes.Width = (LONG)dm.dmPelsWidth;
			ScreenRes.Height = (LONG)dm.dmPelsHeight;
		}
	}
	return diff;
}

// Sets the resolution of the screen
void SetScreenResolution(LONG xWidth, LONG xHeight)
{
	DEVMODE newSettings;
	if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &newSettings) != 0)
	{
		newSettings.dmPelsWidth = xWidth;
		newSettings.dmPelsHeight = xHeight;
		newSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN);
	}
}

// Verifies input and sets screen res to the values sent
void SetScreen(screen_res ScreenRes)
{
	// Verify stored values are large enough
	if (!IsWindowTooSmall(ScreenRes))
	{
		// Get the best screen resolution
		GetBestResolution(ScreenRes, ScreenRes.Width, ScreenRes.Height);

		// Set screen to new resolution
		SetScreenResolution(ScreenRes.Width, ScreenRes.Height);
	}
}

// Resets the screen to the registry-stored values
void ResetScreen()
{
	Log() << "Reseting screen resolution...";

	// Setting screen resolution to fix some display errors on exit
	SetScreen(m_Current_ScreenRes);

	// Sleep short amout of time
	Sleep(0);

	// Reset resolution
	ChangeDisplaySettings(nullptr, 0);
}
