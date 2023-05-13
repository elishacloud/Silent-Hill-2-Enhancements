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

#include "dinput8wrapper.h"
#include "Wrappers\wrapper.h"
#include "Common\Utils.h"

AddressLookupTableDinput8<void> ProxyAddressLookupTableDinput8 = AddressLookupTableDinput8<void>();

DirectInput8CreateProc m_pDirectInput8Create = nullptr;

bool LostWindowFocus = false;
HWND CooperativeLevelWindow = nullptr;
HWINEVENTHOOK hEventHook = nullptr;

// Hook dinput8 API
void HookDirectInput8Create()
{
	// Get DirectInput8Create address
	constexpr BYTE SearchBytes[]{ 0x85, 0xC0, 0x7D, 0x03, 0x33, 0xC0, 0xC3, 0x8B, 0x54, 0x24, 0x08, 0xA1 };
	DWORD Address = SearchAndGetAddresses(0x00406403, 0x00406403, 0x00406413, SearchBytes, sizeof(SearchBytes), -0x05, __FUNCTION__);

	// Checking address pointer
	if (!Address)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get function address
	m_pDirectInput8Create = (DirectInput8CreateProc)(Address + 5 + *(DWORD*)(Address + 1));

	// Write to memory
	WriteCalltoMemory((BYTE*)Address, *DirectInput8CreateWrapper, 5);
}

// Dinput8 DirectInput8Create API wrapper
HRESULT WINAPI DirectInput8CreateWrapper(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	LOG_LIMIT(3, "Redirecting 'DirectInput8Create' ...");

	HRESULT hr = m_pDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	RunDelayedOneTimeItems();

	if (SUCCEEDED(hr))
	{
		if (riidltf == IID_IDirectInput8A)
		{
			*ppvOut = new m_IDirectInput8A((IDirectInput8A*)*ppvOut);
		}
		else
		{
			genericQueryInterface(riidltf, ppvOut);
		}
	}
	else
	{
		Logging::Log() << "'DirectInput8Create' Failed! Error: " << (DIERR)hr;
	}

	// Set window hook to check when SH2 loses focus
	static bool FirstRun = true;
	if (FirstRun)
	{
		Logging::Log() << "Setting DirectInput window hook...";

		if (HookWndProc)
		{
			void CALLBACK windowChangeHook(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

			hEventHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, NULL, &windowChangeHook, 0, 0, WINEVENT_OUTOFCONTEXT);
		}

		// Reset FirstRun
		FirstRun = false;
	}

	return hr;
}

void FixMouseClick(HWND hwnd)
{
	// Check if mouse pointer is inside the Silent Hill 2 window
	if (!IsIconic(hwnd))
	{
		RECT Rect = {};
		if (GetWindowRect(hwnd, &Rect))
		{
			POINT Point = {};
			GetCursorPos(&Point);
			if (Point.x < Rect.left || Point.x > Rect.right || Point.y < Rect.top || Point.y > Rect.bottom)
			{
				// Get center of the Silent Hill 2 window
				const int x = Rect.left + ((Rect.right - Rect.left) / 2);
				const int y = Rect.top + ((Rect.bottom - Rect.top) / 2);
				SetCursorPos(x, y);
			}
		}
	}

	// Send mouse event when focus is lost to clear mouse click
	mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}

void CALLBACK windowChangeHook(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	UNREFERENCED_PARAMETER(hWinEventHook);
	UNREFERENCED_PARAMETER(event);
	UNREFERENCED_PARAMETER(idObject);
	UNREFERENCED_PARAMETER(idChild);
	UNREFERENCED_PARAMETER(dwEventThread);
	UNREFERENCED_PARAMETER(dwmsEventTime);

	if (!CooperativeLevelWindow)
	{
		return;
	}

	// Check window focus
	if (CooperativeLevelWindow == hwnd)
	{
		LostWindowFocus = false;

		FixMouseClick(hwnd);
	}
	else
	{
		LostWindowFocus = true;
	}
}

void UnhookWindowHandle()
{
	if (hEventHook)
	{
		Logging::Log() << "Unhooking window hook";
		UnhookWinEvent(hEventHook);
	}
}
