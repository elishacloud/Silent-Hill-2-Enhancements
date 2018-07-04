# Silent Hill 2 Enhancement ASI
### Introduction
This is a project that is designed to enhance Silent Hill 2.  So far it mainly focuses on enhancing the audio (SFX, BGM and Dialog).  But we hope to do more soon.

### Features
Below is a list of features:

 * Dynamically updates SH2 memory with correct index locations for the SFX from the `sddata.bin` file, required if you are using a modified the `sddata.bin` file
 * ASI loader to load custom libraries with the file extension .asi into game processes using [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)
 * Conversion of Direct3D 8 (d3d8.dll) to Direct3D 9 (d3d9.dll) using [d3d8to9](https://github.com/crosire/d3d8to9)
 * Disables the CD check
 * Resets the display adapter on exit to fix issue when using WineD3D casing the screen to go dark
 * Includes [scripts](AudioScripts) to allow you to create or update the SH2 audio files

### Silent Hill 2 Widescreen Installation Guide
To learn more, check out the [Silent Hill 2 Widescreen Installation Guide](http://www.enhanced.townofsilenthill.com/SH2/) webpage.

### License
Copyright (C) 2018 Elisha Riedlinger

This software is provided 'as-is', without any express or implied warranty. In no event will the author(s) be held liable for any damages arising from the use of this software. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

This project uses code from several other projects. Below is a list of locations that source code was taken from:

 * [DxWrapper](https://github.com/elishacloud/dxwrapper): Includes code to create wrapper dlls and resets the display adapter.
 * [Aqrit's ddwrapper](http://bitpatch.com/ddwrapper.html): Includes code to read the ini config file.
 * [d3d8to9](https://github.com/crosire/d3d8to9): Includes the full Direct3D 8 to Direct3D 9 code.
 * [DxWnd](https://sourceforge.net/projects/dxwnd/): Includes code from DxWnd for API hooking, DxWnd proxy loading (init.cpp) and exception handling.
 * [GetFunctionAddress](http://www.rohitab.com/discuss/topic/40594-parsing-pe-export-table/): Includes code from rohitab.com to parse the PE export table.
 * [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader): Includes code for loading ASI plugins.
 * [AFS Packer](https://www.romhacking.net/utilities/843/) By PacoChan: Used to create the 'voice.afs' file.
 * **adxencd**: Used to encode ADX files from raw WAV files.
 * **adx2aix**: Used to multiplex multiple ADX files into a single AIX file.
 * **aix2adx**: Used to convert the AIX files into ADX format.

Thanks for stopping by!