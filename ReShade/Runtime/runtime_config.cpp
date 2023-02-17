/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
* Copyright (C) 2023 Elisha Riedlinger
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

#define RESHADE_FILE_LIST
#include "runtime.hpp"
#include "runtime_config.hpp"
#include "runtime_objects.hpp"
#include "Resource.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>

extern DWORD GammaLevel;

struct {
	bool loaded = false;
	reshade::ini_file cache;
}g_ini_cache;

reshade::ini_file::ini_file() { load(); }
reshade::ini_file::~ini_file() {}

void reshade::ini_file::load()
{
	if (g_ini_cache.loaded)
	{
		return;
	}

	_sections.clear();
	_modified = false;

	std::string GammaSection("[GammaLevel" + std::to_string(GammaLevel) + "]");

	std::string file, section, line;
	g_ini_cache.loaded = read_resource(IDR_RESHADE_INI, file);
	std::istringstream s_file(file.c_str());
	while (std::getline(s_file, line))
	{
		trim(line);

		if (line.empty() || line[0] == ';' || line[0] == '/' || line[0] == '#')
		{
			continue;
		}

		if (line.compare(GammaSection) == 0)
		{
			line.assign("[" + GammaEffectName + ".fx]");
		}

		// Read section name
		if (line[0] == '[')
		{
			section = trim(line.substr(0, line.find(']')), " \t[]");
			continue;
		}

		// Read section content
		const auto assign_index = line.find('=');
		if (assign_index != std::string::npos)
		{
			const std::string key = trim(line.substr(0, assign_index));
			const std::string value = trim(line.substr(assign_index + 1));

			// Append to key if it already exists
			reshade::ini_file::value &elements = _sections[section][key];
			for (size_t offset = 0, base = 0, len = value.size(); offset <= len;)
			{
				// Treat ",," as an escaped comma and only split on single ","
				const size_t found = std::min(value.find_first_of(',', offset), len);
				if (found + 1 < len && value[found + 1] == ',')
				{
					offset = found + 2;
				}
				else
				{
					std::string &element = elements.emplace_back();
					element.reserve(found - base);

					while (base < found)
					{
						const char c = value[base++];
						element += c;

						if (c == ',' && base < found && value[base] == ',')
						{
							base++; // Skip second comma in a ",," escape sequence
						}
					}

					base = offset = found + 1;
				}
			}
		}
		else
		{
			_sections[section].insert({ line, {} });
		}
	}
}

void reshade::ini_file::reset_config()
{
	g_ini_cache.loaded = false;
	g_ini_cache.cache.load();
}

reshade::ini_file &reshade::ini_file::load_cache()
{
	if (!g_ini_cache.loaded)
	{
		g_ini_cache.cache.load();
	}
	return g_ini_cache.cache;
}
