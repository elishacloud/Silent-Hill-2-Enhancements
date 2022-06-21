#pragma once

#include "BuildNo.rc"

#define IDC_MYICON                      2
#define IDD_CONFIG_DIALOG               102
#define IDM_ABOUT                       104
#define IDI_CONFIG                      107
#define IDI_SMALL                       108
#define IDR_MAINFRAME                   128
#define IDC_COMBO1                      1000
#define IDC_STATIC                      -1

// Included resource files
#define IDR_CONFIG_XML                 901

// Main resource file details
#define APP_NAME				"SH2EE Configuration Tool"
#define APP_MAJOR				1
#define APP_MINOR				8
#define APP_BUILDNUMBER			BUILD_NUMBER
#define APP_REVISION			0
#define APP_COMPANYNAME			"Sadrate Presents"
#define APP_DESCRPTION			"SH2EE Configuration Tool"
#define APP_COPYRIGHT			"Gemini (C) 2022"
#define APP_ORIGINALVERSION		"SH2EEconfig.exe"
#define APP_INTERNALNAME		"Launcher"

// Get APP_VERSION
#define _TO_STRING_(x) #x
#define _TO_STRING(x) _TO_STRING_(x)
#define APP_VERSION _TO_STRING(APP_MAJOR) "." _TO_STRING(APP_MINOR) "." _TO_STRING(APP_BUILDNUMBER) "." _TO_STRING(APP_REVISION)
#define VERSION_NUMBER APP_MAJOR, APP_MINOR, APP_BUILDNUMBER, APP_REVISION

// Shared identifiers
#define IDR_D3DX9_TOOLS					0
