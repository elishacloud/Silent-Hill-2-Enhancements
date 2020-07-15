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

#include <windows.h>
#include "Logging.h"

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
