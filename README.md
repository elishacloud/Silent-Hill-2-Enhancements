# Silent Hill 2 Enhancement ASI
### Introduction
This is a project that is designed to enhance Silent Hill 2.  So far it mainly focuses on enhancing the audio (SFX, BMG and Dialog).  But we hope to do more soon.

### Features
Below is a list of features:

 - Dynamically updates SH2 memory with correct index locations for the SFX from the sddata.bin file, required if you want to modify the sddata.bin file
 - Resets the display adapter on exit to fix issue when using WineD3D casing the screen to go dark

### Installation

1. Download the latest binary release from the repository's Release page and unzip it to the 'scripts' folder.  If you don't have a scripts folder than check out the Silent Hill 2 [widescreen installation Guide](http://widescreenfix.townofsilenthill.com/SH2/).
2. (optional) Edit the 'sh2-enhce.ini' config file for the settings you desire.

### Uninstallation

Delete the 'sh2-enhce.asi' and 'sh2-enhce.ini' files. You can also delete the log file, if there is one.

### License
Copyright (C) 2017 Elisha Riedlinger

This software is provided 'as-is', without any express or implied warranty. In no event will the author(s) be held liable for any damages arising from the use of this software. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

This project uses code from several other projects. Below is a list of locations that source code was taken from:

 - [DxWrapper](https://github.com/elishacloud/dxwrapper): Includes code to resets the display adapter.
 - [Aqrit's ddwrapper](http://bitpatch.com/ddwrapper.html): Includes code to read the ini config file.
 - [Winning Eleven AFSExplorer](http://www.theisozone.com/downloads/playstation/tools/afs-explorer-var-37-afsexplorer-v37/): Used to update the 'voice.asf' file.
 - **adxencd**: Used to encode ADX files from raw WAV files.
 - **adx2aix**: Used to multiplex multiple ADX files into a single AIX file.
 - **aix2adx**: Used to convert the ADX files into WAV format.

Thanks for stopping by!