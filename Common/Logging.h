#pragma once

#include <fstream>
#include <stdarg.h> 

class Log
{
public:
	Log() {}
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

	static std::ofstream LOG;
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
