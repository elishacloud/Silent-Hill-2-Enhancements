#pragma once

#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#include <windows.h>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <array>
#include <math.h>
#include <subauth.h>
#include <thread>
#include <mutex>
#include <set>
#include <map>
#include <iomanip>
#include "WidescreenFixesPack\WidescreenFixesPack.h"
#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\calling.hpp"
#include "External\injector\include\injector\hooking.hpp"
#ifdef _M_IX86
#include "External\injector\include\injector\assembly.hpp"
#endif
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include <filesystem>

#define CEXP
#define DllMain WSFMain
#define Init WSFInit

bool IsUALPresent();
std::tuple<int32_t, int32_t> GetDesktopRes();
std::string format(const char* fmt, ...);

template<typename T>
std::array<uint8_t, sizeof(T)> to_bytes(const T& object)
{
    std::array<uint8_t, sizeof(T)> bytes;
    const uint8_t* begin = reinterpret_cast<const uint8_t*>(std::addressof(object));
    const uint8_t* end = begin + sizeof(T);
    std::copy(begin, end, std::begin(bytes));
    return bytes;
}

template<typename T>
T& from_bytes(const std::array<uint8_t, sizeof(T)>& bytes, T& object)
{
    static_assert(std::is_trivially_copyable<T>::value, "not a TriviallyCopyable type");
    uint8_t* begin_object = reinterpret_cast<uint8_t*>(std::addressof(object));
    std::copy(std::begin(bytes), std::end(bytes), begin_object);
    return object;
}

template<class T, class T1>
T from_bytes(const T1& bytes)
{
    static_assert(std::is_trivially_copyable<T>::value, "not a TriviallyCopyable type");
    T object;
    uint8_t* begin_object = reinterpret_cast<uint8_t*>(std::addressof(object));
    std::copy(std::begin(bytes), std::end(bytes) - (sizeof(T1) - sizeof(T)), begin_object);
    return object;
}

template <size_t n>
std::string pattern_str(const std::array<uint8_t, n> bytes)
{
    std::string result;
    for (size_t i = 0; i < n; i++)
    {
        result += format("%02X ", bytes[i]);
    }
    return result;
}

template <typename T>
std::string pattern_str(T t)
{
    return std::string((std::is_same<T, char>::value ? format("%c ", t) : format("%02X ", t)));
}

template <typename T, typename... Rest>
std::string pattern_str(T t, Rest... rest)
{
    return std::string((std::is_same<T, char>::value ? format("%c ", t) : format("%02X ", t)) + pattern_str(rest...));
}

template<size_t N>
constexpr size_t length(char const (&)[N])
{
    return N - 1;
}

template <typename T, typename V>
bool iequals(const T& s1, const V& s2)
{
    T str1(s1); T str2(s2);
    std::transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
    std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
    return (str1 == str2);
}

template <typename T>
inline std::wstring int_to_hex(T val, size_t width = sizeof(T) * 2)
{
    std::wstringstream ss;
    ss << std::uppercase << std::setfill(L'0') << std::setw(width) << std::hex << (val | 0);
    return ss.str();
}

class CallbackHandler
{
public:
	static inline void RegisterCallback(std::function<void()>&&) {}
	static inline void RegisterCallback(std::wstring_view, std::function<void()>&&) {}
	static inline void RegisterCallback(std::function<void()>&&, hook::pattern) {}

	static inline void RegisterCallback(std::function<void()>&&, bool, ptrdiff_t offset = 0x1100, uint32_t* ptr = nullptr)
	{
		UNREFERENCED_PARAMETER(offset);
		UNREFERENCED_PARAMETER(ptr);
	}

public:
	static inline std::once_flag flag;
};

class CIniReader
{
private:
	std::string m_szFileName;

public:
	CIniReader() {}
	CIniReader(std::string_view) {}
	CIniReader(std::stringstream&) {}

	const std::string& GetIniPath()
	{
		if (!m_szFileName.size())
		{
			// Get config file path
			char configpath[MAX_PATH];
			GetModuleFileNameA(m_hModule, configpath, MAX_PATH);
			strcpy_s(strrchr(configpath, '.'), MAX_PATH - strlen(configpath), ".ini");
			m_szFileName.assign(configpath);
		}
		return m_szFileName;
	}

	bool ReadBoolean(std::string_view szSection, std::string_view szKey, bool bolDefaultValue)
	{
		return (GetValue(szSection, szKey, bolDefaultValue) != 0);
	}

	int ReadInteger(std::string_view szSection, std::string_view szKey, int iDefaultValue)
	{
		return GetValue(szSection, szKey, iDefaultValue);
	}

	void WriteBoolean(std::string_view, std::string_view, bool, bool useparser = false)
	{
		UNREFERENCED_PARAMETER(useparser);
	}

	void WriteInteger(std::string_view, std::string_view, int, bool useparser = false)
	{
		UNREFERENCED_PARAMETER(useparser);
	}
};
