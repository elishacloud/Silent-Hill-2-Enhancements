/*
* Copyright (C) 2022 Gemini
* ===============================================================
* ADX reader module
* ---------------------------------------------------------------
* Code to open and parse ADX files.
* ===============================================================
*/
#include "criware.h"
#include <chrono>

#define MEASURE_ACCESS		0

#if MEASURE_ACCESS
static double TimeGetTime()
{
	return std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 1000.;
}
#endif

int OpenADX(ADXStream* adx)
{
	ADX_headerV3 head;
	adx->Read(&head, sizeof(head));

	if (head.magic.w() != 0x8000)
	{
		adx->Close();
		return -1;
	}

	if (head.copyright_offset.w() <= 32)
	{
		ADX_header_AIX* aix_head = (ADX_header_AIX*)&head;

		adx->block_size = aix_head->block_size;
		adx->channel_count = aix_head->channel_count;
		adx->highpass_frequency = aix_head->highpass_frequency.w();
		adx->sample_bitdepth = aix_head->sample_bitdepth;
		adx->sample_rate = aix_head->sample_rate.dw();
		adx->total_samples = aix_head->total_samples.dw();
		adx->copyright_offset = aix_head->copyright_offset.w();

		adx->loop_enabled = 0;
		adx->loop_start_index = 0;
		adx->loop_end_index = adx->total_samples;
	}
	else if (head.version == 3)
	{
		adx->block_size = head.block_size;
		adx->channel_count = head.channel_count;
		adx->highpass_frequency = head.highpass_frequency.w();
		adx->loop_enabled = head.loop_enabled.w();
		adx->loop_end_index = head.loop_sample_end.dw();
		adx->loop_start_index = head.loop_sample_begin.dw();
		adx->sample_bitdepth = head.sample_bitdepth;
		adx->sample_rate = head.sample_rate.dw();
		adx->total_samples = head.total_samples.dw();
		adx->copyright_offset = head.copyright_offset.w();
	}
	else if (head.version == 4)
	{
		ADX_headerV4* head4 = (ADX_headerV4*)&head;

		adx->block_size = head4->block_size;
		adx->channel_count = head4->channel_count;
		adx->highpass_frequency = head4->highpass_frequency.w();
		adx->loop_enabled = head4->loop_enabled.dw();
		adx->loop_end_index = head4->loop_sample_end.dw();
		adx->loop_start_index = head4->loop_sample_begin.dw();
		adx->sample_bitdepth = head4->sample_bitdepth;
		adx->sample_rate = head4->sample_rate.dw();
		adx->total_samples = head4->total_samples.dw();
		adx->copyright_offset = head4->copyright_offset.w();
	}
	else
	{
		delete adx;
		return -1;
	}

	ADXDEC_SetCoeff(adx);
	return S_OK;
}

int OpenADX(const char* filename, ADXStream** obj)
{
	*obj = nullptr;

	ADXStream* adx = new ADXStream;

	if (FAILED(adx->Open(filename)))
	{
		delete adx;
		return 0;
	}

	ADX_headerV3 head;
	adx->Read(&head, sizeof(head));

	if (head.magic.w() != 0x8000)
	{
		delete adx;
		return 0;
	}

	if (head.version == 3)
	{
		adx->block_size = head.block_size;
		adx->channel_count = head.channel_count;
		adx->highpass_frequency = head.highpass_frequency.w();
		adx->loop_enabled = head.loop_enabled.w();
		adx->loop_end_index = head.loop_sample_end.dw();
		adx->loop_start_index = head.loop_sample_begin.dw();
		adx->sample_bitdepth = head.sample_bitdepth;
		adx->sample_rate = head.sample_rate.dw();
		adx->total_samples = head.total_samples.dw();
		adx->copyright_offset = head.copyright_offset.w();
	}
	else if (head.version == 4)
	{
		ADX_headerV4* head4 = (ADX_headerV4*)&head;

		adx->block_size = head4->block_size;
		adx->channel_count = head4->channel_count;
		adx->highpass_frequency = head4->highpass_frequency.w();
		adx->loop_enabled = head4->loop_enabled.dw();
		adx->loop_end_index = head4->loop_sample_end.dw();
		adx->loop_start_index = head4->loop_sample_begin.dw();
		adx->sample_bitdepth = head4->sample_bitdepth;
		adx->sample_rate = head4->sample_rate.dw();
		adx->total_samples = head4->total_samples.dw();
		adx->copyright_offset = head4->copyright_offset.w();
	}
	else
	{
		delete adx;
		return 0;
	}

	ADXDEC_SetCoeff(adx);

	*obj = adx;

	return 1;
}

void adx_StartFname(ADXT_Object* obj, const char* fname)
{
	if (obj == nullptr)
		return;

#if MEASURE_ACCESS
	char str[MAX_PATH];
	double start = TimeGetTime();
	sprintf_s(str, sizeof(str), __FUNCTION__ ": preparing ADX %s...\n", fname);
	OutputDebugStringA(str);
#endif

	if (obj->state != ADXT_STAT_STOP)
		ADXT_Stop(obj);

	ADXStream* stream;
	OpenADX(fname, &stream);

	obj->state = ADXT_STAT_PLAYING;
	obj->stream = stream;
	obj->obj = adxs_FindObj();
	obj->obj->loops = true;
	obj->obj->adx = obj;

#if MEASURE_ACCESS
	sprintf_s(str, sizeof(str), __FUNCTION__ ": ADX done parsing in %f ms.\n", TimeGetTime() - start);
	OutputDebugStringA(str);
#endif

	obj->obj->CreateBuffer(stream);
	obj->obj->Play();
}

void ADXT_Object::Release()
{
	if (state != ADXT_STAT_STOP)
	{
		if (is_blocking)
		{
			if (obj)
			{
				// wait for any pending data transfers
				while (obj->trans_lock);
				// lock the thread to avoid further transfers
				ADX_lock();
				obj->Stop();
				obj->Release();
				// unlock thread and trash the dsound reference
				ADX_unlock();
				obj = nullptr;
			}
			if (stream && !is_aix)
				delete stream;
			stream = nullptr;
			state = ADXT_STAT_STOP;
		}
		else
		{
			if (obj->Stop())
			{
				obj->Release();
				obj = nullptr;
			}
			if (stream && !is_aix)
				delete stream;
			stream = nullptr;
			state = ADXT_STAT_STOP;
		}
	}
}
