#pragma once

#define INITGUID

#include "DirectX81SDK\include\d3d8.h"
#include "DirectX81SDK\include\d3d8types.h"
#include "DirectX81SDK\include\d3d8caps.h"

class m_IDirect3D8;
class m_IDirect3DDevice8;
class m_IDirect3DCubeTexture8;
class m_IDirect3DIndexBuffer8;
class m_IDirect3DSurface8;
class m_IDirect3DSwapChain8;
class m_IDirect3DTexture8;
class m_IDirect3DVertexBuffer8;
class m_IDirect3DVolume8;
class m_IDirect3DVolumeTexture8;

#include "AddressLookupTable.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

typedef IDirect3D8 *(WINAPI *Direct3DCreate8Proc)(UINT);

void UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow, bool SetWindow);
void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight);

extern HWND DeviceWindow;
extern LONG BufferWidth, BufferHeight;

#include "IDirect3D8.h"
#include "IDirect3DDevice8.h"
#include "IDirect3DCubeTexture8.h"
#include "IDirect3DIndexBuffer8.h"
#include "IDirect3DSurface8.h"
#include "IDirect3DSwapChain8.h"
#include "IDirect3DTexture8.h"
#include "IDirect3DVertexBuffer8.h"
#include "IDirect3DVolume8.h"
#include "IDirect3DVolumeTexture8.h"
