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
	for (int i = 0; i < SOUND_MAX_OBJ; i++)
	{
		if (sound_obj_tbl[i]->used == 0)
		{
			sound_obj_tbl[i]->Release();
			return sound_obj_tbl[i];
		}
	}

	return nullptr;
}

void adxs_Update()
{
	for (int i = 0; i < SOUND_MAX_OBJ; i++)
		sound_obj_tbl[i]->Update();
}

void adxs_Release()
{
	for (int i = 0; i < SOUND_MAX_OBJ; i++)
	{
		if (sound_obj_tbl[i])
		{
			delete sound_obj_tbl[i];
			sound_obj_tbl[i] = nullptr;
		}
	}
}
