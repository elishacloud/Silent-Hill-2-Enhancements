# Silent Hill 2 Enhancement
### Introduction
A project designed to enhance Silent Hill 2 (SH2) graphics and audio for the PC. It also includes scripts to build or modify SH2 audio files (SFX, BGM and Dialog).

### Features
Below is a list of features:

 * **Anisotropic Filtering** - Enable or disable anisotropic filtering.
 * [**Audio Script Builder**](AudioScripts) - Allows you to create or update the SH2 audio files.
 * **ASI Loader** - Loads custom libraries with the file extension .asi into the game using [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader).
 * **Black Pillar Box Fix** - Forces all dynamically made letterboxing and pillarboxing to be black in color.
 * **Borderless Windowed Mode** - Enable or disable windows border. Requires Windowed Mode to be enabled (WndMode = 1).
 * **Catacomb's Meat Cold Room Fix** - Updates the color and lighting of the catacomb's meat cold rooms to be more like the PS2 version of the game.
 * **Cemetery Lighting Fix** - Fixes an issue where wrong data is used when saving the game in the cemetery, which can corrupt fog effects.
 * **Chainsaw Spawn Fix** - Prevents the chainsaw from spawning on a first playthrough, which is a developer-intended design choice.
 * **Closet Cutscene Fix** - Adjusts visuals during the apartment closet cutscene to resemble the PS2 version.
 * **Creature Vehicle Spawn Fix** - Fixes an issue where creatures would incorrectly exit from underneath a vehicle.
 * **Custom Hi-Res Font** - Loads custom font texture `font000.tga` and `fontwdata.bin` as width data for the first 224 chars.
 * **Custom Mod Folder Support** - Enables a custom mod folder `sh2e` to store modified game files so as to not overwrite native Silent Hill 2 files.
 * [**d3d8to9**](https://github.com/crosire/d3d8to9) - Converts Silent Hill 2 to use Direct3D9 (d3d9.dll) rather than Direct3D8.
 * **Disable Red Cross** - Disables the low health (red cross) indicator completely. This option is recommended only if you are using a controller with vibration enabled.
 * **Disable Red Cross In Cutscenes** - Hides the low health (red cross) indicator during in-game cutscenes.
 * **DisableGameUX** - Disables the Microsoft Game Explorer (GameUX) to prevent rundll32.exe high CPU.
 * **Display Noise Filter** - Makes the noise filter resemble the PlayStation 2's noise filter.
 * **DPad Movement Fix** - Allows movement with D-pad on DirectInput and XInput gamepads.
 * **Effects Flicker Fix** - Removes the black flicker that appears at the beginning of post-processing effects.
 * **Flashlight Brightness** - Adjusts flashlight brightness to resemble the PS2 version. Reduces flashlight intensity for environments, but keeps enemies and NPCs bright.
 * **Fog 2D Fix** - Fixes an issue on Nvidia graphics cards where the 2D fog is missing.
 * **Fog Fix** - Makes the game's fog closer resemble the PlayStation 2's version which is considered the best.  Based on [Nemesis2000 Fog Fix](http://ps2wide.net/pc.html#sh2).
 * **Fog Parameter Fix** - Adjusts the fog-of-war boundaries for specific areas to fix visual errors.
 * **Fullscreen Windowed Mode** - Enables fullscreen windowed mode. Requires Windowed Mode to be enabled (WndMode = 1).
 * **Game Load Fix** - Disables free-saving in a few rooms that would cause game issues upon file loading back into the rooms.
 * **Halogen Light Fix** - Fixes the prison hallway halogen lights
 * **Hang On Pause Fix** - Fixes an issue where the game will hang when Esc is pressed while transition is active.
 * **Hospital Chase Fix** - Correctly syncs an attack animation to the rest of the cutscene that plays out during the Hospital chase.
 * **Hotel Water Fix** - Restores lighting values for the hotel water.
 * **Improved Storage Support** - Allows you to save the game when you have more than 2 TB of free space.
 * **Increase Blood** - Increases the blood pool size of dead enemies to better match the PS2 version.
 * **Increase Draw Distance** - Fixes distant hallway walls (such as those in the Woodside Apartments) from suddenly appearing.  This makes them appear gradually, more naturally.
 * **Left-handed Joystick Support** - Swaps left and right joystick functions. Useful for left-handed players.
 * **Lighting Transition Fix** - Makes lighting transition smooth from one light source to another for a few particular areas.
 * **Joystick Camera Movement** - Sets right joystick mode for search camera movement on controllers.
 * **Missing Wall Chunks Fix** - Fixes an issue on Nvidia graphics cards where wall chunks are missing in some locations.
 * [**modupdater**](https://github.com/ThirteenAG/modupdater) - Automatically updates the module when new versions comes out.
 * **Multi-Language Support** - Restores the language selection in the Options menu.
 * **NoCD Patch** - Disables the CD check. _Note: not yet supported in all game versions._
 * **Piston Room Fix** - Hides a piston behind a door that should not be seen during a cutscene.
 * **Reset Screen Res** - Resets the display adapter on exit which fixes an issue when using WineD3D casing the screen to go dark.
 * **Room 312 Pause Menu Fix** - Restores the noise filter and bloom effects in the pause menu for Room 312.
 * **Room 312 Shadow Fix** - Prevents distracting shadow flickering while in Room 312 of the Hotel.
 * **Rowboat Animation Fix** - Fixes an issue with rowboat animation if you exit to the main menu and reload the game.
 * **SFX Address Fix** - Dynamically updates SH2 memory with correct index locations for the SFX from the `sddata.bin` file (required if you are using a modified version of the `sddata.bin` audio file).
 * **Soft Shadow Support** - Adds soft shadows, shadow level intensities, shadow fading on flashlight toggles, and self shadows.
 * **Special FX Fix** - Restores post-processing effects, which includes depth-of-field, motion blur, and pseudo blooms.
 * **Texture Address Fix** - Dynamically updates SH2 memory to reserve additional space for large texture (required if you are using a large texture files).
 * **Town West Gate Event Fix** - Changes James' commentary about the back alley Heaven's Night gate at night to properly reflect the gate's status.
 * **UAC Control** - Checks if administrator access is required for the game to function correctly and prompts for UAC if needed.
 * **Vibration Support** - Enables force feedback vibration support for XInput and DirectInput gamepads.
 * **White Shader Fix** - Fixes an issue on Nvidia graphics cards where certain textures would appear as white when they should be black.
 * [**Widescreen Fix**](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/sh2) - Allows the game to fit any sized monitor appropriately and fixes other inherit game bugs.
 * **Windowed Mode** - Runs the game in windowed mode.
 * **Woodside Apartment Object Fix** - Fixes spawning placements for objects in Woodside Apartments Room 205.

### Configuration
To view an ini example see the [settings.ini](https://github.com/elishacloud/Silent-Hill-2-Enhancements/blob/master/Common/Settings.ini) file.

For more details on how to configure the module see the [Configuration Details](http://www.enhanced.townofsilenthill.com/SH2/config.htm) webpage.

### Silent Hill 2 Enhanced Edition Installation Guide
To learn more, check out the [Silent Hill 2 Enhanced Edition Installation Guide](http://www.enhanced.townofsilenthill.com/SH2/) webpage.

### Donations

All my work here is free and can be freely used.  For more details on how you can use this module see the [license](#license) section below.  However, if you would like to donate to me then check out my [donations page](https://PayPal.me/elishacloud).  All donations are for work already completed!  Please don't donate for future work or to try and increase my development speed.  Thanks!

Note: these donation only go to me, not to anyone else who helped with this project.  To donate to others check out the [credits page](http://www.enhanced.townofsilenthill.com/SH2/credit.htm).

### License
Copyright (C) 2020 Elisha Riedlinger

This software is provided 'as-is', without any express or implied warranty. In no event will the author(s) be held liable for any damages arising from the use of this software. Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

This project uses code from several other projects. Below is a list of locations that source code was taken from:

 * **adx2aix**: Used to multiplex multiple ADX files into a single AIX file.
 * **adxencd**: Used to encode ADX files from raw WAV files.
 * **aix2adx**: Used to convert the AIX files into ADX format.
 * [AFS Packer](https://www.romhacking.net/utilities/843/): By PacoChan. Used to create the 'voice.afs' file.
 * [Aqrit's ddwrapper](http://bitpatch.com/ddwrapper.html): Includes code to read the ini config file.
 * [d3d8to9](https://github.com/crosire/d3d8to9): Includes the full Direct3D 8 to Direct3D 9 code.
 * [DSoundCtrl](https://github.com/nRaecheR/DirectSoundControl): Includes code from DirectSoundControl for GetOSVersion and GetVersionReg functions.
 * [DxWnd](https://sourceforge.net/projects/dxwnd/): Includes code from DxWnd for proxy loading (init.cpp) and exception handling.
 * [DxWrapper](https://github.com/elishacloud/dxwrapper): Includes code to create wrapper dlls and resets the display adapter.
 * [GetComputerManufacturer](http://www.rohitab.com/discuss/topic/35915-win32-api-to-get-system-information/): Includes code from rohitab.com to get the computer vendor and model.
 * [GetFileVersionInfo](https://stackoverflow.com/a/940743): Includes code from stackoverflow.com for getting the version of a PE file.
 * [md5 hash](http://www.zedwood.com/article/cpp-md5-function): Includes code for computing md5 hash.
 * [MemoryModule](https://github.com/fancycode/MemoryModule): Includes code for loading libraries from memory.
 * [modupdater](https://github.com/ThirteenAG/modupdater): Includes the full binary file for the modupdater.
 * [Nemesis2000 Fog Fix](http://ps2wide.net/pc.html#sh2): Includes code created by reviewing the Nemesis2000 fog fix.
 * [ReShade](https://github.com/crosire/reshade): Includes code from ReShade for supporting custom shaders.
 * [SPIRV](https://github.com/KhronosGroup/SPIRV-Headers): Includes code from SPIRV.
 * [stb](https://github.com/nothings/stb): Includes code from stb.
 * [utfcpp](https://github.com/nemtrif/utfcpp): Includes code from utfcpp.
 * [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader): Includes code for loading ASI plugins and code for loading modules from a module resource.
 * [Widescreen Fix](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/sh2): Includes code from the Widescreen Fix by [ThirteenAG](https://github.com/ThirteenAG) and [AeroWidescreen](https://github.com/AeroWidescreen).

### Development
This project is written in C++ using Microsoft Visual Studio Community 2017.

The project uses the Windows 10 SDK, WDK and [DirectX9 SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812). The exact version required can be seen in the project properties in Visual Studio.

Thanks for stopping by!
