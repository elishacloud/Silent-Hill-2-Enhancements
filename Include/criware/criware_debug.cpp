/*
* Copyright (C) 2022 Gemini
* ===============================================================
* Debugging module
* ---------------------------------------------------------------
* Generic interface for handling error messages.
* ===============================================================
*/

#include "criware.h"

void ADXD_Error(const char* caption, const char* fmt, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, fmt);
	vsprintf_s(buf, fmt, ap);
	va_end(ap);

	MessageBoxA(nullptr, buf, caption, MB_ICONERROR);
	exit(0);
}

void ADXD_Warning(const char* caption, const char* fmt, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, fmt);
	vsprintf_s(buf, fmt, ap);
	va_end(ap);

	MessageBoxA(nullptr, buf, caption, MB_ICONEXCLAMATION);
}

void ADXD_Log(const char* fmt, ...)
{
#if _DEBUG
	va_list ap;
	char buf[256];

	va_start(ap, fmt);
	vsprintf_s(buf, fmt, ap);
	va_end(ap);

	OutputDebugStringA(buf);
#endif
}
