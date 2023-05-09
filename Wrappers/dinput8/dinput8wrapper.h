#pragma once

#define INITGUID

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class m_IDirectInput8A;
class m_IDirectInputDevice8A;
class m_IDirectInputEffect;

#include "AddressLookupTable.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

typedef HRESULT(WINAPI *DirectInput8CreateProc)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);

void genericQueryInterface(REFIID CalledID, LPVOID * ppvObj);
extern AddressLookupTableDinput8<void> ProxyAddressLookupTableDinput8;

extern bool LostWindowFocus;
extern HWND CooperativeLevelWindow;

#include "IDirectInput8A.h"
#include "IDirectInputDevice8A.h"
#include "IDirectInputEffect.h"
