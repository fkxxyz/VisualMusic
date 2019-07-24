#pragma once
#include "stdafx.h"

#include "BaseAudioDecoder.h"
#include <fstream>
#include <pthread.h>

class AudioFileDecoder {
public:
	AudioFileDecoder();
	bool Run(const char *file_path);
	void Stop();
	BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *GetPCMPipe();

	unsigned int GetChanners() const { assert (m_current_decoder); return m_current_decoder->GetChanners();}
	unsigned int GetSampleRate() const { assert (m_current_decoder); return m_current_decoder->GetSampleRate();}
	size_t GetSizeofSample() const { assert (m_current_decoder); return m_current_decoder->GetSizeofSample();}
	enum BaseAudioDecoder::sample_type GetTypeofSample() const { assert (m_current_decoder); return m_current_decoder->GetTypeofSample();}

protected:
	static void *thread_file_read_proc(void *pthis);
	static void *thread_decode_proc(void *pthis);

	std::ifstream *m_ifs;
	static BaseAudioDecoder *const *m_decoders;
	BaseAudioDecoder *m_current_decoder;

	pthread_t m_thread_file_read;
	pthread_t m_thread_decode;

	BasePipe<unsigned char, INPUT_RAWDATA_BUFFER_LEN> m_input_rawdata_pipe;
	BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> m_output_pcm_pipe;

	sem_t m_sem_meta_nodify;
};