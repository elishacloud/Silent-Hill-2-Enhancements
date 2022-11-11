#pragma once

typedef enum _DLLTYPE
{
	DTYPE_ASI		= 0,
	DTYPE_D3D8		= 1,
	DTYPE_DINPUT8	= 2,
	DTYPE_DSOUND	= 3
} DLLTYPE;

// Designated Initializer does not work in VS 2015 so must pay attention to the order
static constexpr wchar_t* dtypename[] = {
	L"sh2-enhce.asi",	// 0
	L"d3d8.dll",		// 1
	L"dinput8.dll",		// 2
	L"dsound.dll",		// 3
};
static constexpr int dtypeArraySize = (sizeof(dtypename) / sizeof(*dtypename));

namespace Wrapper
{
	extern DLLTYPE dtype;
	void GetWrapperMode();
	HMODULE CreateWrapper(HMODULE hWrapper);
}

#define VISIT_DLLS(visit) \
	visit(d3d8) \
	visit(dinput8) \
	visit(dsound)

// Wrappers
#include "d3d8.h"
#include "dinput8.h"
#include "dsound.h"

typedef struct IUnknown *LPUNKNOWN;
typedef struct IDirect3D8 *LPDIRECT3D8;
typedef struct IDirectSound8 *LPDIRECTSOUND8;

HMODULE GetD3d8ScriptDll();
void HookDirect3DCreate8(HMODULE ScriptDll);
void HookDirectInput8Create();
void HookDirectSoundCreate8();
IDirect3D8 *WINAPI Direct3DCreate8Wrapper(UINT SDKVersion);
HRESULT WINAPI DirectInput8CreateWrapper(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter);
HRESULT WINAPI DirectSoundCreate8Wrapper(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);

#define DECLARE_FORWARD_FUNCTIONS(procName, unused) \
	extern "C" void __stdcall procName();

#define DECLARE_PROC_VARIABLES(procName, unused) \
	extern FARPROC procName ## _var;

namespace d3d8
{
	VISIT_PROCS_D3D8(DECLARE_PROC_VARIABLES);
}
