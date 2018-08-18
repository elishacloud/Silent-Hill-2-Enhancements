#pragma once

#define INITGUID

#include "d3d8.h"
#include "d3d8types.h"
#include "d3d8caps.h"
#include "LookupTable.h"
#include "Common\Settings.h"
#include "Common\Logging.h"

typedef IDirect3D8 *(WINAPI *Direct3DCreate8Proc)(UINT);

void AdjustWindow(HWND MainhWnd, LONG displayWidth, LONG displayHeight);

extern HWND DeviceWindow;
extern UINT BufferWidth, BufferHeight;

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
