#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <MMSystem.h>
#include <dsound.h>

class m_IDirectSound8;
class m_IDirectSoundBuffer8;

#include "AddressLookupTable.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

typedef HRESULT(WINAPI *DirectSoundCreate8Proc)(LPCGUID, LPDIRECTSOUND8*, LPUNKNOWN);

void WINAPI genericQueryInterface(REFIID riid, LPVOID * ppvObj);

extern AddressLookupTableDsound<void> ProxyAddressLookupTableDsound;

#include "IDirectSound8.h"
#include "IDirectSoundBuffer8.h"
