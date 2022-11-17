#pragma once

#include "Settings.h"

#define VISIT_BGM_FILES(visit) \
	visit(bgm_001, adx, L"\\sound\\adx\\apart") \
	visit(bgm_003, adx, L"\\sound\\adx\\apart") \
	visit(bgm_014, adx, L"\\sound\\adx\\apart") \
	visit(bgm_101, aix, L"\\sound\\adx\\apart") \
	visit(bgm_102, aix, L"\\sound\\adx\\apart") \
	visit(bgm_103, aix, L"\\sound\\adx\\apart") \
	visit(bgm_118, aix, L"\\sound\\adx\\apart") \
	visit(bgm_125, aix, L"\\sound\\adx\\apart") \
	visit(bgm_002, adx, L"\\sound\\adx\\end") \
	visit(bgm_012, adx, L"\\sound\\adx\\end") \
	visit(bgm_022, adx, L"\\sound\\adx\\forest") \
	visit(bgm_114_a, adx, L"\\sound\\adx\\forest") \
	visit(bgm_114_b, aix, L"\\sound\\adx\\forest") \
	visit(bgm_115, aix, L"\\sound\\adx\\forest") \
	visit(bgm_016, adx, L"\\sound\\adx\\hospital") \
	visit(bgm_021, adx, L"\\sound\\adx\\hospital") \
	visit(bgm_100, aix, L"\\sound\\adx\\hospital") \
	visit(bgm_105, aix, L"\\sound\\adx\\hospital") \
	visit(bgm_106, aix, L"\\sound\\adx\\hospital") \
	visit(bgm_111, aix, L"\\sound\\adx\\hospital") \
	visit(bgm_119, aix, L"\\sound\\adx\\hospital") \
	visit(bgm_123, aix, L"\\sound\\adx\\hospital") \
	visit(bgm_007, adx, L"\\sound\\adx\\hotel") \
	visit(bgm_009, adx, L"\\sound\\adx\\hotel") \
	visit(bgm_017, adx, L"\\sound\\adx\\hotel") \
	visit(bgm_108, aix, L"\\sound\\adx\\hotel") \
	visit(bgm_112, aix, L"\\sound\\adx\\hotel") \
	visit(bgm_112_ng, aix, L"\\sound\\adx\\hotel") \
	visit(bgm_113, aix, L"\\sound\\adx\\hotel") \
	visit(bgm_121, aix, L"\\sound\\adx\\hotel") \
	visit(bgm_122, aix, L"\\sound\\adx\\hotel") \
	visit(bgm_124, aix, L"\\sound\\adx\\hotel") \
	visit(bgm_126, adx, L"\\sound\\adx\\hotel") \
	visit(bgm_104, aix, L"\\sound\\adx\\mansion") \
	visit(bgm_015, adx, L"\\sound\\adx\\prison") \
	visit(bgm_020, adx, L"\\sound\\adx\\prison") \
	visit(bgm_107, aix, L"\\sound\\adx\\prison") \
	visit(bgm_109, aix, L"\\sound\\adx\\prison") \
	visit(bgm_110, aix, L"\\sound\\adx\\prison") \
	visit(bgm_116, aix, L"\\sound\\adx\\prison") \
	visit(bgm_120, aix, L"\\sound\\adx\\prison") \
	visit(bgm_128, aix, L"\\sound\\adx\\prison") \
	visit(bgm_004, adx, L"\\sound\\adx\\town") \
	visit(bgm_005, adx, L"\\sound\\adx\\town") \
	visit(bgm_018, adx, L"\\sound\\adx\\town") \
	visit(bgm_117, aix, L"\\sound\\adx\\town") \
	visit(voice, afs, L"\\sound\\adx\\voice")

template<typename T>
bool isInString(T strCheck, T str, size_t size);

char* GetFileModPath(const char* sh2, const char* str);
wchar_t* GetFileModPath(const wchar_t* sh2, const wchar_t* str);

LPCSTR GetModPath(LPCSTR);
LPCWSTR GetModPath(LPCWSTR);

void InstallCreateProcessHooks();
void InstallFileSystemHooks();
