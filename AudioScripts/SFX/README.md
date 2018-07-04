# SFX Builder for Silent Hill 2

### Description:
The SFX files in Silent Hill 2 on the PC are stored in the `Silent Hill 2\data\sound\sddata.bin` file. This file is a very simple concatenation of wav files. There are 417 SFX files and all of these are very short, most are 1 to 2 seconds.

This project contains two tools:
* [mergebin](mergebin.cpp) - This file will take input of 417 wav files and output a single concatenated `sddata.bin` file that is ready to be used by the PC.  These files need to be located in the same folder as the tool.
* [splitbin](splitbin.cpp) - This file will take input the `sddata.bin` Silent Hill 2 file and will output 417 wav files.

**NOTE: All wav files that are used by Silent Hill 2 in the SFX file must be mono.  Silent Hill 2 cannot handle stereo SFX files.

### Audio file mapping:
You can view a partial map of the audio files and where they are used [here](SFX-Map.csv).

Thanks to [Polymega](https://github.com/Polymega) for help in creating this map!

### Looping:
Some of these files loop, but most just play once.  The files that loop have metadata added to the end, which is just a simple hex `0x00` for non-looping tracks and `0x01` for looping tracks.

The following files have looping metadata added to them when you run `mergebin.exe`:
1. `sddata350` through `sddata366`
2. `sddata416`

The following files have non-looping metadata added to them when you run `mergebin.exe`:
1. `sddata367` through `sddata393`

All other files have no metadata added to them.

**NOTE: sddata352 must be saved as 16000 Hz and sddata377 at 11025 Hz in order for them to work correctly with Silent Hill 2.

### Instructions:
To create the 'sddata.bin' file for Silent Hill 2 copy all the WAV files into a folder and run the `mergebin.exe` tool.  This tool will automatically create the `sddata.bin` file.  Then just copy the `sddata.bin` file over the top of the `sddata.bin` file in the `Silent Hill 2\data` folder.

**NOTE: you will need to enable the Silent Hill 2 Enhancement ASI plugin for the updated `sddata.bin` file to work.

1. Copy all the SFX mono WAV files into the same folder with the mergebin.exe tool.
2. Run the `mergebin.exe` tool.
3. Copy the `sddata.bin` file unto the `Silent Hill 2\data` folder overriding any files.

**IMPORTANT: Make sure to back up the original `sddata.bin` in case you run into any issues.

#### Required Files:
1. mergebin.exe
2. Silent Hill 2 SFX wav sound files: sddata[000...416].wav

#### Script Files:
	mergebin.exe
	splitbin.exe

#### Audio Files:
	sddata000.wav
	sddata001.wav
	sddata002.wav
	sddata003.wav
	sddata004.wav
	sddata005.wav
	sddata006.wav
	sddata007.wav
	sddata008.wav
	sddata009.wav
	sddata010.wav
	sddata011.wav
	sddata012.wav
	sddata013.wav
	sddata014.wav
	sddata015.wav
	sddata016.wav
	sddata017.wav
	sddata018.wav
	sddata019.wav
	sddata020.wav
	sddata021.wav
	sddata022.wav
	sddata023.wav
	sddata024.wav
	sddata025.wav
	sddata026.wav
	sddata027.wav
	sddata028.wav
	sddata029.wav
	sddata030.wav
	sddata031.wav
	sddata032.wav
	sddata033.wav
	sddata034.wav
	sddata035.wav
	sddata036.wav
	sddata037.wav
	sddata038.wav
	sddata039.wav
	sddata040.wav
	sddata041.wav
	sddata042.wav
	sddata043.wav
	sddata044.wav
	sddata045.wav
	sddata046.wav
	sddata047.wav
	sddata048.wav
	sddata049.wav
	sddata050.wav
	sddata051.wav
	sddata052.wav
	sddata053.wav
	sddata054.wav
	sddata055.wav
	sddata056.wav
	sddata057.wav
	sddata058.wav
	sddata059.wav
	sddata060.wav
	sddata061.wav
	sddata062.wav
	sddata063.wav
	sddata064.wav
	sddata065.wav
	sddata066.wav
	sddata067.wav
	sddata068.wav
	sddata069.wav
	sddata070.wav
	sddata071.wav
	sddata072.wav
	sddata073.wav
	sddata074.wav
	sddata075.wav
	sddata076.wav
	sddata077.wav
	sddata078.wav
	sddata079.wav
	sddata080.wav
	sddata081.wav
	sddata082.wav
	sddata083.wav
	sddata084.wav
	sddata085.wav
	sddata086.wav
	sddata087.wav
	sddata088.wav
	sddata089.wav
	sddata090.wav
	sddata091.wav
	sddata092.wav
	sddata093.wav
	sddata094.wav
	sddata095.wav
	sddata096.wav
	sddata097.wav
	sddata098.wav
	sddata099.wav
	sddata100.wav
	sddata101.wav
	sddata102.wav
	sddata103.wav
	sddata104.wav
	sddata105.wav
	sddata106.wav
	sddata107.wav
	sddata108.wav
	sddata109.wav
	sddata110.wav
	sddata111.wav
	sddata112.wav
	sddata113.wav
	sddata114.wav
	sddata115.wav
	sddata116.wav
	sddata117.wav
	sddata118.wav
	sddata119.wav
	sddata120.wav
	sddata121.wav
	sddata122.wav
	sddata123.wav
	sddata124.wav
	sddata125.wav
	sddata126.wav
	sddata127.wav
	sddata128.wav
	sddata129.wav
	sddata130.wav
	sddata131.wav
	sddata132.wav
	sddata133.wav
	sddata134.wav
	sddata135.wav
	sddata136.wav
	sddata137.wav
	sddata138.wav
	sddata139.wav
	sddata140.wav
	sddata141.wav
	sddata142.wav
	sddata143.wav
	sddata144.wav
	sddata145.wav
	sddata146.wav
	sddata147.wav
	sddata148.wav
	sddata149.wav
	sddata150.wav
	sddata151.wav
	sddata152.wav
	sddata153.wav
	sddata154.wav
	sddata155.wav
	sddata156.wav
	sddata157.wav
	sddata158.wav
	sddata159.wav
	sddata160.wav
	sddata161.wav
	sddata162.wav
	sddata163.wav
	sddata164.wav
	sddata165.wav
	sddata166.wav
	sddata167.wav
	sddata168.wav
	sddata169.wav
	sddata170.wav
	sddata171.wav
	sddata172.wav
	sddata173.wav
	sddata174.wav
	sddata175.wav
	sddata176.wav
	sddata177.wav
	sddata178.wav
	sddata179.wav
	sddata180.wav
	sddata181.wav
	sddata182.wav
	sddata183.wav
	sddata184.wav
	sddata185.wav
	sddata186.wav
	sddata187.wav
	sddata188.wav
	sddata189.wav
	sddata190.wav
	sddata191.wav
	sddata192.wav
	sddata193.wav
	sddata194.wav
	sddata195.wav
	sddata196.wav
	sddata197.wav
	sddata198.wav
	sddata199.wav
	sddata200.wav
	sddata201.wav
	sddata202.wav
	sddata203.wav
	sddata204.wav
	sddata205.wav
	sddata206.wav
	sddata207.wav
	sddata208.wav
	sddata209.wav
	sddata210.wav
	sddata211.wav
	sddata212.wav
	sddata213.wav
	sddata214.wav
	sddata215.wav
	sddata216.wav
	sddata217.wav
	sddata218.wav
	sddata219.wav
	sddata220.wav
	sddata221.wav
	sddata222.wav
	sddata223.wav
	sddata224.wav
	sddata225.wav
	sddata226.wav
	sddata227.wav
	sddata228.wav
	sddata229.wav
	sddata230.wav
	sddata231.wav
	sddata232.wav
	sddata233.wav
	sddata234.wav
	sddata235.wav
	sddata236.wav
	sddata237.wav
	sddata238.wav
	sddata239.wav
	sddata240.wav
	sddata241.wav
	sddata242.wav
	sddata243.wav
	sddata244.wav
	sddata245.wav
	sddata246.wav
	sddata247.wav
	sddata248.wav
	sddata249.wav
	sddata250.wav
	sddata251.wav
	sddata252.wav
	sddata253.wav
	sddata254.wav
	sddata255.wav
	sddata256.wav
	sddata257.wav
	sddata258.wav
	sddata259.wav
	sddata260.wav
	sddata261.wav
	sddata262.wav
	sddata263.wav
	sddata264.wav
	sddata265.wav
	sddata266.wav
	sddata267.wav
	sddata268.wav
	sddata269.wav
	sddata270.wav
	sddata271.wav
	sddata272.wav
	sddata273.wav
	sddata274.wav
	sddata275.wav
	sddata276.wav
	sddata277.wav
	sddata278.wav
	sddata279.wav
	sddata280.wav
	sddata281.wav
	sddata282.wav
	sddata283.wav
	sddata284.wav
	sddata285.wav
	sddata286.wav
	sddata287.wav
	sddata288.wav
	sddata289.wav
	sddata290.wav
	sddata291.wav
	sddata292.wav
	sddata293.wav
	sddata294.wav
	sddata295.wav
	sddata296.wav
	sddata297.wav
	sddata298.wav
	sddata299.wav
	sddata300.wav
	sddata301.wav
	sddata302.wav
	sddata303.wav
	sddata304.wav
	sddata305.wav
	sddata306.wav
	sddata307.wav
	sddata308.wav
	sddata309.wav
	sddata310.wav
	sddata311.wav
	sddata312.wav
	sddata313.wav
	sddata314.wav
	sddata315.wav
	sddata316.wav
	sddata317.wav
	sddata318.wav
	sddata319.wav
	sddata320.wav
	sddata321.wav
	sddata322.wav
	sddata323.wav
	sddata324.wav
	sddata325.wav
	sddata326.wav
	sddata327.wav
	sddata328.wav
	sddata329.wav
	sddata330.wav
	sddata331.wav
	sddata332.wav
	sddata333.wav
	sddata334.wav
	sddata335.wav
	sddata336.wav
	sddata337.wav
	sddata338.wav
	sddata339.wav
	sddata340.wav
	sddata341.wav
	sddata342.wav
	sddata343.wav
	sddata344.wav
	sddata345.wav
	sddata346.wav
	sddata347.wav
	sddata348.wav
	sddata349.wav
	sddata350.wav
	sddata351.wav
	sddata352.wav
	sddata353.wav
	sddata354.wav
	sddata355.wav
	sddata356.wav
	sddata357.wav
	sddata358.wav
	sddata359.wav
	sddata360.wav
	sddata361.wav
	sddata362.wav
	sddata363.wav
	sddata364.wav
	sddata365.wav
	sddata366.wav
	sddata367.wav
	sddata368.wav
	sddata369.wav
	sddata370.wav
	sddata371.wav
	sddata372.wav
	sddata373.wav
	sddata374.wav
	sddata375.wav
	sddata376.wav
	sddata377.wav
	sddata378.wav
	sddata379.wav
	sddata380.wav
	sddata381.wav
	sddata382.wav
	sddata383.wav
	sddata384.wav
	sddata385.wav
	sddata386.wav
	sddata387.wav
	sddata388.wav
	sddata389.wav
	sddata390.wav
	sddata391.wav
	sddata392.wav
	sddata393.wav
	sddata394.wav
	sddata395.wav
	sddata396.wav
	sddata397.wav
	sddata398.wav
	sddata399.wav
	sddata400.wav
	sddata401.wav
	sddata402.wav
	sddata403.wav
	sddata404.wav
	sddata405.wav
	sddata406.wav
	sddata407.wav
	sddata408.wav
	sddata409.wav
	sddata410.wav
	sddata411.wav
	sddata412.wav
	sddata413.wav
	sddata414.wav
	sddata415.wav
	sddata416.wav
