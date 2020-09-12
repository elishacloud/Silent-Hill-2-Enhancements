#pragma once

struct TexSize
{
	char* Name;
	unsigned short X;
	unsigned short Y;
	bool IsReference;
	bool IsMap;
	bool IsScaled;
};

constexpr TexSize DefaultTextureList[] = {
	
	// Maps
	
	{ "data/pic/add/mapboldwin.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/apartmape1f.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/apartmape2f.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/apartmape3f.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/apartmapew1f.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/apartmapew2f.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/apartmapew3f.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/apartmapw.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hospitalmap01.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hospitalmap02.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hotelmap1f01.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hotelmap1f02.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hotelmap2f01.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hotelmap2f02.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hotelmap3f01.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hotelmap3f02.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hotelmapbf01.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/hotelmapbf02.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/outmap.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/outmape.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/outmapw.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/prisonmap.tex", 1024, 1024, 1, 1, 0 },
	{ "data/pic/map/prisonmap03.tex", 1024, 1024, 1, 1, 0 },
	
	// Map Markings
	
	{ "data/pic/add/mapmark.tex", 512, 512, 0, 1, 0 },
	{ "data/pic/map/apartmapmark.tex", 256, 512, 0, 1, 0 },
	{ "data/pic/map/hospitalmapmk.tex", 512, 512, 0, 1, 0 },
	{ "data/pic/map/hotelmapmark.tex", 512, 512, 0, 1, 0 },
	{ "data/pic/map/outmapmark.tex", 512, 512, 0, 1, 0 },
	{ "data/pic/map/prisonmap01mark.tex", 512, 512, 0, 1, 0 },
	{ "data/pic/map/prisonmap02mark.tex", 512, 512, 0, 1, 0 },
	{ "data/pic/map/prisonmap03mark.tex", 512, 512, 0, 1, 0 },
	
	// Inventory Images
	
	{ "data/pic/add/itemmenux.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/etc/itemmenu2.tex", 1024, 1024, 1, 0, 0 },
	
	// Inventory Examine Images
	
	{ "data/pic/add/xblackhouban.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/add/xredhouban.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/add/xwhitehouban.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/etc/p_laura_letter.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/etc/p_letterm.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/etc/p_letterm_w.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/htl/p_redreling.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/item/x_coinelder.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_coinelder_ura.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_coinprisoner.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_coinprisoner_ura.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_coinsnake.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_coinsnake_ura.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_keyclock.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_keygate.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_keynorth.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_keyrapis.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_keyspiral.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_mary_p.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_plate_female.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_plate_kick.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_plate_pig.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_ringlead.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/item/x_ringopper.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/out/p_lostmemory.tex", 512, 512, 1, 0, 0 },
	
	// Outside/Town Full Screen Images
	
	{ "data/pic/out/p_incar.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/out/p_outmape.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/out/p_outmapw.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/out/p_redpaper.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/out/p_spana.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/out/p_swamp.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/out/p_swamp2.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/out/statuebox.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/out/statuekey.tex", 512, 512, 1, 0, 1 },
	
	// Outside/Town Overlaid Images
	
	{ "data/pic/out/p_incar_map.tex", 256, 256, 0, 0, 1 },
	{ "data/pic/out/statuekey2.tex", 256, 256, 0, 0, 1 },
	
	// Apartment Full Screen Images
	
	{ "data/pic/apt/clock_close.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/clock_memo.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/clock_memo_2.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/clock_name.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/clock_open.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/p_desk_coin.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/p_desk_hint.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/p_dust_in.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/p_dust_out.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/p_endhint.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/p_family.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/p_safe_close.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/apt/p_tourist.tex", 512, 512, 1, 0, 1 },
	
	// Apartment Overlaid Images
	
	{ "data/pic/apt/clock_hari.tex", 256, 256, 0, 0, 1 },
	{ "data/pic/apt/p_desk_coin_coin.tex", 64, 256, 0, 0, 1 },
	{ "data/pic/apt/p_dust_out_coin.tex", 128, 128, 0, 0, 1 },
	{ "data/pic/apt/p_safe_close2.tex", 128, 128, 0, 0, 1 },
	
	// Hospital Full Screen Images
	
	{ "data/pic/hsp/carbon.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_box.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_boxnumber.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_carbon.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_diary.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_doctormemo.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_drainage.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_female.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_hair.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_h_elevator.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_h_elevator_2.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_musiuminfo.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_panel.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/hsp/p_patient.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/p_quiz.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/hsp/whiteboard.tex", 512, 512, 1, 0, 1 },
	
	// Hospital Overlaid Images
	
	{ "data/pic/hsp/p_boxnumber_2.tex", 512, 256, 0, 0, 1 },
	{ "data/pic/hsp/p_doctormemo_key.tex", 128, 128, 0, 0, 1 },
	{ "data/pic/hsp/p_drainage_key.tex", 64, 64, 0, 0, 1 },
	{ "data/pic/hsp/p_female_ring.tex", 64, 64, 0, 0, 1 },
	{ "data/pic/hsp/p_hair_hair.tex", 256, 64, 0, 0, 1 },
	{ "data/pic/hsp/p_h_elevator_botan.tex", 128, 256, 0, 0, 1 },
	{ "data/pic/hsp/p_museuminfo_key.tex", 128, 256, 0, 0, 1 },
	{ "data/pic/hsp/p_quiz_btn.tex", 128, 128, 0, 0, 1 },
	{ "data/pic/hsp/pboxkey01.tex", 512, 512, 0, 0, 1 },
	
	// Prison/Labyrinth Full Screen Images
	
	{ "data/pic/dls/p_436.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_fireman.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_flingwoman.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_hanging_2.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_hanging_easy.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_hanging_extra.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_hanging_hard.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_hanging_normal.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_magazin.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_manacles_off.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_manacles_on.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_newspaper.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_panelrun.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_pull_off.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_pull_on.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_sankaku.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/dls/p_tablet.tex", 512, 512, 1, 0, 1 },
	
	// Prison/Labyrinth Overlaid Images
	
	{ "data/pic/dls/p_panelrun_btn.tex", 256, 256, 0, 0, 1 },
	{ "data/pic/dls/p_tablet_plate.tex", 256, 256, 0, 0, 1 },
	
	// Hotel Full Screen Images
	
	{ "data/pic/htl/p_bag_close.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_bag_open.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_can_close.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_can_open.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_egghollow.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_htl_elevator.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_htl_elevator_2.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_laura_letter.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/htl/p_marker.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_receipt.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_trank.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/htl/p_video.tex", 512, 512, 1, 0, 1 },	
	
	// Hotel Overlaid Images

	{ "data/pic/htl/dram.tex", 256, 256, 0, 0, 1 },
	{ "data/pic/htl/egg.tex", 64, 64, 0, 0, 1 },
	{ "data/pic/htl/maker.tex", 512, 512, 0, 0, 1 },
	{ "data/pic/htl/p_bag_open_key.tex", 64, 64, 0, 0, 1 },
	{ "data/pic/htl/p_can_open2.tex", 64, 128, 0, 0, 1 },
	{ "data/pic/htl/p_hanging2.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/htl/p_video02.tex", 256, 128, 0, 0, 1 },
	
	// Born From a Wish Full Screen Images
	
	{ "data/pic/add/p_birthday.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/add/p_entotsu.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/add/p_lostbook.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/add/p_teien.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/add/p_zukan.tex", 512, 512, 1, 0, 1 },
	
	// Born From a Wish Overlaid Images
	
	{ "data/pic/add/teien2.tex", 512, 512, 0, 0, 1 },
	
	// UFO Full Screen Images
	
	{ "data/pic/ufo/hj01.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/hj02.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/hj03.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/hj032.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/hj04.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/hj05.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/hj06.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/staff01.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/staff02.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/staff03.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/staff04.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/staff05.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/staff06.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/staff07.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/ufomado01.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/ufomado02.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/ufomizu01.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/ufomizu03.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/ufoniwa01.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/ufoniwa03.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/words01.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/words02.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/words03.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/words04.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/words05.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/words06.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/words062.tex", 512, 512, 1, 0, 1 },
	{ "data/pic/ufo/words07.tex", 512, 512, 1, 0, 1 },
	
	// UFO Overlaid Images
	
	{ "data/pic/ufo/hj07.tex", 512, 512, 0, 0, 1 },
	{ "data/pic/ufo/ufomado03.tex", 512, 128, 0, 0, 1 },
	{ "data/pic/ufo/ufomizu02.tex", 64, 64, 0, 0, 1 },
	{ "data/pic/ufo/ufomizu04.tex", 512, 256, 0, 0, 1 },
	{ "data/pic/ufo/ufoniwa02.tex", 64, 64, 0, 0, 1 },
	{ "data/pic/ufo/ufoniwa04.tex", 512, 512, 0, 0, 1 },
	
	// Splash/Main Menu/Save Screen Images
	
	{ "data/menu/mc/mu_alpha.tbn2", 512, 64, 0, 0, 0 },
	{ "data/menu/mc/savebg.tbn2", 512, 512, 1, 0, 0 },
	{ "data/menu/mc/savebg2.tbn2", 512, 512, 1, 0, 0 },
	{ "data/pic/etc/konami2048x2048.tex", 2048, 2048, 0, 0, 0 },
	{ "data/pic/etc/splash_logo.tex", 2048, 2048, 0, 0, 0 },
	{ "data/pic/etc/start00.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/etc/start00j.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/etc/start01.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/start01j.tex", 512, 512, 0, 0, 0 },
	
	// Other Full Screen Images
	
	{ "data/pic/add/warning.tex", 512, 512, 1, 0, 1 },
	{ "data/chr/jms/lll_jms.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/footmark.tbn2", 64, 64, 0, 0, 0 },
	{ "data/etc/effect/lens_flare.tbn2", 128, 128, 0, 0, 0 },
	{ "data/pic/etc/carsol.tex", 256, 128, 0, 0, 0 },
	{ "data/pic/etc/gameover1.tex", 512, 512, 1, 0, 0 },
	{ "data/pic/etc/p_memo.tex", 512, 512, 1, 0, 0 },
	
	// Not used in game
	
	{ "data/pic/effect/apstair00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/bar00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/bloodtex0.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/boatmask.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/candle.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/fire00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/fire01.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/flame003.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/flame032.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/flear.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/fly1.tbn2", 64, 64, 0, 0, 0 },
	{ "data/pic/effect/fog.tbn2", 256, 256, 0, 0, 0 },
	{ "data/pic/effect/fog.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/effect/footmark.tbn2", 64, 64, 0, 0, 0 },
	{ "data/pic/effect/gesui00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/gesui01.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/glas03.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/hbstair00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/kitchen00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/lens_flare.tbn2", 128, 128, 0, 0, 0 },
	{ "data/pic/effect/msi00.tbn2", 256, 256, 0, 0, 0 },
	{ "data/pic/effect/passba00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/passbb00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/playable0.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/poison.tbn2", 256, 256, 0, 0, 0 },
	{ "data/pic/effect/psground00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/psground01.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/psground02.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/psground03.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/psground04.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/spark.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/sprash.tbn2", 256, 256, 0, 0, 0 },
	{ "data/pic/effect/water00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/water01.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/water02.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/effect/water40.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/apstair00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/bar00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/bloodtex0.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/boatmask.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/candle.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/fire00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/fire01.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/flame003.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/flame032.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/flear.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/fly1.tbn2", 64, 64, 0, 0, 0 },
	{ "data/etc/effect/fog.tbn2", 256, 256, 0, 0, 0 },
	{ "data/etc/effect/fog.tex", 256, 256, 0, 0, 0 },
	{ "data/etc/effect/gesui00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/gesui01.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/glas03.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/hbstair00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/kitchen00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/msi00.tbn2", 256, 256, 0, 0, 0 },
	{ "data/etc/effect/passba00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/passbb00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/playable0.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/poison.tbn2", 256, 256, 0, 0, 0 },
	{ "data/etc/effect/psground00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/psground01.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/psground02.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/psground03.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/psground04.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/spark.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/sprash.tbn2", 256, 256, 0, 0, 0 },
	{ "data/etc/effect/water00.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/water01.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/water02.tbn2", 512, 512, 0, 0, 0 },
	{ "data/etc/effect/water40.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/carbon.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/comingsoon.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/female.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/itemmenu.tex", 1024, 1024, 0, 0, 0 },
	{ "data/pic/panel.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/p_quiz_btn.tex", 128, 128, 0, 0, 0 },
	{ "data/pic/title.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/apt/dust_in.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/apt/dust_out.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/dls/p_hanging.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/dls/p_marker.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/dls/pmanacleson_tr.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/botan.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/etc/botan_j.tex", 256, 256, 0, 0, 0 },
	{ "data/pic/etc/cesa.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/comingsoon.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/kcet.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/konami.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/konami_r.tbn2", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/p_lostmemory.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/p_redreling.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/savebg1.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/sce.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/sh2dc.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/etc/splash_logo_us.tex", 2048, 2048, 0, 0, 0 },
	{ "data/pic/etc/itemmenu.tex", 1024, 1024, 1, 0, 0 },
	{ "data/pic/etc/title.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/hsp/p_panel02.tex", 128, 256, 0, 0, 1 },
	{ "data/pic/hsp/pboxkey.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/htl/p_hanging.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/htl/prisonmap03mark.tex", 128, 128, 0, 0, 0 },
	{ "data/pic/item/p_laura_letter.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/item/p_letterm.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/item/p_letterm_w.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/item/p_lostmemory.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/item/p_memo.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/item/p_redreling.tex", 512, 512, 0, 0, 0 },
	{ "data/pic/map/p01.tex", 512, 512, 0, 1, 0 },
	{ "data/pic/map/p02.tex", 512, 512, 0, 1, 0 },
	{ "data/pic/map/prisonmap01.tex", 512, 512, 1, 1, 0 },
	{ "data/pic/map/prisonmap02.tex", 512, 512, 1, 1, 0 }
};
