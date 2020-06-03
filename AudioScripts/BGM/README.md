# BGM Builder for Silent Hill 2

### Description:
The BGM files in Silent Hill 2 on the PC are stored in multiple adx and aix format files which are all located in the `Silent Hill 2\data\sound\adx` folder and associated subfolders. adx is a compressed audio format used mostly in console video games. aix is a multiplexed file that includes several adx file streams. Just like the Dialog files, Silent Hill 2 supports both wav and adx for these files. However since all of these files need to loop and Silent Hill 2 does not support looping wav files these need to be in the lower quality adx format.

### Audio file mapping:
You can view a partial map of the audio files and where they are used [here](BGM-Map.csv).

Thanks to [Polymega](https://github.com/Polymega) for help in creating this map!

### Multiplexed files:

The AIX files can include multiple ADX files in them.  For example `bgm_115.aix` includes the following files in it:
* bgm_115_0000.adx
* bgm_115_0001.adx
* bgm_115_0002.adx
* bgm_115_0003.adx
* bgm_115_0004.adx

** NOTE: All of the audio files in an AIX multiplex file needs to be the exact same length, format and have the exact same audio metadata in them for them to work correctly with Silent Hill 2.  In addition, ADX metadata is not supported with multiplexed AIX files in Silent Hill 2.

### Looping:
All of the BGM files in Silent Hill 2 should loop.  To do this requires modification of the adx file metadata.  The `adxencd.exe` tool has two parameters to handle this.  The first is `-lps` and the second is `-lpe`.  `-lps` stands for "loop start" and indicates the first audio [sample](https://en.wikipedia.org/wiki/Sampling_(signal_processing)) of the area in the file where you want to loop.  `-lpe` stands for "loop end" and indicates the last audio sample that you want to include in the loop.  A loop can start or end anywhere in the wav file

The `Build-Dialog-Files.bat` file should be modified with the loop parameters before running it so that you ensure that these files loop correctly.

Here is an example:
```
adxencd bgm_001.wav -lps3241390 -lpe7106605
```

In this example the loop starts at audio sample 3241390 and ends at audio sample 7106605.

### Instructions:
To create the ADX and AIX files for Silent Hill 2 copy all the WAV files into a folder, copy and run the `Build-BGM-Files.bat` tool.  This tool will create all the ADX and AIX.  It will also create a folder call `sound` and put all the files in their correct folders under `sound`.  Just copy the `sound` folder over the top of the `sound` folder in the `Silent Hill 2\data` folder.

1. Copy all the BGM WAV files into a folder.
2. Copy `adxencd.exe`, `adx2aix.exe` and `Build-Dialog-Files.bat` files into the same folder with the WAV files.
3. Run the `Build-BGM-Files.bat` script.
4. Copy the new `sound` folder created by the `Build-BGM-Files.bat` script unto the `Silent Hill 2\data` folder overriding any files.

**IMPORTANT: Make sure to back up the original `sound` folder in case you run into any issues.

#### Required Files:
1. adxencd.exe
2. adx2aix.exe
3. Build-BGM-Files.bat
4. Silent Hill 2 dialog wav sound files: bgm_[001...128][0000...0007].wav

#### Script Files:
	adx2aix.exe
	adxencd.exe
	aix2adx.exe
	Build-BGM-Files.bat

#### Audio Files:
	bgm_001.wav
	bgm_002.wav
	bgm_003.wav
	bgm_004.wav
	bgm_005.wav
	bgm_007.wav
	bgm_009.wav
	bgm_012.wav
	bgm_014.wav
	bgm_015.wav
	bgm_016.wav
	bgm_017.wav
	bgm_018.wav
	bgm_020.wav
	bgm_021.wav
	bgm_022.wav
	bgm_100_0000.wav
	bgm_100_0001.wav
	bgm_100_0002.wav
	bgm_100_0003.wav
	bgm_100_0004.wav
	bgm_100_0005.wav
	bgm_100_0006.wav
	bgm_101_0000.wav
	bgm_101_0001.wav
	bgm_101_0002.wav
	bgm_101_0003.wav
	bgm_102_0000.wav
	bgm_102_0001.wav
	bgm_102_0002.wav
	bgm_102_0003.wav
	bgm_102_0004.wav
	bgm_102_0005.wav
	bgm_102_0006.wav
	bgm_103_0000.wav
	bgm_103_0001.wav
	bgm_104_0000.wav
	bgm_104_0001.wav
	bgm_105_0000.wav
	bgm_105_0001.wav
	bgm_105_0002.wav
	bgm_105_0003.wav
	bgm_105_0004.wav
	bgm_105_0005.wav
	bgm_106_0000.wav
	bgm_106_0001.wav
	bgm_106_0002.wav
	bgm_106_0003.wav
	bgm_107_0000.wav
	bgm_107_0001.wav
	bgm_107_0002.wav
	bgm_107_0003.wav
	bgm_107_0004.wav
	bgm_107_0005.wav
	bgm_108_0000.wav
	bgm_108_0001.wav
	bgm_108_0002.wav
	bgm_108_0003.wav
	bgm_108_0004.wav
	bgm_108_0005.wav
	bgm_109_0000.wav
	bgm_109_0001.wav
	bgm_109_0002.wav
	bgm_109_0003.wav
	bgm_109_0004.wav
	bgm_109_0005.wav
	bgm_110_0000.wav
	bgm_110_0001.wav
	bgm_110_0002.wav
	bgm_110_0003.wav
	bgm_110_0004.wav
	bgm_110_0005.wav
	bgm_111_0000.wav
	bgm_111_0001.wav
	bgm_111_0002.wav
	bgm_111_0003.wav
	bgm_111_0004.wav
	bgm_111_0005.wav
	bgm_111_0006.wav
	bgm_112_0000.wav
	bgm_112_0001.wav
	bgm_112_0002.wav
	bgm_112_0003.wav
	bgm_112_0004.wav
	bgm_112_0005.wav
	bgm_112_0006.wav
	bgm_112_NG_0000.wav
	bgm_112_NG_0001.wav
	bgm_112_NG_0002.wav
	bgm_112_NG_0003.wav
	bgm_112_NG_0004.wav
	bgm_112_NG_0005.wav
	bgm_112_NG_0006.wav
	bgm_113_0000.wav
	bgm_113_0001.wav
	bgm_113_0002.wav
	bgm_113_0003.wav
	bgm_113_0004.wav
	bgm_113_0005.wav
	bgm_113_0006.wav
	bgm_114_a.wav
	bgm_114_b_0000.wav
	bgm_114_b_0001.wav
	bgm_114_b_0002.wav
	bgm_114_b_0003.wav
	bgm_114_b_0004.wav
	bgm_114_b_0005.wav
	bgm_115_0000.wav
	bgm_115_0001.wav
	bgm_115_0002.wav
	bgm_115_0003.wav
	bgm_115_0004.wav
	bgm_116_0000.wav
	bgm_116_0001.wav
	bgm_116_0002.wav
	bgm_116_0003.wav
	bgm_117_0000.wav
	bgm_117_0001.wav
	bgm_118_0000.wav
	bgm_118_0001.wav
	bgm_118_0002.wav
	bgm_118_0003.wav
	bgm_118_0004.wav
	bgm_119_0000.wav
	bgm_119_0001.wav
	bgm_119_0002.wav
	bgm_119_0003.wav
	bgm_119_0004.wav
	bgm_120_0000.wav
	bgm_120_0001.wav
	bgm_120_0002.wav
	bgm_120_0003.wav
	bgm_120_0004.wav
	bgm_121_0000.wav
	bgm_121_0001.wav
	bgm_122_0000.wav
	bgm_122_0001.wav
	bgm_122_0002.wav
	bgm_122_0003.wav
	bgm_122_0004.wav
	bgm_123_0000.wav
	bgm_123_0001.wav
	bgm_124_0000.wav
	bgm_124_0001.wav
	bgm_125_0000.wav
	bgm_125_0001.wav
	bgm_125_0002.wav
	bgm_126.wav
	bgm_128_0000.wav
	bgm_128_0001.wav
