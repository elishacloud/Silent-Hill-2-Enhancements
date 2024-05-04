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
extern "C" HRESULT WINAPI DSOAL_DirectSoundCreate8(LPCGUID lpcGUID, IDirectSound8 * *ppDS, IUnknown * pUnkOuter);

void WINAPI genericQueryInterface(REFIID riid, LPVOID * ppvObj);

// Volume array falloff
constexpr LONG VolumeArray[] = { -10000, -2401, -1799, -1411, -1170, -961, -809, -665, -555, -445, -359, -270, -198, -124, -63, 0 };

extern std::vector<m_IDirectSoundBuffer8*> BufferList;
extern AddressLookupTableDsound<void> ProxyAddressLookupTableDsound;

#include "IDirectSound8.h"
#include "IDirectSoundBuffer8.h"
