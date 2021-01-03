#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "External\Logging\Logging.h"

typedef enum _D3DERR { } D3DERR;
typedef enum _DIERR { } DIERR;
typedef enum _DSERR { } DSERR;

std::ostream& operator<<(std::ostream& os, const D3DERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const DIERR& ErrCode);
std::ostream& operator<<(std::ostream& os, const DSERR& ErrCode);
#ifdef GUID_DEFINED
std::ostream& operator<<(std::ostream& os, REFIID riid);
#endif
