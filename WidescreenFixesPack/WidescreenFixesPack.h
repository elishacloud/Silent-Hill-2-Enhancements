#pragma once

#include <Windows.h>
#include <string>
#include <string_view>

extern HMODULE m_hModule;

void WSFInit();
int GetValue(std::string_view, std::string_view szKey, int);
