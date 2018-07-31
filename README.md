# Silent Hill 2 Enhancement
### Introduction
A project designed to enhance Silent Hill 2 (SH2) graphics and audio for the PC. It also includes scripts to build or modify SH2 audio files (SFX, BGM and Dialog).

### Features
Below is a list of features:

 * [**Widescreen Fix**](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/sh2) - Allows the game to fit any sized monitor appropriately and fixes other inherit game bugs.
 * [**Nemesis2000 Fog Fix**](http://ps2wide.net/pc.html#sh2) - Makes the game's fog closer resemble the PlayStation 2's version which is considered the best.
 * **PS2 Style Noise Filter** - Makes the noise filter resemble the PlayStation 2's noise filter.
 * **Increase Draw Distance** - Fixes distant hallway walls (such as those in the Woodside Apartments) from suddenly appearing.  This makes them appear gradually, more naturally.
 * **Cemetery Lighting Fix** - Fixes an issue where wrong data is used when saving the game in the cemetery, which can corrupt fog effects.
 * **Rowboat Animation Fix** - Fixes an issue with rowboat animation if you exit to the main menu and reload the game.
 * **Catacomb's Meat Cold Room Fix** - Updates the color and lighting of the catacomb's meat cold rooms to be more like the PS2 version of the game.
 * **SFX Address Fix** - Dynamically updates SH2 memory with correct index locations for the SFX from the `sddata.bin` file (required if you are using a modified version of the `sddata.bin` audio file).
 * **ASI Loader** - Loads custom libraries with the file extension .asi into the game using [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader).
 * **WndMode** - Runs the game in windowed mode.
 * [**d3d8to9**](https://github.com/crosire/d3d8to9) - Converts Silent Hill 2 to use Direct3D9 (d3d9.dll) rather than Direct3D8.
 * **NoCD Patch** - Disables the CD check.
 * **Reset Screen Res** - Resets the display adapter on exit which fixes an issue when using WineD3D casing the screen to go dark.
 * [**modupdater**](https://github.com/ThirteenAG/modupdater) - Automatically updates the module when new versions comes out.
 * [**Audio Script Builder**](AudioScripts) - Allows you to create or update the SH2 audio files.

### Silent Hill 2 Enhanced Edition Installation Guide
To learn more, check out the [Silent Hill 2 Enhanced Edition Installation Guide](http://www.enhanced.townofsilenthill.com/SH2/) webpage.

### Donations

All my work here is free and can be freely used.  For more details on how you can use this module see the [license](#license) section below.  However, if you would like to donate to me then check out my [donations page](https://PayPal.me/elishacloud).  All donations are for work already completed!  Please don't donate for future work or to try and increase my development speed.  Thanks!

Note: these donation only go to me, not to anyone else who helped with this project.  To donate to others check out the [credits page](http://www.enhanced.townofsilenthill.com/SH2/credit.htm).

### License
Copyright (C) 2018 Elisha Riedlinger

This software is provided 'as-is', without any express or implied warranty. In no event will the author(s) be held liable for any damages arising from the use of this software. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

This project uses code from several other projects. Below is a list of locations that source code was taken from:

 * **adx2aix**: Used to multiplex multiple ADX files into a single AIX file.
 * **adxencd**: Used to encode ADX files from raw WAV files.
 * **aix2adx**: Used to convert the AIX files into ADX format.
 * [AFS Packer](https://www.romhacking.net/utilities/843/) By PacoChan: Used to create the 'voice.afs' file.
 * [Aqrit's ddwrapper](http://bitpatch.com/ddwrapper.html): Includes code to read the ini config file.
 * [d3d8to9](https://github.com/crosire/d3d8to9): Includes the full Direct3D 8 to Direct3D 9 code.
 * [DSoundCtrl](https://github.com/nRaecheR/DirectSoundControl): Includes code from DirectSoundControl for GetOSVersion and GetVersionReg functions.
 * [DxWnd](https://sourceforge.net/projects/dxwnd/): Includes code from DxWnd for API hooking, DxWnd proxy loading (init.cpp) and exception handling.
 * [DxWrapper](https://github.com/elishacloud/dxwrapper): Includes code to create wrapper dlls and resets the display adapter.
 * [GetComputerManufacturer](http://www.rohitab.com/discuss/topic/35915-win32-api-to-get-system-information/): Includes code from rohitab.com to get the computer vendor and model.
 * [GetFileVersionInfo ](https://stackoverflow.com/a/940743): Includes code from stackoverflow.com for getting the version of a PE file.
 * [GetFunctionAddress](http://www.rohitab.com/discuss/topic/40594-parsing-pe-export-table/): Includes code from rohitab.com to parse the PE export table.
 * [MemoryModule](https://github.com/fancycode/MemoryModule): Includes code for loading libraries from memory.
 * [modupdater](https://github.com/ThirteenAG/modupdater): Includes the full binary file for the modupdater.
 * [Nemesis2000 Fog Fix](http://ps2wide.net/pc.html#sh2): Includes the full binary file for fog fix by Nemesis2000.
 * [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader): Includes code for loading ASI plugins and code for loading modules from a module resource.
 * [Widescreen Fix](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/sh2): Includes the full binary file for the Widescreen Fix by [ThirteenAG](https://github.com/ThirteenAG) and [AeroWidescreen](https://github.com/AeroWidescreen).

### Development
This project is written in C++ using Microsoft Visual Studio Community 2015.

The project uses the Windows 10 SDK and WDK. The exact version required can be seen in the project properties in Visual Studio.

Thanks for stopping by!