#pragma once

#include <dsound.h>

class m_IDirectSound8;
class m_IDirectSoundBuffer8;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include "AddressLookupTable.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

typedef HRESULT(WINAPI *DirectSoundCreate8Proc)(LPCGUID, LPDIRECTSOUND8*, LPUNKNOWN);

void WINAPI genericQueryInterface(REFIID riid, LPVOID * ppvObj);

extern bool ds_threadExit;
extern AddressLookupTableDsound<void> ProxyAddressLookupTableDsound;
extern std::vector<m_IDirectSoundBuffer8*> LastStopped;

#include "IDirectSound8.h"
#include "IDirectSoundBuffer8.h"
