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
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

DEFINE_GUID(IID_GetProxyInterface, 0x00000000, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);
DEFINE_GUID(IID_GetRenderTarget, 0x11111111, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);
DEFINE_GUID(IID_GetReplacedInterface, 0x22222222, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);
DEFINE_GUID(IID_SetReplacedInterface, 0x33333333, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);
DEFINE_GUID(IID_ClearRenderTarget, 0x44444444, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);
DEFINE_GUID(IID_SetTextureRenderTarget, 0x55555555, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0xff, 0xff);

typedef IDirect3D8 *(WINAPI *Direct3DCreate8Proc)(UINT);

void genericQueryInterface(REFIID riid, LPVOID *ppvObj, m_IDirect3DDevice8* m_pDevice);

DWORD GetBitCount(D3DFORMAT Format);
void UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow, bool SetWindow);
void UpdatePresentParameterForMultisample(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DMULTISAMPLE_TYPE MultiSampleType);
void GetDesktopRes(LONG &screenWidth, LONG &screenHeight);
void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight);
DWORD WINAPI SaveScreenshotFile(LPVOID pvParam);

#define D3DRS_ADAPTIVETESS_Y 181

extern bool ClassReleaseFlag;
extern HWND DeviceWindow;
extern LONG BufferWidth, BufferHeight;
extern bool CopyRenderTarget;
extern bool SetSSAA;
extern D3DMULTISAMPLE_TYPE DeviceMultiSampleType;
extern DWORD MaxAnisotropy;

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
