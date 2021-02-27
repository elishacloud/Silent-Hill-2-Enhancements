#pragma once

#include "BuildNo.rc"

// Included resource files
#define IDR_RESHADE_INI                 101

#define IDR_SMAA_FX                     201
#define IDR_COLORGRAD_FX                202
#define IDR_GAMMAGAIN_FX                203

#define IDR_SEARCHTEX_DDS               301
#define IDR_AREATEX_DDS                 302

#define IDR_D3DX9_TOOLS                 401

// Main resource file details
#define APP_NAME				"Silent Hill 2: Enhanced Edition"
#define APP_MAJOR				1
#define APP_MINOR				7
#define APP_BUILDNUMBER			BUILD_NUMBER
#define APP_REVISION			0
#define APP_COMPANYNAME			"Sadrate Presents"
#define APP_DESCRPTION			"A project designed to enhance Silent Hill 2 (SH2) graphics and audio for the PC."
#define APP_COPYRIGHT			"Copyright (C) 2021 Elisha Riedlinger"
#define APP_ORIGINALVERSION		"d3d8.dll"
#define APP_INTERNALNAME		"sh2-enhce"

/////////////////////////////////////////////////////////////////////////////
//
// ReShade
//
#define RESHADE_DATE "2020-10-03"
#define RESHADE_TIME "01:58:54"
#define RESHADE_FULL 4.5.3.797
#define RESHADE_MAJOR 4
#define RESHADE_MINOR 5
#define RESHADE_REVISION 3
#define RESHADE_BUILD 797
#define RESHADE_STRING_FILE "4.5.3.797"
#define RESHADE_STRING_PRODUCT "4.5.3 UNOFFICIAL"
/////////////////////////////////////////////////////////////////////////////

// Get APP_VERSION
#define _TO_STRING_(x) #x
#define _TO_STRING(x) _TO_STRING_(x)
#define APP_VERSION _TO_STRING(APP_MAJOR) "." _TO_STRING(APP_MINOR) "." _TO_STRING(APP_BUILDNUMBER) "." _TO_STRING(APP_REVISION)

#ifdef RESHADE_FILE_LIST

namespace
{
	struct FILELIST {
		DWORD value;
		std::string name;
	};

	std::string SMAAEffectName = "SMAA";
	std::string BrightnessEffectName = "MinimalColorGrading";
	std::string GammaEffectName = "LiftGammaGain";

	std::vector<FILELIST> shaderList{
		{ IDR_SMAA_FX, "SMAA.fx" },
		{ IDR_COLORGRAD_FX, "MinimalColorGrading.fx" },
		{ IDR_GAMMAGAIN_FX, "LiftGammaGain.fx" },
	};

	std::vector<FILELIST> textureList{
		{ IDR_SEARCHTEX_DDS, "SearchTex.dds" },
		{ IDR_AREATEX_DDS, "AreaTex.dds" },
	};
}

#endif
