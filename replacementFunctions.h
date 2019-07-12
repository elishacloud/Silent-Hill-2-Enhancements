#pragma once
#include <Windows.h>
#include <cstdint>

struct CUSTOMVERTEX
{
	FLOAT x, y, z, rhw;
	DWORD color;
};

struct CUSTOMVERTEX_UV
{
	FLOAT x, y, z, rhw;
	FLOAT u, v;
};

void parentShadowFunc(uint32_t arg1/*, uint32_t arg2*/);
void drawDynamicShadows();
void softShadows(int screenWidth, int screenHeight);
