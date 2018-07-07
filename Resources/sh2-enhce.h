#pragma once

#include "BuildNo.rc"

// Included resource files
#define IDR_SH2FOG   101
#define IDR_SH2WID   102
#define IDR_SH2WND   103

// Main resource file details
#define APP_NAME				"Silent Hill 2 Enhancement Module"
#define APP_MAJOR				1
#define APP_MINOR				2
#define APP_BUILDNUMBER			BUILD_NUMBER
#define APP_REVISION			0
#define APP_COMPANYNAME			"Sadrate Presents"
#define APP_DESCRPTION			"A project designed to enhance Silent Hill 2 (SH2) graphics and audio for the PC."
#define APP_COPYRIGHT			"Copyright (C) 2018 Elisha Riedlinger"
#define APP_ORIGINALVERSION		"d3d8.dll"
#define APP_INTERNALNAME		"sh2-enhce"

// Get APP_VERSION
#define _TO_STRING_(x) #x
#define _TO_STRING(x) _TO_STRING_(x)
#define APP_VERSION _TO_STRING(APP_MAJOR) "." _TO_STRING(APP_MINOR) "." _TO_STRING(APP_BUILDNUMBER) "." _TO_STRING(APP_REVISION)
