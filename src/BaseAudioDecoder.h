#pragma once
#include "stdafx.h"
#include <cstddef>
#include "BasePipe.h"


#define INPUT_RAWDATA_BUFFER_LEN 4096
#define OUTPUT_PCM_BUFFER_LEN 4096

class BaseAudioDecoder {
public:
	virtual bool run(
			BasePipe<unsigned char, INPUT_RAWDATA_BUFFER_LEN> *input_rawdata_pipe,
			BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *output_pcm_pipe
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



