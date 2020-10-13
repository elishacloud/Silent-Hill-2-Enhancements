/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
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

#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <vector>
#include <string>
#include <unordered_map>

inline void trim(std::string &str, const char chars[] = " \t\n\r")
{
	str.erase(0, str.find_first_not_of(chars));
	str.erase(str.find_last_not_of(chars) + 1);
}
inline std::string trim(const std::string &str, const char chars[] = " \t\n\r")
{
	std::string res(str);
	trim(res, chars);
	return res;
}

namespace reshade
{
	class ini_file
	{
	public:
		explicit ini_file();
		~ini_file();

		// Checks whether the specified "section" and "key" currently exist in Settings.
		bool has(const std::string &section, const std::string &key) const
		{
			const auto it1 = _sections.find(section);
			if (it1 == _sections.end())
				return false;
			const auto it2 = it1->second.find(key);
			if (it2 == it1->second.end())
				return false;
			return true;
		}

		// Gets the value of the specified "section" and "key" from the Settings.
		template <typename T>
		bool get(const std::string &section, const std::string &key, T &value) const
		{
			const auto it1 = _sections.find(section);
			if (it1 == _sections.end())
				return false;
			const auto it2 = it1->second.find(key);
			if (it2 == it1->second.end())
				return false;
			value = convert<T>(it2->second, 0);
			return true;
		}
		template <typename T, size_t SIZE>
		bool get(const std::string &section, const std::string &key, T(&values)[SIZE]) const
		{
			const auto it1 = _sections.find(section);
			if (it1 == _sections.end())
				return false;
			const auto it2 = it1->second.find(key);
			if (it2 == it1->second.end())
				return false;
			for (size_t i = 0; i < SIZE; ++i)
				values[i] = convert<T>(it2->second, i);
			return true;
		}
		template <typename T>
		bool get(const std::string &section, const std::string &key, std::vector<T> &values) const
		{
			const auto it1 = _sections.find(section);
			if (it1 == _sections.end())
				return false;
			const auto it2 = it1->second.find(key);
			if (it2 == it1->second.end())
				return false;
			values.resize(it2->second.size());
			for (size_t i = 0; i < it2->second.size(); ++i)
				values[i] = convert<T>(it2->second, i);
			return true;
		}

		// Returns 'true' only if the specified "section" and "key" exists and is not zero.
		bool get(const std::string &section, const std::string &key) const
		{
			bool value = false;
			return get<bool>(section, key, value) && value;
		}

		// Sets the value of the specified "section" and "key" to a new "value".
		template <typename T>
		void set(const std::string &section, const std::string &key, const T &value)
		{
			set(section, key, std::to_string(value));
		}
		template <>
		void set(const std::string &section, const std::string &key, const bool &value)
		{
			set<std::string>(section, key, value ? "1" : "0");
		}
		template <>
		void set(const std::string &section, const std::string &key, const std::string &value)
		{
			auto &v = _sections[section][key];
			v.assign(1, value);
			_modified = true;
		}
		void set(const std::string &section, const std::string &key, std::string &&value)
		{
			auto &v = _sections[section][key];
			v.resize(1);
			v[0] = std::forward<std::string>(value);
			_modified = true;
		}
		template <typename T, size_t SIZE>
		void set(const std::string &section, const std::string &key, const T(&values)[SIZE], const size_t size = SIZE)
		{
			auto &v = _sections[section][key];
			v.resize(size);
			for (size_t i = 0; i < size; ++i)
				v[i] = std::to_string(values[i]);
			_modified = true;
		}
		template <>
		void set(const std::string &section, const std::string &key, const std::vector<std::string> &values)
		{
			auto &v = _sections[section][key];
			v = values;
			_modified = true;
		}
		void set(const std::string &section, const std::string &key, std::vector<std::string> &&values)
		{
			auto &v = _sections[section][key];
			v = std::forward<std::vector<std::string>>(values);
			_modified = true;
		}

		// Gets the settings from cache.
		static reshade::ini_file &load_cache();
		void reset_config();

	private:
		void load();

		template <typename T>
		static const T convert(const std::vector<std::string> &values, size_t i) = delete;
		template <>
		static const bool convert(const std::vector<std::string> &values, size_t i)
		{
			return convert<int>(values, i) != 0 || i < values.size() && (values[i] == "true" || values[i] == "True" || values[i] == "TRUE");
		}
		template <>
		static const int convert(const std::vector<std::string> &values, size_t i)
		{
			return static_cast<int>(convert<long>(values, i));
		}
		template <>
		static const unsigned int convert(const std::vector<std::string> &values, size_t i)
		{
			return static_cast<unsigned int>(convert<unsigned long>(values, i));
		}
		template <>
		static const long convert(const std::vector<std::string> &values, size_t i)
		{
			return i < values.size() ? std::strtol(values[i].c_str(), nullptr, 10) : 0l;
		}
		template <>
		static const unsigned long convert(const std::vector<std::string> &values, size_t i)
		{
			return i < values.size() ? std::strtoul(values[i].c_str(), nullptr, 10) : 0ul;
		}
		template <>
		static const long long convert(const std::vector<std::string> &values, size_t i)
		{
			return i < values.size() ? std::strtoll(values[i].c_str(), nullptr, 10) : 0ll;
		}
		template <>
		static const unsigned long long convert(const std::vector<std::string> &values, size_t i)
		{
			return i < values.size() ? std::strtoull(values[i].c_str(), nullptr, 10) : 0ull;
		}
		template <>
		static const float convert(const std::vector<std::string> &values, size_t i)
		{
			return static_cast<float>(convert<double>(values, i));
		}
		template <>
		static const double convert(const std::vector<std::string> &values, size_t i)
		{
			return i < values.size() ? std::strtod(values[i].c_str(), nullptr) : 0.0;
		}
		template <>
		static const std::string convert(const std::vector<std::string> &values, size_t i)
		{
			return i < values.size() ? values[i] : std::string();
		}

		// Describes a single value from Settings.
		using value = std::vector<std::string>;
		// Describes a section of multiple key/value pairs in Settings.
		using section = std::unordered_map<std::string, value>;

		bool _modified = false;
		std::unordered_map<std::string, section> _sections;
	};
}
