/**
* Copyright (C) 2022 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

#define ADD_FARPROC_MEMBER(procName, prodAddr) \
	FARPROC procName ## _var = prodAddr;

#define CREATE_PROC_STUB(procName, unused) \
	extern "C" __declspec(naked) void __stdcall procName() \
	{ \
		__asm mov edi, edi \
		__asm jmp procName ## _var \
	}

#define	LOAD_ORIGINAL_PROC(procName, prodAddr) \
	procName ## _var = GetProcAddress(dll, #procName); \
	if (procName ## _var == nullptr) \
	{ \
		procName ## _var =  prodAddr; \
	}

#define PROC_CLASS(className, Extension, VISIT_PROCS) \
	namespace className \
	{ \
		using namespace Wrapper; \
		wchar_t *Name = L ## #className ## "." ## #Extension; \
		VISIT_PROCS(ADD_FARPROC_MEMBER); \
		VISIT_PROCS(CREATE_PROC_STUB); \
		HMODULE Load(const wchar_t *strName, const HMODULE hWrapper) \
		{ \
			wchar_t path[MAX_PATH]; \
			HMODULE dll = hWrapper; \
			if (!dll && strName && _wcsicmp(strName, Name) != 0) \
			{ \
				dll = LoadLibrary(Name); \
			} \
			if (!dll) \
			{ \
				GetSystemDirectory(path, MAX_PATH); \
				wcscat_s(path, MAX_PATH, L"\\"); \
				wcscat_s(path, MAX_PATH, Name); \
				dll = LoadLibrary(path); \
			} \
			if (dll) \
			{ \
				VISIT_PROCS(LOAD_ORIGINAL_PROC); \
			} \
			return dll; \
		} \
	}

namespace Wrapper
{
	struct wrapper_map
	{
		FARPROC Proc;
		FARPROC *val;
	};

	// Forward function declaration
	HRESULT __stdcall _jmpaddr();
	HRESULT __stdcall _jmpaddrvoid();

	// Variable declaration
	const FARPROC jmpaddr = (FARPROC)*_jmpaddr;
	const FARPROC jmpaddrvoid = (FARPROC)*_jmpaddrvoid;
}

#include "wrapper.h"

namespace Wrapper
{
	DLLTYPE dtype = DTYPE_ASI;
}

__declspec(naked) HRESULT __stdcall Wrapper::_jmpaddrvoid()
{
	__asm
	{
		retn
	}
}

__declspec(naked) HRESULT __stdcall Wrapper::_jmpaddr()
{
	__asm
	{
		mov eax, 0x80004001L	// return E_NOTIMPL
		retn 16
	}
}

#define CHECK_FOR_WRAPPER(dllName) \
	{ using namespace dllName; if (_wcsicmp(WrapperMode, Name) == 0) { dll = Load(ProxyDll, hWrapper); return dll; }}

void Wrapper::GetWrapperMode()
{
	// Get module name
	wchar_t* WrapperMode = nullptr;
	wchar_t WrapperName[MAX_PATH] = { 0 };
	if (GetModulePath(WrapperName, MAX_PATH) && wcsrchr(WrapperName, '\\'))
	{
		WrapperMode = wcsrchr(WrapperName, '\\') + 1;
	}

	std::string sWrapperType(WrapperType);
	std::wstring wWrapperType(sWrapperType.begin(), sWrapperType.end());

	// Save wrapper mode
	if (_wcsicmp(WrapperMode, dtypename[DTYPE_D3D8]) == 0 || _wcsicmp(wWrapperType.c_str(), dtypename[DTYPE_D3D8]) == 0)
	{
		dtype = DTYPE_D3D8;
	}
	else if (_wcsicmp(WrapperMode, dtypename[DTYPE_DINPUT8]) == 0 || _wcsicmp(wWrapperType.c_str(), dtypename[DTYPE_DINPUT8]) == 0)
	{
		dtype = DTYPE_DINPUT8;
	}
	else if (_wcsicmp(WrapperMode, dtypename[DTYPE_DSOUND]) == 0 || _wcsicmp(wWrapperType.c_str(), dtypename[DTYPE_DSOUND]) == 0)
	{
		dtype = DTYPE_DSOUND;
	}
	else
	{
		dtype = DTYPE_ASI;
	}
}

HMODULE Wrapper::CreateWrapper(HMODULE hWrapper)
{
	wchar_t *WrapperMode = dtypename[dtype];

	Logging::Log() << "Loading as dll: " << WrapperMode;

	if (dtype == DTYPE_ASI)
	{
		return nullptr;
	}

	// Declare vars
	HMODULE dll = nullptr;
	wchar_t *ProxyDll = nullptr;

	// Check dll name and load correct wrapper
	VISIT_DLLS(CHECK_FOR_WRAPPER);

	// Exit and return handle
	return dll;
}
