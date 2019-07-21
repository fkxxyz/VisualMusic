#pragma once
#include "stdafx.h"
#include "Mp3Decoder.h"
#include "WavDecoder.h"

#include <fstream>
#include <pthread.h>

class AudioFileDecoder {
public:
	AudioFileDecoder();
	bool Run(const char *file_path);
	void Stop();
	BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *GetPCMPipe();

protected:
	static void *thread_file_read_proc(void *pthis);
	static void *thread_decode_proc(void *pthis);

	std::ifstream *m_ifs;
	BaseAudioDecoder *m_decoder;

	pthread_t m_thread_file_read;
	pthread_t m_thread_decode;

	BasePipe<unsigned char, INPUT_RAWDATA_BUFFER_LEN> m_input_rawdata_pipe;
	BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> m_output_pcm_pipe;
};
