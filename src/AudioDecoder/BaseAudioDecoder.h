#pragma once
#include "stdafx.h"
#include <cstddef>
#include "BasePipe.h"
#include <semaphore.h>


#define INPUT_RAWDATA_BUFFER_LEN 16384
#define OUTPUT_PCM_BUFFER_LEN 16384

class BaseAudioDecoder {
public:
	virtual bool run(
			BasePipe<unsigned char, INPUT_RAWDATA_BUFFER_LEN> *input_rawdata_pipe,
			BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *output_pcm_pipe,
			sem_t *sem  // Notify after reading audio metadata
			) = 0;
	void stop();

	BaseAudioDecoder();
	virtual ~BaseAudioDecoder();

	enum sample_type {
		st_void,
		st_uchar,
		st_short,
		st_int,
		st_float,
		st_double
	};

	unsigned int GetChanners() const { assert (m_channels); return m_channels;}
	unsigned int GetSampleRate() const { assert (m_channels); return m_sample_rate;}
	size_t GetSizeofSample() const { assert (m_channels); return m_sizeof_sample;}
	enum sample_type GetTypeofSample() const { assert (m_channels); return m_typeof_sample;}

protected:
	BasePipe<unsigned char, INPUT_RAWDATA_BUFFER_LEN> *m_input_rawdata_pipe;
	BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *m_output_pcm_pipe;

	unsigned int m_channels;
	unsigned int m_sample_rate;
	size_t m_sizeof_sample;
	enum sample_type m_typeof_sample;

	bool m_stop_flag;
};

inline void BaseAudioDecoder::stop(){
	m_stop_flag = true;
}



