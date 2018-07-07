# Silent Hill 2 Audio Enhancement Pack Builder
### Introduction
This is a project contains three modules that are designed to build the enhanced Silent Hill 2 audio files.  There are three sets of audio files:
* [Sound effects (SFX)](SFX)
* [Dialog or voice](Dialog)
* [Background music / sounds (BGM)](BGM)

### Overview
This project was initially designed to build the Silent Hill 2 audio files for the Audio Enhancement Pack, but it will also allow you to modify or update any of the audio files in Silent Hill 2.  There is a script for each type of audio file in Silent Hill 2 that allows you to create the audio files in a format acceptable for Silent Hill 2.  All of the scripts require wav files for input.

#### Sound Effects (SFX):
The SFX files in Silent Hill 2 on the PC are stored in the `Silent Hill 2\data\sound\sddata.bin` file.  This file is a very simple concatenation of wav files.  There are 417 SFX files and all of these are very short, most are 1 to 2 seconds.  See the readme file in the [SFX](SFX) folder for more details.

#### Dialog:
The dialogue files in Silent Hill 2 on the PC are stored in the `Silent Hill 2\data\sound\adx\voice\voice.afs` file.  The afs format is just a container format (like an ISO) and stores all the dialog entries.  The dialog files need to be in a specific order in the afs file and have a specific name.  All of them have the extension adx, but the actual content can be in either adx or wav format.  See the readme file in the [Dialog](Dialog) folder for more details.

#### Background Music (BGM):
The BGM files in Silent Hill 2 on the PC are stored in multiple adx and aix format files which are all located in the `Silent Hill 2\data\sound\adx` folder and associated subfolders.  adx is a compressed  audio format used mostly in console video games.  aix is a multiplexed file that includes several adx file streams.  Just like the Dialog files, Silent Hill 2 supports both wav and adx for these files.  However since all of these files need to loop and Silent Hill 2 does not support looping wav files these need to be in the lower quality adx format.  See the readme file in the [BGM](BGM) folder for more details.

### Silent Hill 2 Enhanced Edition
To learn more, check out the [Silent Hill 2 Enhanced Edition Installation Guide](http://www.enhanced.townofsilenthill.com/SH2/) webpage.

### License
Copyright (C) 2018 Elisha Riedlinger

This software is provided 'as-is', without any express or implied warranty. In no event will the author(s) be held liable for any damages arising from the use of this software. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

This project uses code from several other projects. Below is a list of locations that source code was taken from:

 * [AFS Packer](https://www.romhacking.net/utilities/843/) By PacoChan: Used to create the 'voice.afs' file.
 * **adxencd**: Used to encode ADX files from raw WAV files.
 * **adx2aix**: Used to multiplex multiple ADX files into a single AIX file.
 * **aix2adx**: Used to convert the AIX files into ADX format.

Thanks for stopping by!
