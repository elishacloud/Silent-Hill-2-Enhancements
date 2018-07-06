#pragma once

#include <fstream>
#include <stdarg.h> 

void LogOSVersion();
void LogComputerManufacturer();
void LogVideoCard();

static std::ostream& operator<<(std::ostream& os, const wchar_t* wchr)
{
	std::wstring ws(wchr);
	return os << std::string(ws.begin(), ws.end()).c_str();
}

extern std::ofstream LOG;

class Log
{
public:
	Log()
	{
		SYSTEMTIME st = {};
		GetLocalTime(&st);

		char time[100];
		sprintf_s(time, "%02hu:%02hu:%02hu.%03hu ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		LOG << time;
	}
	~Log()
	{
		if (LOG.is_open())
		{
			LOG << std::endl;
		}
	}

	template <typename T>
	Log& operator<<(const T& t)
	{
		if (LOG.is_open())
		{
			LOG << t;
		}
		return *this;
	}
};

static void logf(char * fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	auto size = vsnprintf(nullptr, 0, fmt, ap);
	std::string output(size + 1, '\0');
	vsprintf_s(&output[0], size + 1, fmt, ap);
	Log() << output.c_str();
	va_end(ap);
}
