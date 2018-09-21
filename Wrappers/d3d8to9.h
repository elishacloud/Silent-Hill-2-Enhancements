#pragma once

#include "External\d3d8to9\source\d3d8to9.hpp"

void EnableD3d8to9();
Direct3D8 *WINAPI Direct3DCreate8to9(UINT SDKVersion);

const FARPROC p_Direct3DCreate8to9 = (FARPROC)*Direct3DCreate8to9;
extern FARPROC p_Direct3DCreate9;
