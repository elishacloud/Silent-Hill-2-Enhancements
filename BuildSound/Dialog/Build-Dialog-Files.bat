echo off

rem ****
echo Creating ADX files
rem ****

adxencd 1fwb_rain.wav
adxencd 62_0A.wav
adxencd 703_a.wav
adxencd 703_b.wav
adxencd 703_c.wav
adxencd aijin.wav
adxencd aijin_2.wav
adxencd aijin_2a.wav
adxencd aijin_2b.wav
adxencd aijin_2b_mix.wav
adxencd ana_1.wav
adxencd ap_boss.wav
adxencd be_mae_2.wav
adxencd boilers.wav
adxencd boilers_l.wav
adxencd bort_1.wav
adxencd bort_2.wav
adxencd bp_edi_1_a.wav
adxencd bp_edi_1_b.wav
adxencd bp_edi_2.wav
adxencd bp_edi_3.wav
adxencd bp_mae.wav
adxencd bp_mar_tsuika.wav
adxencd byouin_exit.wav
adxencd byouin_hairu.wav
adxencd byouin_lau.wav
adxencd chasaw_h_46.wav
adxencd chasaw_l_45.wav
adxencd CHEN_WO.wav
adxencd clock_4.wav
adxencd dust_1.wav
adxencd dust_ju_6.wav
adxencd edi_korosu_1.wav
adxencd edi_korosu_2.wav
adxencd elev_move_12.wav
adxencd fan_go.wav
adxencd fire_agl.wav
adxencd forest_wind.wav
adxencd fukkatsu.wav
adxencd fukkatsu_2.wav
adxencd GAME_OVER.wav
adxencd geki.wav
adxencd gekijyou_1.wav
adxencd gekijyou_2.wav
adxencd gekijyou_3.wav
adxencd gero_ed.wav
adxencd gero_edi_1.wav
adxencd gero_edi_2.wav
adxencd gero_edi_3.wav
adxencd goki_jet.wav
adxencd go_to_ura_1.wav
adxencd go_to_ura_2.wav
adxencd g_start.wav
adxencd hakaba_agl_1.wav
adxencd hakaba_agl_2.wav
adxencd hakaba_agl_3.wav
adxencd harigane.wav
adxencd hei.wav
adxencd henjee_1.wav
adxencd henjee_2.wav
adxencd hiroba.wav
adxencd hotel_d1_cl.wav
adxencd inai.wav
adxencd ind_ev_mv.wav
adxencd inoru_onna.wav
adxencd inuend_bgm.wav
adxencd jisatsu.wav
adxencd jisatsu_2.wav
adxencd jump_1.wav
adxencd jump_2.wav
adxencd jump_3.wav
adxencd jump_4.wav
adxencd jump_5.wav
adxencd kagikeri_1.wav
adxencd kanaami.wav
adxencd kao.wav
adxencd killer_edi_1.wav
adxencd killer_edi_2.wav
adxencd killer_edi_3.wav
adxencd killer_edi_4.wav
adxencd kitanai_1.wav
adxencd knife_agl.wav
adxencd kokuchi.wav
adxencd kubituri.wav
adxencd lakeside.wav
adxencd lau_damasu_1.wav
adxencd lau_damasu_2.wav
adxencd lau_damasu_3.wav
adxencd letter_water.wav
adxencd letter_water_st.wav
adxencd maria_2dead.wav
adxencd mary_let_fix.wav
adxencd mar_dead_1.wav
adxencd mar_dead_2_1.wav
adxencd mar_dead_2_2.wav
adxencd mar_deai_1.wav
adxencd mar_deai_2.wav
adxencd mar_end_roll.wav
adxencd mar_hagureru_1.wav
adxencd mar_hagureru_2.wav
adxencd mar_hagureru_3.wav
adxencd mar_ikiteruka_1.wav
adxencd mar_ikiteruka_2.wav
adxencd mar_saikai_1.wav
adxencd mar_saikai_2.wav
adxencd meabos_mori.wav
adxencd mizu_hiki.wav
adxencd mry_rakka.wav
adxencd mukashi_room_2.wav
adxencd m_deai_tsuika.wav
adxencd nageru.wav
adxencd nuigurumi_1.wav
adxencd nuigurumi_2.wav
adxencd papa_agl_1.wav
adxencd papa_agl_2.wav
adxencd papa_agl_3.wav
adxencd piano_1.wav
adxencd piano_2.wav
adxencd pipe_car_e.wav
adxencd QUIZ.wav
adxencd radio_43.wav
adxencd reisouko_1.wav
adxencd reisouko_2.wav
adxencd saisho_1.wav
adxencd saisho_2.wav
adxencd saisho_3.wav
adxencd sankaku_jisatsu.wav
adxencd sankaku_toujyou.wav
adxencd sankaku_vs.wav
adxencd satsugai.wav
adxencd save_sound.wav
adxencd SCENE_01.wav
adxencd SCENE_02.wav
adxencd SCENE_45.wav
adxencd SCENE_53.wav
adxencd silen.wav
adxencd splay_at_48.wav
adxencd suiteki.wav
adxencd surprise_01.wav
adxencd surprise_02.wav
adxencd surprise_03.wav
adxencd surprise_07.wav
adxencd tejyou.wav
adxencd tobira_mado_1.wav
adxencd tobira_mado_2.wav
adxencd tobira_mado_3.wav
adxencd toile_nok.wav
adxencd tsuri_1.wav
adxencd tv_noiz_1.wav
adxencd UFO_1.wav
adxencd UFO_2.wav
adxencd UFO_END.wav
adxencd WATER_MARY.wav
adxencd what_amy.wav
adxencd what_amy_ato.wav
adxencd yarinaoshi.wav
adxencd yarinaoshi_2_2.wav
adxencd yarinaoshi_letter.wav
adxencd YARINAOSHI_LETTER_MIX.wav

rem ****
echo Create folders and move files to folders
rem ****

md ADX

set filename=1fwb_rain
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=62_0A
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=703_a
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=703_b
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=703_c
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=aijin
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=aijin_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=aijin_2a
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=aijin_2b
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=aijin_2b_mix
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=ana_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=ap_boss
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=be_mae_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=boilers
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=boilers_l
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=bort_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=bort_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=bp_edi_1_a
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=bp_edi_1_b
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=bp_edi_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=bp_edi_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=bp_mae
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=bp_mar_tsuika
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=byouin_exit
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=byouin_hairu
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=byouin_lau
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=chasaw_h_46
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=chasaw_l_45
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=CHEN_WO
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=clock_4
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=dust_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=dust_ju_6
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=edi_korosu_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=edi_korosu_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=elev_move_12
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=fan_go
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=fire_agl
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=forest_wind
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=fukkatsu
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=fukkatsu_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=GAME_OVER
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=geki
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=gekijyou_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=gekijyou_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=gekijyou_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=gero_ed
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=gero_edi_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=gero_edi_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=gero_edi_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=goki_jet
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=go_to_ura_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=go_to_ura_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=g_start
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=hakaba_agl_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=hakaba_agl_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=hakaba_agl_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=harigane
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=hei
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=henjee_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=henjee_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=hiroba
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=hotel_d1_cl
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=inai
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=ind_ev_mv
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=inoru_onna
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=inuend_bgm
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=jisatsu
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=jisatsu_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=jump_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=jump_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=jump_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=jump_4
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=jump_5
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=kagikeri_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=kanaami
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=kao
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=killer_edi_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=killer_edi_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=killer_edi_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=killer_edi_4
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=kitanai_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=knife_agl
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=kokuchi
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=kubituri
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=lakeside
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=lau_damasu_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=lau_damasu_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=lau_damasu_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=letter_water
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=letter_water_st
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=maria_2dead
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mary_let_fix
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_dead_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_dead_2_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_dead_2_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_deai_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_deai_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_end_roll
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_hagureru_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_hagureru_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_hagureru_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_ikiteruka_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_ikiteruka_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_saikai_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mar_saikai_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=meabos_mori
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mizu_hiki
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mry_rakka
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=mukashi_room_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=m_deai_tsuika
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=nageru
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=nuigurumi_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=nuigurumi_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=papa_agl_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=papa_agl_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=papa_agl_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=piano_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=piano_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=pipe_car_e
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=QUIZ
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=radio_43
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=reisouko_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=reisouko_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=saisho_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=saisho_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=saisho_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=sankaku_jisatsu
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=sankaku_toujyou
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=sankaku_vs
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=satsugai
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=save_sound
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=SCENE_01
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=SCENE_02
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=SCENE_45
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=SCENE_53
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=silen
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=splay_at_48
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=suiteki
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=surprise_01
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=surprise_02
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=surprise_03
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=surprise_07
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=tejyou
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=tobira_mado_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=tobira_mado_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=tobira_mado_3
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=toile_nok
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=tsuri_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=tv_noiz_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=UFO_1
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=UFO_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=UFO_END
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=WATER_MARY
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=what_amy
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=what_amy_ato
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=yarinaoshi
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=yarinaoshi_2_2
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=yarinaoshi_letter
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )
set filename=YARINAOSHI_LETTER_MIX
if exist %filename%.adx ( move /y %filename%.adx ADX ) else ( copy /y %filename%.wav ADX )

rem ****
echo Create voice.afs
rem ****

del filelist.lst
dir ADX\yarinaoshi_2_2.* /B >> filelist.lst
dir ADX\aijin.* /B >> filelist.lst
dir ADX\aijin_2.* /B >> filelist.lst
dir ADX\aijin_2a.* /B >> filelist.lst
dir ADX\aijin_2b.* /B >> filelist.lst
dir ADX\ana_1.* /B >> filelist.lst
dir ADX\byouin_lau.* /B >> filelist.lst
dir ADX\dust_1.* /B >> filelist.lst
dir ADX\edi_korosu_1.* /B >> filelist.lst
dir ADX\edi_korosu_2.* /B >> filelist.lst
dir ADX\fire_agl.* /B >> filelist.lst
dir ADX\fukkatsu.* /B >> filelist.lst
dir ADX\fukkatsu_2.* /B >> filelist.lst
dir ADX\1fwb_rain.* /B >> filelist.lst
dir ADX\boilers.* /B >> filelist.lst
dir ADX\boilers_l.* /B >> filelist.lst
dir ADX\chasaw_h_46.* /B >> filelist.lst
dir ADX\chasaw_l_45.* /B >> filelist.lst
dir ADX\CHEN_WO.* /B >> filelist.lst
dir ADX\clock_4.* /B >> filelist.lst
dir ADX\dust_ju_6.* /B >> filelist.lst
dir ADX\elev_move_12.* /B >> filelist.lst
dir ADX\fan_go.* /B >> filelist.lst
dir ADX\forest_wind.* /B >> filelist.lst
dir ADX\g_start.* /B >> filelist.lst
dir ADX\gero_ed.* /B >> filelist.lst
dir ADX\goki_jet.* /B >> filelist.lst
dir ADX\hiroba.* /B >> filelist.lst
dir ADX\hotel_d1_cl.* /B >> filelist.lst
dir ADX\ind_ev_mv.* /B >> filelist.lst
dir ADX\lakeside.* /B >> filelist.lst
dir ADX\meabos_mori.* /B >> filelist.lst
dir ADX\mizu_hiki.* /B >> filelist.lst
dir ADX\pipe_car_e.* /B >> filelist.lst
dir ADX\radio_43.* /B >> filelist.lst
dir ADX\save_sound.* /B >> filelist.lst
dir ADX\silen.* /B >> filelist.lst
dir ADX\splay_at_48.* /B >> filelist.lst
dir ADX\suiteki.* /B >> filelist.lst
dir ADX\surprise_01.* /B >> filelist.lst
dir ADX\surprise_02.* /B >> filelist.lst
dir ADX\surprise_03.* /B >> filelist.lst
dir ADX\surprise_07.* /B >> filelist.lst
dir ADX\toile_nok.* /B >> filelist.lst
dir ADX\tv_noiz_1.* /B >> filelist.lst
dir ADX\gekijyou_1.* /B >> filelist.lst
dir ADX\gekijyou_2.* /B >> filelist.lst
dir ADX\gekijyou_3.* /B >> filelist.lst
dir ADX\gero_edi_1.* /B >> filelist.lst
dir ADX\gero_edi_2.* /B >> filelist.lst
dir ADX\gero_edi_3.* /B >> filelist.lst
dir ADX\go_to_ura_1.* /B >> filelist.lst
dir ADX\go_to_ura_2.* /B >> filelist.lst
dir ADX\hakaba_agl_1.* /B >> filelist.lst
dir ADX\hakaba_agl_2.* /B >> filelist.lst
dir ADX\hakaba_agl_3.* /B >> filelist.lst
dir ADX\harigane.* /B >> filelist.lst
dir ADX\hei.* /B >> filelist.lst
dir ADX\inoru_onna.* /B >> filelist.lst
dir ADX\inuend_bgm.* /B >> filelist.lst
dir ADX\jisatsu.* /B >> filelist.lst
dir ADX\jisatsu_2.* /B >> filelist.lst
dir ADX\jump_1.* /B >> filelist.lst
dir ADX\jump_2.* /B >> filelist.lst
dir ADX\jump_3.* /B >> filelist.lst
dir ADX\jump_4.* /B >> filelist.lst
dir ADX\jump_5.* /B >> filelist.lst
dir ADX\kagikeri_1.* /B >> filelist.lst
dir ADX\kanaami.* /B >> filelist.lst
dir ADX\kao.* /B >> filelist.lst
dir ADX\killer_edi_1.* /B >> filelist.lst
dir ADX\killer_edi_2.* /B >> filelist.lst
dir ADX\killer_edi_3.* /B >> filelist.lst
dir ADX\killer_edi_4.* /B >> filelist.lst
dir ADX\kitanai_1.* /B >> filelist.lst
dir ADX\knife_agl.* /B >> filelist.lst
dir ADX\kokuchi.* /B >> filelist.lst
dir ADX\kubituri.* /B >> filelist.lst
dir ADX\lau_damasu_1.* /B >> filelist.lst
dir ADX\lau_damasu_2.* /B >> filelist.lst
dir ADX\lau_damasu_3.* /B >> filelist.lst
dir ADX\letter_water.* /B >> filelist.lst
dir ADX\letter_water_st.* /B >> filelist.lst
dir ADX\m_deai_tsuika.* /B >> filelist.lst
dir ADX\mar_dead_1.* /B >> filelist.lst
dir ADX\mar_dead_2_1.* /B >> filelist.lst
dir ADX\mar_dead_2_2.* /B >> filelist.lst
dir ADX\mar_deai_1.* /B >> filelist.lst
dir ADX\mar_deai_2.* /B >> filelist.lst
dir ADX\mar_hagureru_1.* /B >> filelist.lst
dir ADX\mar_hagureru_2.* /B >> filelist.lst
dir ADX\mar_hagureru_3.* /B >> filelist.lst
dir ADX\mar_ikiteruka_1.* /B >> filelist.lst
dir ADX\mar_ikiteruka_2.* /B >> filelist.lst
dir ADX\mar_saikai_1.* /B >> filelist.lst
dir ADX\mar_saikai_2.* /B >> filelist.lst
dir ADX\maria_2dead.* /B >> filelist.lst
dir ADX\mary_let_fix.* /B >> filelist.lst
dir ADX\mry_rakka.* /B >> filelist.lst
dir ADX\mukashi_room_2.* /B >> filelist.lst
dir ADX\nuigurumi_1.* /B >> filelist.lst
dir ADX\nuigurumi_2.* /B >> filelist.lst
dir ADX\papa_agl_1.* /B >> filelist.lst
dir ADX\papa_agl_2.* /B >> filelist.lst
dir ADX\papa_agl_3.* /B >> filelist.lst
dir ADX\piano_1.* /B >> filelist.lst
dir ADX\piano_2.* /B >> filelist.lst
dir ADX\QUIZ.* /B >> filelist.lst
dir ADX\reisouko_1.* /B >> filelist.lst
dir ADX\reisouko_2.* /B >> filelist.lst
dir ADX\saisho_1.* /B >> filelist.lst
dir ADX\saisho_2.* /B >> filelist.lst
dir ADX\saisho_3.* /B >> filelist.lst
dir ADX\sankaku_jisatsu.* /B >> filelist.lst
dir ADX\sankaku_toujyou.* /B >> filelist.lst
dir ADX\sankaku_vs.* /B >> filelist.lst
dir ADX\satsugai.* /B >> filelist.lst
dir ADX\SCENE_01.* /B >> filelist.lst
dir ADX\SCENE_02.* /B >> filelist.lst
dir ADX\SCENE_45.* /B >> filelist.lst
dir ADX\SCENE_53.* /B >> filelist.lst
dir ADX\tejyou.* /B >> filelist.lst
dir ADX\tobira_mado_1.* /B >> filelist.lst
dir ADX\tobira_mado_2.* /B >> filelist.lst
dir ADX\tobira_mado_3.* /B >> filelist.lst
dir ADX\tsuri_1.* /B >> filelist.lst
dir ADX\WATER_MARY.* /B >> filelist.lst
dir ADX\yarinaoshi.* /B >> filelist.lst
dir ADX\62_0A.* /B >> filelist.lst
dir ADX\yarinaoshi_letter.* /B >> filelist.lst
dir ADX\aijin_2b_mix.* /B >> filelist.lst
dir ADX\ap_boss.* /B >> filelist.lst
dir ADX\be_mae_2.* /B >> filelist.lst
dir ADX\bort_1.* /B >> filelist.lst
dir ADX\bort_2.* /B >> filelist.lst
dir ADX\bp_edi_1_a.* /B >> filelist.lst
dir ADX\bp_edi_1_b.* /B >> filelist.lst
dir ADX\bp_edi_2.* /B >> filelist.lst
dir ADX\bp_edi_3.* /B >> filelist.lst
dir ADX\bp_mae.* /B >> filelist.lst
dir ADX\bp_mar_tsuika.* /B >> filelist.lst
dir ADX\byouin_exit.* /B >> filelist.lst
dir ADX\byouin_hairu.* /B >> filelist.lst
dir ADX\GAME_OVER.* /B >> filelist.lst
dir ADX\YARINAOSHI_LETTER_MIX.* /B >> filelist.lst
dir ADX\703_a.* /B >> filelist.lst
dir ADX\703_b.* /B >> filelist.lst
dir ADX\703_c.* /B >> filelist.lst
dir ADX\geki.* /B >> filelist.lst
dir ADX\henjee_1.* /B >> filelist.lst
dir ADX\henjee_2.* /B >> filelist.lst
dir ADX\inai.* /B >> filelist.lst
dir ADX\mar_end_roll.* /B >> filelist.lst
dir ADX\nageru.* /B >> filelist.lst
dir ADX\UFO_1.* /B >> filelist.lst
dir ADX\UFO_2.* /B >> filelist.lst
dir ADX\UFO_END.* /B >> filelist.lst
dir ADX\what_amy.* /B >> filelist.lst
dir ADX\what_amy_ato.* /B >> filelist.lst

rem ****
echo Create voice.afs
rem ****

md voice
del voice\voice.afs
cd ADX
..\AFSPacker -c . ..\voice\voice.afs ..\filelist.lst
cd ..
del filelist.lst
rd ADX /s /q
