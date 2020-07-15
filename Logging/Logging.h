#pragma once

#include "External\Logging\Logging.h"

#ifdef GUID_DEFINED
std::ostream& operator<<(std::ostream& os, REFIID riid);
#endif
