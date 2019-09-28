/**
* Copyright (C) 2019 Elisha Riedlinger
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
#include "Common\md5.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

constexpr char *GoodHashes[] = {
	{ "c65baee155f3f47b4538e67ff9073d25" },
	{ "1be408eca5e437e9d7f63461e2cb9e69" },
	{ "1e797657100d0b6e6de23cffec94a331" },
	{ "957b9349b0dd8a48e64e6d7218b7522c" },
	{ "b2d4a1f0cff2542f9f856150cd770a16" },
	{ "f3f5360298b6d0f6f2e09cf24a0925d1" },
	{ "c16ed8436fdcfc53a07ab21723994770" },
};

// Get number of textures in array
constexpr DWORD HashNum = (sizeof(GoodHashes) / sizeof(*GoodHashes));

void ValidateBinary()
{
	wchar_t pathname[MAX_PATH];
	GetModuleFileName(nullptr, pathname, MAX_PATH);
	std::ifstream file(pathname, std::ios::in | std::ios::binary | std::ios::ate);
	std::string buffer;

	if (file.is_open())
	{
		// Get binary size
		DWORD size = (DWORD)file.tellg();
		buffer.resize(size);

		// Read binary file
		file.seekg(0, std::ios::beg);
		file.read((char*)&buffer[0], size);

		// Close binary
		file.close();
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: Could not open binary!";
		return;
	}

	// Compute hash
	std::string hash(md5(buffer));

	// Check for good binary
	for (int x = 0; x < HashNum; x++)
	{
		if (hash.compare(GoodHashes[x]) == 0)
		{
			// Found good binary
			return;
		}
	}

	Logging::Log() << "Warning: Unknown game binary hash: " << md5(buffer).c_str();
}
