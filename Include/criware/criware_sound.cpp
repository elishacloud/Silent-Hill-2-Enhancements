/*
* Copyright (C) 2022 Gemini
* ===============================================================
* Generic sound interface
* ---------------------------------------------------------------
* A generic sound interface that can be expanded to support any
* API.
* ===============================================================
*/
#include "criware.h"

SndObjBase* sound_obj_tbl[SOUND_MAX_OBJ];

SndObjBase* adxs_FindObj()
{
	for (int i = 0; SOUND_MAX_OBJ; i++)
	{
		if (sound_obj_tbl[i]->used == 0)
			return sound_obj_tbl[i];
	}

	return nullptr;
}

void adxs_Update()
{
	for (int i = 0; i < SOUND_MAX_OBJ; i++)
		sound_obj_tbl[i]->Update();
}
