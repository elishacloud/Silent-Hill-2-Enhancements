#pragma once

#define INITGUID

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>

class m_IDirect3D9;
class m_IDirect3DDevice9;
class m_IDirect3DSwapChain9;

#include "Logging\Logging.h"
#include "Common\Settings.h"

typedef int(WINAPI *Direct3D9EnableMaximizedWindowedModeShimProc)(BOOL);
typedef IDirect3D9 *(WINAPI *Direct3DCreate9Proc)(UINT);

int WINAPI rs_Direct3D9EnableMaximizedWindowedModeShim(BOOL mEnable);
IDirect3D9 *WINAPI rs_Direct3DCreate9(UINT SDKVersion);

#include "IDirect3D9.h"
#include "IDirect3DDevice9.h"
#include "IDirect3DSwapChain9.h"
#include "runtime_d3d9.hpp"
#include "ReShade\Runtime\runtime_config.hpp"
#include "ReShade\Runtime\runtime_objects.hpp"

void Initd3d9();

extern Direct3D9EnableMaximizedWindowedModeShimProc m_pDirect3D9EnableMaximizedWindowedModeShim;
extern Direct3DCreate9Proc m_pDirect3DCreate9;
