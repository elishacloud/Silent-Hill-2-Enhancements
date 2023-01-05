/*
* Copyright (C) 2022 Gemini
* ===============================================================
* File streaming interfaces
* ---------------------------------------------------------------
* Classes to manage ADX and AIX streaming to a sound object, also
* common Windows file handle operations.
* ===============================================================
*/
#include "criware.h"
#include "Common\FileSystemHooks.h"
#include <mmreg.h>

// ------------------------------------------------
// Helpers for debloating I/O code
// ------------------------------------------------
HANDLE ADXF_OpenFile(const char* filename)
{
	char tmpfilename[MAX_PATH];
	return CreateFileA(GetFileModPath(filename, tmpfilename), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY /*FILE_ATTRIBUTE_NORMAL*/, nullptr);
}

void ADXF_CloseFile(HANDLE fp)
{
	CloseHandle(fp);
}

u_long ADXF_ReadFile(HANDLE fp, void* buffer, size_t size)
{
	DWORD read;
	ReadFile(fp, buffer, size, &read, nullptr);
#if _DEBUG
	if (read != size)
		ADXD_Log(__FUNCTION__ "Warning: read data is not the same as requested.\n");
#endif

	return read;
}

u_long ADXF_Tell(HANDLE fp)
{
	return SetFilePointer(fp, 0, nullptr, FILE_CURRENT);
}

u_long ADXF_Seek(HANDLE fp, LONG pos, u_long mode)
{
	return SetFilePointer(fp, pos, nullptr, mode);
}

// ------------------------------------------------
// WAV stream code, normal file wrapping and chunk
// parsing
// ------------------------------------------------
#define memalign(x, y) ((x + (y - 1)) & ~(y - 1))

int WAVStream::Open(HANDLE _fp, u_long pos)
{
	fp = _fp;
	start = pos;

	// check if it's a valid RIFF WAVE
	WAV_riff head;
	ADXF_ReadFile(fp, &head, sizeof(head));
	if ((head.ChunkID != 'RIFF' && head.ChunkID != 'FFIR') &&
		(head.Format != 'WAVE' && head.Format != 'EVAW'))
	{
		return S_FALSE;
	}

	// process the relevant tags
	if (find_data() == 0)
		return S_FALSE;
	// set position to the waveform
	pcm_seek(0);

	return S_OK;
}

u_long WAVStream::Decode(int16_t* buffer, unsigned samples_needed, bool looping_enabled)
{
	UNREFERENCED_PARAMETER(looping_enabled);

	fill((BYTE*)buffer, samples_needed);
	sample_index += samples_needed;
	return 0;
}

void WAVStream::fill(BYTE* dst, DWORD samples)
{
	// doesn't loop and already past waveform position
	if (!loop_enabled && looped)
	{
		// fill with silence
		memset(dst, 0, samples * fmt.blockAlign);
		return;
	}

	size_t cur_pos = pcm_tell();

	if (cur_pos + samples >= loop_end_index)
	{
		int rest = loop_end_index - cur_pos;
		// read whatever is at the end
		pcm_read(dst, rest);
		if (loop_enabled)
		{
			// restart the wave position
			pcm_seek(loop_start_index);
			// read the other reminder
			pcm_read(&dst[rest / fmt.blockAlign], samples - rest);
		}
		else
		{
			// fill non looping with blanks
			memset(&dst[rest / fmt.blockAlign], 0, (samples - rest) / fmt.blockAlign);
			looped = 1;
		}
	}
	else pcm_read(dst, samples);
}

int WAVStream::find_data()
{
	wav_chunk c;
	bool parse_loop = true;

	do
	{
		// parse all chunks until no data is left
		ADXF_ReadFile(fp, &c, sizeof(c));

		switch (c.magic)
		{
		case 'fmt ':	// found format chunk, store it
		case ' tmf':
			{
				size_t pos = ADXF_Tell(fp);
				ADXF_ReadFile(fp, &fmt, sizeof(fmt));
				switch (fmt.AudioFormat)
				{
				case WAVE_FORMAT_PCM:
					type = Type_PCM;
					break;
				case WAVE_FORMAT_IEEE_FLOAT:
					type = Type_FLOAT;
					break;
				default:
					return 0;
				}

				sample_rate = fmt.SamplesPerSec;
				channel_count = fmt.NumOfChan;
				ADXF_Seek(fp, pos + c.size, FILE_BEGIN);
			}
			continue;
		case 'data':	// found data chunk
		case 'atad':
			// grab the necessary data for streaming
			loop_start_index = 0;
			wav_size = c.size;
			loop_end_index = c.size / fmt.blockAlign;
			sample_bitdepth = fmt.bitsPerSample;
			total_samples = loop_end_index;
			wav_pos = ADXF_Tell(fp);
			parse_loop = false;
			break;
		}
		// go to next chunk
		ADXF_Seek(fp, memalign(c.size, 4), FILE_CURRENT);
	} while (parse_loop);

	return 1;
}

void WAVStream::pcm_read(BYTE* dst, size_t samples)
{
	switch (type)
	{
	case Type_PCM:
		ADXF_ReadFile(fp, dst, samples * fmt.blockAlign);
		break;
	case Type_FLOAT:
		{
			int16_t* d16 = (int16_t*)dst;
			for (int i = 0, count = samples * 2; i < count; i++)
			{
				float f;
				ADXF_ReadFile(fp, &f, 4);
				d16[i] = (int16_t)(f * 32768.f);
			}
		}
		break;
	}
}

void WAVStream::pcm_seek(size_t sample_pos)
{
	ADXF_Seek(fp, wav_pos + sample_pos * fmt.blockAlign, FILE_BEGIN);
}

size_t WAVStream::pcm_tell()
{
	return (ADXF_Tell(fp) - wav_pos) / fmt.blockAlign;
}

// ------------------------------------------------
// ADX stream code, normal file wrapping
// ------------------------------------------------
int ADXStream::Open(const char* filename)
{
	fp = ADXF_OpenFile(filename);
	if (fp == INVALID_HANDLE_VALUE)
		return S_FALSE;

	start = 0;
#if STR_ADX_CACHING
	Seek(0, SEEK_SET);
#endif

	return S_OK;
}

int ADXStream::Open(HANDLE _fp, u_long pos)
{
	fp = _fp;
	start = pos;
#if STR_ADX_CACHING
	pos_cache = 0;
	last_pos = (u_long)-1;
#endif
	Seek(0, SEEK_SET);

	return S_OK;
}

void ADXStream::Close()
{
	//if(start == 0)
		CloseHandle(fp);
}

u_long ADXStream::Decode(int16_t* buffer, unsigned samples_needed, bool looping_enabled)
{
	return ADXDEC_Decode(this, buffer, samples_needed, looping_enabled);
}

void ADXStream::Read(void* buffer, size_t size)
{
#if STR_ADX_CACHING
	BYTE* dst = (BYTE*)buffer;
	if (size + pos_cache > STREAM_CACHE_SIZE)
	{
		size_t remainder = STREAM_CACHE_SIZE - pos_cache;
		memcpy(dst, &cache[pos_cache], remainder);
		dst += remainder;
		size -= remainder;
		ADXF_ReadFile(fp, cache, STREAM_CACHE_SIZE);
		pos_cache = 0;
		last_pos++;
	}

	memcpy(dst, &cache[pos_cache], size);
	pos_cache += size;
#else
	ADXF_ReadFile(fp, buffer, size);
#endif
}

void ADXStream::Seek(u_long pos, u_long mode)
{
#if STR_ADX_CACHING
	u_long npos = (pos + start) / STREAM_CACHE_SIZE;
	if (npos != last_pos)
	{
		ADXF_Seek(fp, npos * STREAM_CACHE_SIZE, mode);
		ADXF_ReadFile(fp, cache, STREAM_CACHE_SIZE);
		last_pos = npos;
	}
	pos_cache = (pos + start) % STREAM_CACHE_SIZE;
#else
	ADXF_Seek(fp, pos + start, mode);
#endif
}

// ------------------------------------------------
// AIX demux & stream code
// ------------------------------------------------
void AIX_Demuxer::Open(HANDLE _fp, u_long _stream_count, u_long total_size)
{
	fp = _fp;
	stream_count = _stream_count;

	stream = new AIXStream*[stream_count];
	for (u_long i = 0; i < stream_count; i++)
	{
		stream[i] = new AIXStream();
		stream[i]->parent = this;
		stream[i]->stream_id = i;
		stream[i]->MakeBuffer(total_size / stream_count);
		stream[i]->is_aix = 1;
	}

#if STR_AIX_CACHING
	InitCache();
#endif

#if AIX_SEGMENTED
	// request the necessary amount of data for headers and initial buffer filling
	RequestData(4);
#else
	AIX_CHUNK chunk;
	AIXP_HEADER aixp;

	AIXStream* s;

	for (int i = 0, loop = 1; loop;)
	{
		Read(&chunk, sizeof(chunk));

		switch (chunk.type)
		{
		case 'P':
			Read(&aixp, sizeof(aixp));
			s = stream[aixp.stream_id];
			Read(&s->data[s->cached], chunk.next.dw() - sizeof(aixp));
			s->cached += chunk.next.dw() - sizeof(aixp);
			break;
		case 'E':
			loop = 0;
		}
	}
#endif
}

void AIX_Demuxer::Close()
{
	if (stream_count)
	{
		if(stream)
			delete[] stream;
		stream = nullptr;
		stream_count = 0;
	}

	CloseHandle(fp);
	fp = INVALID_HANDLE_VALUE;
}

#if STR_AIX_CACHING
void AIX_Demuxer::InitCache()
{
	pos_cache = 0;
	ADXF_ReadFile(fp, cache, STREAM_CACHE_SIZE);
}
#endif

void AIX_Demuxer::Read(void* buffer, size_t size)
{
	BYTE* dst = (BYTE*)buffer;
#if STR_AIX_CACHING
	// cache overflow
	if (size + pos_cache > STREAM_CACHE_SIZE)
	{
		size_t remainder = STREAM_CACHE_SIZE - pos_cache;	// how much we can still read
		memcpy(dst, &cache[pos_cache], remainder);			// send whatever is left in the buffer
		dst += remainder;									// skip what is stored
		size -= remainder;
		// check if next read fits in the cache
		if (size > STREAM_CACHE_SIZE)
		{
			size_t blocks = size / STREAM_CACHE_SIZE;
			for (u_long i = 0; i < blocks; i++, dst += STREAM_CACHE_SIZE, size -= STREAM_CACHE_SIZE)
				ADXF_ReadFile(fp, dst, STREAM_CACHE_SIZE);
		}
		// refill the cache
		pos_cache = 0;
		ADXF_ReadFile(fp, cache, STREAM_CACHE_SIZE);		// cache more
	}

	if (size)
	{
		memcpy(dst, &cache[pos_cache], size);
		pos_cache += size;
	}
#else
	ADXF_ReadFile(fp, buffer, size);
#endif
}

void AIX_Demuxer::Skip(size_t size)
{
	DWORD read;
	for (size_t i = 0; i < size; i += 4)
		Read(&read, 4);
}

void AIX_Demuxer::RequestData(u_long count)
{
#if AIX_SEGMENTED
	AIX_CHUNK chunk;
	AIXP_HEADER aixp;
	AIXStream* s;

	for (u_long i = 0; i < stream_count * count; i++)
	{
		Read(&chunk, sizeof(chunk));

		switch (chunk.type)
		{
		case 'P':
			Read(&aixp, sizeof(aixp));
			s = stream[aixp.stream_id];
			if (s)
			{
				Read(&s->data[s->cached], chunk.next.dw() - sizeof(aixp));
				s->cached += chunk.next.dw() - sizeof(aixp);
			}
			else
				Skip(chunk.next.dw() - sizeof(aixp));
			break;
		case 'E':	// end parsing
			return;
		}
	}
#endif
}

// ------------------------------------------------
// AIX stream, behaves the same as ADX streams but
// from memory
// ------------------------------------------------
u_long AIXStream::Decode(int16_t* buffer, unsigned samples_needed, bool looping_enabled)
{
	return ADXDEC_Decode(this, buffer, samples_needed, looping_enabled);
}

void AIXStream::Read(void* buffer, size_t size)
{
#if !AIX_SEGMENTED
	memcpy(buffer, &data[pos], size);
	pos += size;
#else
	// we're reading ahead of what's cached from AIX
	if (pos + size > cached)
	{
		Lock();
		parent->RequestData(1);	// read until filled enough
		Unlock();
	}
	// still inside cache, just copy over
	memcpy(buffer, &data[pos], size);
	pos += size;
#endif
}

void AIXStream::Seek(u_long _pos, u_long mode)
{
	UNREFERENCED_PARAMETER(mode);

#if !AIX_SEGMENTED
	pos = _pos;
#else
	if (_pos > cached)
	{
		Lock();
		while (_pos > cached)
			parent->RequestData(1);	// request until we're good
		Unlock();
	}
	pos = _pos;
#endif
}

void AIXStream::Lock()
{
	EnterCriticalSection(&parent->crit);
}

void AIXStream::Unlock()
{
	LeaveCriticalSection(&parent->crit);
}
