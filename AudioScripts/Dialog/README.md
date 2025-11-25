# Dialog Builder for Silent Hill 2

### Description:
The dialogue files in Silent Hill 2 on the PC are stored in the `Silent Hill 2\data\sound\adx\voice\voice.afs` file. The afs format is just a container format (like an ISO) and stores all the dialog entries. The dialog files need to be in a specific order in the afs file and have a specific name. All of them have the extension adx, but the actual content can be in either adx or wav format. 

### Audio file mapping:
You can view a partial map of the audio files and where they are used [here](Dialog-Map.csv).

Thanks to [Polymega](https://github.com/Polymega) for help in creating this map!

### Looping:
Some dialog files in Silent Hill 2 should loop.  To do this requires modification of the adx file metadata.  The `adxencd.exe` tool has two parameters to handle this.  The first is `-lps` and the second is `-lpe`.  `-lps` stands for "loop start" and indicates the first audio [sample](https://en.wikipedia.org/wiki/Sampling_(signal_processing)) of the area in the file where you want to loop.  `-lps` stands for "loop end" and indicates the last audio sample that you want to include in the loop.  A loop can start or end anywhere in the wav file

The `Build-Dialog-Files.bat` file should be modified with the loop parameters before running it so that you ensure that these files loop correctly.

Here is an example:
```
adxencd gero_ed.wav -lps483700 -lpe1065230
```

In this example the loop starts at audio sample 483700 and ends at audio sample 1065230.

The following dialog files should loop in Silent Hill 2:
* 1fwb_rain.wav
* clock_4.wav
* forest_wind.wav
* gero_ed.wav
* goki_jet.wav
* lakeside.wav
* silen.wav
* suiteki.wav
* tv_noiz_1.wav

For the looped dialog audio found in our project's Audio Enhancement Pack, here are their loop parameters:
```
1fwb_rain.wav -lps1 -lpe923844
clock_4.wav -lps1710 -lpe102657
forest_wind.wav -lps85632 -lpe803278
gero_ed.wav -lps483700 -lpe1065230
goki_jet.wav -lps63335 -lpe644599
ind_ev_mv.wav -lps1 -lpe1737606
lakeside.wav -lps88921 -lpe752775
silen.wav -lps0 -lpe591491
suiteki.wav -lps64032 -lpe338299
tv_noiz_1.wav -lps1 -lpe53759
```

### Instructions:
To create the `voice.afs` file for Silent Hill 2 copy all the WAV files into a folder and run the `Build-Dialog-Files.bat` tool.  This tool will create the `voice.afs` file.  All you need to do is copy the `voice` folder over the top of the `voice` folder in the `Silent Hill 2\data\sound\adx` folder.

1. Copy all the Dialog WAV files into a folder.
2. Copy `adxencd.exe`, `AFSPacker.exe` and `Build-Dialog-Files.bat` into the same folder with the WAV files.
3. Run the `Build-Dialog-Files.bat` script.
4. Copy this new `voice` folder into the `Silent Hill 2\data\sound\adx` folder.

**IMPORTANT: Make sure to back up the original `voice.afs` in case you run into any issues.

#### Required Files:
1. adxencd.exe
2. Build-Dialog-Files.bat
3. AFSPacker.exe
4. Silent Hill 2 dialog wav sound files

#### Script Files:
	adxencd.exe
	AFSPacker.exe
	Build-Dialog-Files.bat

#### Audio Files:
	1fwb_rain.wav
	62_0A.wav
	703_a.wav
	703_b.wav
	703_c.wav
	aijin.wav
	aijin_2.wav
	aijin_2a.wav
	aijin_2b.wav
	aijin_2b_mix.wav
	ana_1.wav
	ap_boss.wav
	be_mae_2.wav
	boilers.wav
	boilers_l.wav
	bort_1.wav
	bort_2.wav
	bp_edi_1_a.wav
	bp_edi_1_b.wav
	bp_edi_2.wav
	bp_edi_3.wav
	bp_mae.wav
	bp_mar_tsuika.wav
	byouin_exit.wav
	byouin_hairu.wav
	byouin_lau.wav
	chasaw_h_46.wav
	chasaw_l_45.wav
	CHEN_WO.wav
	clock_4.wav
	dust_1.wav
	dust_ju_6.wav
	edi_korosu_1.wav
	edi_korosu_2.wav
	elev_move_12.wav
	fan_go.wav
	fire_agl.wav
	forest_wind.wav
	fukkatsu.wav
	fukkatsu_2.wav
	GAME_OVER.wav
	geki.wav
	gekijyou_1.wav
	gekijyou_2.wav
	gekijyou_3.wav
	gero_ed.wav
	gero_edi_1.wav
	gero_edi_2.wav
	gero_edi_3.wav
	goki_jet.wav
	go_to_ura_1.wav
	go_to_ura_2.wav
	g_start.wav
	hakaba_agl_1.wav
	hakaba_agl_2.wav
	hakaba_agl_3.wav
	harigane.wav
	hei.wav
	henjee_1.wav
	henjee_2.wav
	hiroba.wav
	hotel_d1_cl.wav
	inai.wav
	ind_ev_mv.wav
	inoru_onna.wav
	inuend_bgm.wav
	jisatsu.wav
	jisatsu_2.wav
	jump_1.wav
	jump_2.wav
	jump_3.wav
	jump_4.wav
	jump_5.wav
	kagikeri_1.wav
	kanaami.wav
	kao.wav
	killer_edi_1.wav
	killer_edi_2.wav
	killer_edi_3.wav
	killer_edi_4.wav
	kitanai_1.wav
	knife_agl.wav
	kokuchi.wav
	kubituri.wav
	lakeside.wav
	lau_damasu_1.wav
	lau_damasu_2.wav
	lau_damasu_3.wav
	letter_water.wav
	letter_water_st.wav
	maria_2dead.wav
	mary_let_fix.wav
	mar_dead_1.wav
	mar_dead_2_1.wav
	mar_dead_2_2.wav
	mar_deai_1.wav
	mar_deai_2.wav
	mar_end_roll.wav
	mar_hagureru_1.wav
	mar_hagureru_2.wav
	mar_hagureru_3.wav
	mar_ikiteruka_1.wav
	mar_ikiteruka_2.wav
	mar_saikai_1.wav
	mar_saikai_2.wav
	meabos_mori.wav
	mizu_hiki.wav
	mry_rakka.wav
	mukashi_room_2.wav
	m_deai_tsuika.wav
	nageru.wav
	nuigurumi_1.wav
	nuigurumi_2.wav
	papa_agl_1.wav
	papa_agl_2.wav
	papa_agl_3.wav
	piano_1.wav
	piano_2.wav
	pipe_car_e.wav
	QUIZ.wav
	radio_43.wav
	reisouko_1.wav
	reisouko_2.wav
	saisho_1.wav
	saisho_2.wav
	saisho_3.wav
	sankaku_jisatsu.wav
	sankaku_toujyou.wav
	sankaku_vs.wav
	satsugai.wav
	save_sound.wav
	SCENE_01.wav
	SCENE_02.wav
	SCENE_45.wav
	SCENE_53.wav
	silen.wav
	splay_at_48.wav
	suiteki.wav
	surprise_01.wav
	surprise_02.wav
	surprise_03.wav
	surprise_07.wav
	tejyou.wav
	tobira_mado_1.wav
	tobira_mado_2.wav
	tobira_mado_3.wav
	toile_nok.wav
	tsuri_1.wav
	tv_noiz_1.wav
	UFO_1.wav
	UFO_2.wav
	UFO_END.wav
	WATER_MARY.wav
	what_amy.wav
	what_amy_ato.wav
	yarinaoshi.wav
	yarinaoshi_2_2.wav
	yarinaoshi_letter.wav
	YARINAOSHI_LETTER_MIX.wav

### Extract audio files from compiled `voice.afs` container:
Run `Extract-Dialog-Files.bat` in the same folder as the `voice.afs` container file. This will create a new `Extracted-Dialog-Files` folder in the directory with the audio files extracted out as .wav.

Note: If an audio file had loop parameters set when compiling the `voice.afs` file, these particular audio files will be extracted out as .adx instead of .wav. There might be the possibility of quality loss due to the ADX being a lossy format.
