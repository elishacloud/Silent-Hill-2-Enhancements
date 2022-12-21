#pragma once

#include "BuildNo.rc"

// Included resource files
#define OIC_SH2_ICON                    151

#define IDR_RESHADE_INI                 101
#define IDR_SETTINGS_INI                102

#define IDR_SMAA_FX                     201
#define IDR_COLORGRAD_FX                202
#define IDR_GAMMAGAIN_FX                203
#define IDR_PIRATEBLOOM_FX              204
#define IDR_FRUTBUNN_FX                 205
#define IDR_LOTTES_FX                   206
#define IDR_REFRESH_FX                  207

#define IDR_SEARCHTEX_DDS               301
#define IDR_AREATEX_DDS                 302

#define IDR_D3DX9_DLL                   401
#define IDR_D3DCOMPI43_DLL              402
#define IDR_D3DCOMPILE_DLL              403

#define IDR_LANG_RES_JA                 501
#define IDR_LANG_RES_EN                 502
#define IDR_LANG_RES_FR                 503
#define IDR_LANG_RES_DE                 504
#define IDR_LANG_RES_IT                 505
#define IDR_LANG_RES_ES                 506

// Main resource file details
#define APP_NAME				"Silent Hill 2: Enhanced Edition"
#define APP_MAJOR				2
#define APP_MINOR				0
#define APP_BUILDNUMBER			BUILD_NUMBER
#define APP_REVISION			0
#define APP_COMPANYNAME			"Sadrate Presents"
#define APP_DESCRPTION			"A project designed to enhance Silent Hill 2 (SH2) graphics and audio for the PC."
#define APP_COPYRIGHT			"Copyright (C) 2022 Elisha Riedlinger"
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
#define VERSION_NUMBER APP_MAJOR, APP_MINOR, APP_BUILDNUMBER, APP_REVISION

#ifdef RESHADE_FILE_LIST

#include "Common\Settings.h"

namespace
{
	struct FILELIST {
		bool* enabled;
		DWORD value;
		std::string name;
	};

	std::string GammaEffectName = "LiftGammaGain";

	std::vector<FILELIST> shaderList{
		{ &EnableSMAA, IDR_SMAA_FX, "SMAA.fx" },
		{ &AdjustColorTemp, IDR_COLORGRAD_FX, "MinimalColorGrading.fx" },
		{ &RestoreBrightnessSelector, IDR_GAMMAGAIN_FX, "LiftGammaGain.fx" },
		{ &EnableCRTShader, IDR_PIRATEBLOOM_FX, "PirateBloom.fx" },
		{ &CRTNonCurveShader, IDR_FRUTBUNN_FX, "FrutbunnNonCurve.fx" },
		{ &CRTCurveShader, IDR_FRUTBUNN_FX, "FrutbunnCurve.fx" },
		{ &EnableCRTShader, IDR_LOTTES_FX, "Lottes.fx" },
		{ &EnableCRTShader, IDR_REFRESH_FX, "Refresh.fx" },
	};

	std::vector<FILELIST> textureList{
		{ nullptr, IDR_SEARCHTEX_DDS, "SearchTex.dds" },
		{ nullptr, IDR_AREATEX_DDS, "AreaTex.dds" },
	};
}

#endif
