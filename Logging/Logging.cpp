/**
* Copyright (C) 2020 Elisha Riedlinger
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

#define INITGUID
#define DIRECTINPUT_VERSION 0x0800

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
#include <dinput.h>
#include <MMSystem.h>
#include <dsound.h>
#include "Logging.h"

typedef unsigned char byte;

std::ostream& operator<<(std::ostream& os, REFIID riid)
{
	UINT x = 0;
	char buffer[(sizeof(IID) * 2) + 5] = { '\0' };
	for (size_t j : {3, 2, 1, 0, 0xFF, 5, 4, 0xFF, 7, 6, 0xFF, 8, 9, 0xFF, 10, 11, 12, 13, 14, 15})
	{
		if (j == 0xFF)
		{
			buffer[x] = '-';
		}
		else
		{
			sprintf_s(buffer + x, 3, "%02X", ((byte*)&riid)[j]);
			x++;
		}
		x++;
	}

	return Logging::LogStruct(os) << buffer;
}

std::ostream& operator<<(std::ostream& os, const D3DERR& ErrCode)
{
#define VISIT_D3DERR_CODES(visit) \
	visit(D3D_OK) \
	visit(D3DERR_WRONGTEXTUREFORMAT) \
	visit(D3DERR_UNSUPPORTEDCOLOROPERATION) \
	visit(D3DERR_UNSUPPORTEDCOLORARG) \
	visit(D3DERR_UNSUPPORTEDALPHAOPERATION) \
	visit(D3DERR_UNSUPPORTEDALPHAARG) \
	visit(D3DERR_TOOMANYOPERATIONS) \
	visit(D3DERR_CONFLICTINGTEXTUREFILTER) \
	visit(D3DERR_UNSUPPORTEDFACTORVALUE) \
	visit(D3DERR_CONFLICTINGRENDERSTATE) \
	visit(D3DERR_UNSUPPORTEDTEXTUREFILTER) \
	visit(D3DERR_CONFLICTINGTEXTUREPALETTE) \
	visit(D3DERR_DRIVERINTERNALERROR) \
	visit(D3DERR_NOTFOUND) \
	visit(D3DERR_MOREDATA) \
	visit(D3DERR_DEVICELOST) \
	visit(D3DERR_DEVICENOTRESET) \
	visit(D3DERR_NOTAVAILABLE) \
	visit(D3DERR_OUTOFVIDEOMEMORY) \
	visit(D3DERR_INVALIDDEVICE) \
	visit(D3DERR_INVALIDCALL) \
	visit(D3DERR_DRIVERINVALIDCALL) \
	visit(E_NOINTERFACE) \
	visit(E_POINTER)

#define VISIT_D3DERR_RETURN(x) \
	if (ErrCode == x) \
	{ \
		return os << #x; \
	}

	VISIT_D3DERR_CODES(VISIT_D3DERR_RETURN);

	return os << Logging::hex((DWORD)ErrCode);
}

std::ostream& operator<<(std::ostream& os, const DIERR& ErrCode)
{
#define VISIT_DIERR_CODES(visit) \
	visit(DI_OK) \
	visit(DI_POLLEDDEVICE) \
	visit(DI_DOWNLOADSKIPPED) \
	visit(DI_EFFECTRESTARTED) \
	visit(DI_TRUNCATED) \
	visit(DI_SETTINGSNOTSAVED) \
	visit(DI_TRUNCATEDANDRESTARTED) \
	visit(DI_WRITEPROTECT) \
	visit(DIERR_OLDDIRECTINPUTVERSION) \
	visit(DIERR_BETADIRECTINPUTVERSION) \
	visit(DIERR_BADDRIVERVER) \
	visit(DIERR_DEVICENOTREG) \
	visit(DIERR_NOTFOUND) \
	visit(DIERR_OBJECTNOTFOUND) \
	visit(DIERR_INVALIDPARAM) \
	visit(DIERR_NOINTERFACE) \
	visit(DIERR_GENERIC) \
	visit(DIERR_OUTOFMEMORY) \
	visit(DIERR_UNSUPPORTED) \
	visit(DIERR_NOTINITIALIZED) \
	visit(DIERR_ALREADYINITIALIZED) \
	visit(DIERR_NOAGGREGATION) \
	visit(DIERR_OTHERAPPHASPRIO) \
	visit(DIERR_INPUTLOST) \
	visit(DIERR_ACQUIRED) \
	visit(DIERR_NOTACQUIRED) \
	visit(DIERR_READONLY) \
	visit(DIERR_HANDLEEXISTS) \
	visit(E_PENDING) \
	visit(DIERR_INSUFFICIENTPRIVS) \
	visit(DIERR_DEVICEFULL) \
	visit(DIERR_MOREDATA) \
	visit(DIERR_NOTDOWNLOADED) \
	visit(DIERR_HASEFFECTS) \
	visit(DIERR_NOTEXCLUSIVEACQUIRED) \
	visit(DIERR_INCOMPLETEEFFECT) \
	visit(DIERR_NOTBUFFERED) \
	visit(DIERR_EFFECTPLAYING) \
	visit(DIERR_UNPLUGGED) \
	visit(DIERR_REPORTFULL) \
	visit(DIERR_MAPFILEFAIL) \
	visit(E_NOINTERFACE) \
	visit(E_POINTER)

#define VISIT_DIERR_RETURN(x) \
	if (ErrCode == x) \
	{ \
		return os << #x; \
	}

	if (ErrCode == S_FALSE)
	{
		return os << "'DI_NOTATTACHED' or 'DI_BUFFEROVERFLOW' or 'DI_PROPNOEFFECT' or 'DI_NOEFFECT'";
	}

	VISIT_DIERR_CODES(VISIT_DIERR_RETURN);

	return os << Logging::hex((DWORD)ErrCode);
}

std::ostream& operator<<(std::ostream& os, const DSERR& ErrCode)
{
#define VISIT_DSERR_CODES(visit) \
	visit(DS_OK) \
	visit(DS_NO_VIRTUALIZATION) \
	visit(DSERR_ALLOCATED) \
	visit(DSERR_CONTROLUNAVAIL) \
	visit(DSERR_INVALIDPARAM) \
	visit(DSERR_INVALIDCALL) \
	visit(DSERR_GENERIC) \
	visit(DSERR_PRIOLEVELNEEDED) \
	visit(DSERR_OUTOFMEMORY) \
	visit(DSERR_BADFORMAT) \
	visit(DSERR_UNSUPPORTED) \
	visit(DSERR_NODRIVER) \
	visit(DSERR_ALREADYINITIALIZED) \
	visit(DSERR_NOAGGREGATION) \
	visit(DSERR_BUFFERLOST) \
	visit(DSERR_OTHERAPPHASPRIO) \
	visit(DSERR_UNINITIALIZED) \
	visit(DSERR_NOINTERFACE) \
	visit(DSERR_ACCESSDENIED) \
	visit(DSERR_BUFFERTOOSMALL) \
	visit(DSERR_DS8_REQUIRED) \
	visit(DSERR_SENDLOOP) \
	visit(DSERR_BADSENDBUFFERGUID) \
	visit(DSERR_OBJECTNOTFOUND) \
	visit(DSERR_FXUNAVAILABLE) \
	visit(E_NOINTERFACE) \
	visit(E_POINTER)

#define VISIT_DSERR_RETURN(x) \
	if (ErrCode == x) \
	{ \
		return os << #x; \
	}

	VISIT_DSERR_CODES(VISIT_DSERR_RETURN);

	return os << Logging::hex((DWORD)ErrCode);
}
