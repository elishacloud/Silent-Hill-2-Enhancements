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
		if (sound_obj_tbl[i]->used == 0)
			return sound_obj_tbl[i];

	ADXD_Log(__FUNCTION__ ": couldn't find any unused sound objects.\n");

	return nullptr;
}

void adxs_Clear(SndObjBase* obj)
{
	obj->Release();
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
