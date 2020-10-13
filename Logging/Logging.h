#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "External\Logging\Logging.h"

#ifdef GUID_DEFINED
std::ostream& operator<<(std::ostream& os, REFIID riid);
#endif
