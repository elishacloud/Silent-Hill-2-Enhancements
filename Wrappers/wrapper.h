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
	bool ValidProcAddress(FARPROC ProcAddress);
	void ShimProc(FARPROC &var, FARPROC in, FARPROC &out);
	HMODULE CreateWrapper();
}

#define VISIT_DLLS(visit) \
	visit(d3d8) \
	visit(dinput8) \
	visit(dsound)

// Wrappers
#include "d3d8.h"
#include "dinput8.h"
#include "dsound.h"

#define DECLARE_FORWARD_FUNCTIONS(procName, unused) \
	extern "C" void __stdcall procName();

#define DECLARE_PROC_VARIABLES(procName, unused) \
	extern FARPROC procName ## _var;

namespace d3d8
{
	VISIT_PROCS_D3D8(DECLARE_PROC_VARIABLES);
}

extern FARPROC p_Direct3DCreate8Wrapper;
