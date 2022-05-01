#pragma once

#define STR_ADX_CACHING		1		// caching switch for ADX
#define STR_AIX_CACHING		1		// caching switch for AIX
#define STREAM_CACHE_SIZE	2048

#define AIX_SEGMENTED		1		// set to 1 for caching on demand

// file helpers
HANDLE ADXF_OpenFile(const char* filename);
void   ADXF_CloseFile(HANDLE fp);
u_long ADXF_ReadFile(HANDLE fp, void* buffer, size_t size);
u_long ADXF_Tell(HANDLE fp);
u_long ADXF_Seek(HANDLE fp, LONG pos, u_long mode);

// generic streaming interface
class CriFileStream
{
public:
	CriFileStream() :
		is_aix(0),
		copyright_offset(0),
		block_size(0),
		sample_bitdepth(0),
		channel_count(0),
		sample_rate(0),
		total_samples(0),
		highpass_frequency(0),
		loop_enabled(0),
		loop_start_index(0),
		loop_end_index(0),
		past_samples{ 0 },
		sample_index(0),
		coefficient{ 0, 0 }
	{}
	virtual ~CriFileStream() {}

	virtual void Read(void* buffer, size_t size)
	{
		UNREFERENCED_PARAMETER(buffer);
		UNREFERENCED_PARAMETER(size);
	}
	virtual void Seek(u_long pos, u_long mode)
	{
		UNREFERENCED_PARAMETER(pos);
		UNREFERENCED_PARAMETER(mode);
	}
	virtual u_long Decode(int16_t* buffer, unsigned samples_needed, bool looping_enabled)
	{
		UNREFERENCED_PARAMETER(buffer);
		UNREFERENCED_PARAMETER(samples_needed);
		UNREFERENCED_PARAMETER(looping_enabled);
		return 0;
	}

	u_long is_aix;
	// attributes
	u_long copyright_offset,
		block_size,
		sample_bitdepth,
		channel_count,
		sample_rate,
		total_samples,
		highpass_frequency,
		loop_enabled,
		loop_start_index,
		loop_end_index;
	// decoding fields
	short past_samples[2][2];
	short coefficient[2];
	u_long sample_index;
};

// WAV streaming interface, for AFS voices
class WAVStream : public CriFileStream
{
public:
	WAVStream() : fp(INVALID_HANDLE_VALUE),
		start(0),
		fmt{0},
		wav_size(0),
		wav_pos(0),
		type(0),
		looped(0)
	{
	}

	~WAVStream()
	{
		if (fp != INVALID_HANDLE_VALUE)
			CloseHandle(fp);
	}

	int Open(HANDLE fp, u_long pos);

	virtual void Read(void* buffer, size_t size)
	{
		UNREFERENCED_PARAMETER(buffer);
		UNREFERENCED_PARAMETER(size);
	}
	virtual void Seek(u_long pos, u_long mode)
	{
		UNREFERENCED_PARAMETER(pos);
		UNREFERENCED_PARAMETER(mode);
	}
	virtual u_long Decode(int16_t* buffer, unsigned samples_needed, bool looping_enabled);

	HANDLE fp;
	u_long start;

private:
	typedef struct WAV_riff
	{
		unsigned long ChunkID,
			ChunkSize,
			Format;
	} wav_riff;

	typedef struct WAV_fmt
	{
		unsigned short AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 
		unsigned short NumOfChan;      // Number of channels 1=Mono 2=Sterio                   
		unsigned long  SamplesPerSec;  // Sampling Frequency in Hz                             
		unsigned long  bytesPerSec;    // bytes per second 
		unsigned short blockAlign;     // 2=16-bit mono, 4=16-bit stereo 
		unsigned short bitsPerSample;  // Number of bits per sample      
	} wav_fmt;

	typedef struct WAV_chunk
	{
		DWORD magic,
			size;
	} wav_chunk;

	typedef struct sample
	{
		DWORD cue_point_id,
			type,
			start,
			end,
			fraction,
			play_count;
	} sample;

	typedef struct SMPL_chunk
	{
		DWORD Manufacturer,
			Product,
			Sample_period,
			MIDI_unity_note,
			MIDI_pitch_fraction,
			SMPTE_format,
			SMPTE_offset,
			Num_sample_loops,
			Sampler_data;
	} smpl_chunk;

	enum
	{
		Type_PCM,
		Type_FLOAT
	};

	int find_data();
	void pcm_read(BYTE* dst, size_t samples);
	void pcm_seek(size_t sample_pos);
	size_t pcm_tell();

	void fill(BYTE* dst, DWORD size);

	wav_fmt fmt;
	size_t wav_size,
		wav_pos;
	int type, looped;
};

// ADX streaming interface, very simple
class ADXStream : public CriFileStream
{
public:
	ADXStream() : fp(nullptr),
		start(0)
#if STR_ADX_CACHING
		, cache{0},
		pos_cache(0),
		last_pos((u_long)-1)
#endif
	{}

	virtual ~ADXStream()
	{
		Close();
	}

	int Open(const char* filename);
	int Open(HANDLE fp, u_long pos);
	void Close();

	virtual void Read(void* buffer, size_t size);
	virtual void Seek(u_long pos, u_long mode);
	virtual u_long Decode(int16_t* buffer, unsigned samples_needed, bool looping_enabled);

	HANDLE fp;
	u_long start;
#if STR_ADX_CACHING
	BYTE cache[STREAM_CACHE_SIZE];
	u_long pos_cache, last_pos;
#endif
};

// AIX streaming interface, a hell of caching and deinterleaving
class AIXStream;

class AIX_Demuxer
{
public:
	AIX_Demuxer() : fp(INVALID_HANDLE_VALUE),
		stream(nullptr),
		stream_count(0)
#if STR_AIX_CACHING
		, cache{0},
		pos_cache(0)
#endif
	{

	}

	virtual ~AIX_Demuxer()
	{
		Close();
	}

	void Open(HANDLE fp, u_long stream_count, u_long total_size);
	void Close();
	void Read(void* buffer, size_t size);

#if STR_AIX_CACHING
	void InitCache();
#endif
	void RequestData(u_long count);

	HANDLE fp;
	AIXStream** stream;
	u_long stream_count;
#if STR_AIX_CACHING
	BYTE cache[STREAM_CACHE_SIZE];
	u_long pos_cache;
#endif
};

class AIXStream : public CriFileStream
{
public:
	AIXStream() : parent(nullptr),
		stream_id(0),
		pos(0),
		cached(0),
		data(nullptr)
	{
	}
	virtual ~AIXStream()
	{
		pos = 0;
		if (data)
		{
			delete[] data;
			data = nullptr;
		}
	}

	void MakeBuffer(u_long size)
	{
		data = new BYTE[size];
	}

	virtual void Read(void* buffer, size_t size);
	virtual void Seek(u_long pos, u_long mode);
	virtual u_long Decode(int16_t* buffer, unsigned samples_needed, bool looping_enabled);

	AIX_Demuxer* parent;
	u_long stream_id,
		pos,
		cached;
	BYTE* data;
};
