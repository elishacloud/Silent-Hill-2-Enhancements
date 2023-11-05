#pragma once

#define INITGUID

#include "DirectX81SDK\include\d3d8.h"
#include "DirectX81SDK\include\d3dx8.h"
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

IDirect3D8 *WINAPI Direct3DCreate8Wrapper(UINT SDKVersion);

void genericQueryInterface(REFIID riid, LPVOID *ppvObj, m_IDirect3DDevice8* m_pDevice);

DWORD GetBitCount(D3DFORMAT Format);
void UpdatePresentParameter(D3DPRESENT_PARAMETERS* pPresentationParameters, HWND hFocusWindow, bool SetWindow);
void UpdatePresentParameterForMultisample(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DMULTISAMPLE_TYPE MultiSampleType);
void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight);
DWORD WINAPI SaveScreenshotFile(LPVOID pvParam);

#define D3DRS_ADAPTIVETESS_Y (D3DRENDERSTATETYPE)181

#define ATI_VENDOR_ID		0x1002	/* ATI Technologies Inc.			*/
#define NVIDIA_VENDOR_ID	0x10DE	/* NVIDIA Corporation				*/
#define MATROX_VENDOR_ID	0x102B	/* Matrox Electronic Systems Ltd.	*/
#define _3DFX_VENDOR_ID		0x121A	/* 3dfx Interactive Inc.			*/
#define S3_VENDOR_ID		0x5333	/* S3 Graphics Co., Ltd.			*/
#define INTEL_VENDOR_ID		0x8086	/* Intel Corporation				*/

// Transparent Supersample
#define FOURCC_SSAA         (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')

// Transparent Multisample
#define FOURCC_A2M_ENABLE   (D3DFORMAT)MAKEFOURCC( 'A', '2', 'M', '1' )
#define FOURCC_A2M_DISABLE  (D3DFORMAT)MAKEFOURCC( 'A', '2', 'M', '0' )
#define FOURCC_ATOC         (D3DFORMAT)MAKEFOURCC( 'A', 'T', 'O', 'C' )

extern HWND DeviceWindow;
extern LONG BufferWidth, BufferHeight;
extern bool DeviceLost;
extern bool ClassReleaseFlag;
extern bool CopyRenderTarget;
extern bool SetSSAA;
extern bool SetATOC;
extern D3DMULTISAMPLE_TYPE DeviceMultiSampleType;
extern bool TakeScreenShot;

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
