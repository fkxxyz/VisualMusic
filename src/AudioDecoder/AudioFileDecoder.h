#pragma once
#include "stdafx.h"

#include "BaseAudioDecoder.h"
#include <fstream>
#include "thread/thread.h"

class AudioFileDecoder {
public:
	AudioFileDecoder();
	bool open(const char *file_path);
	void close();
	bool run();
	void stop();
	bool set_pos(int milliseconds);

	enum status {
		st_closed,
		st_opened,
		st_running
	};
	enum status get_status() const;

	BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *GetPCMPipe();

	unsigned int GetChanners() const { assert (m_current_decoder); return m_current_decoder->GetChanners();}
	unsigned int GetSampleRate() const { assert (m_current_decoder); return m_current_decoder->GetSampleRate();}
	size_t GetSizeofSample() const { assert (m_current_decoder); return m_current_decoder->GetSizeofSample();}
	enum BaseAudioDecoder::sample_type GetTypeofSample() const { assert (m_current_decoder); return m_current_decoder->GetTypeofSample();}

protected:
	static void *thread_file_read_proc(void *pthis);
	static void *thread_decode_proc(void *pthis);

	event_t m_event_enable_running;

	event_t m_event_thread_file_read_watting;
	event_t m_event_thread_decode_watting;

	std::ifstream *m_ifs;
	static BaseAudioDecoder *const *m_decoders;
	BaseAudioDecoder *m_current_decoder;

	thread_t m_thread_file_read;
	thread_t m_thread_decode;

	BasePipe<unsigned char, INPUT_RAWDATA_BUFFER_LEN> m_input_rawdata_pipe;
	BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> m_output_pcm_pipe;

	event_t m_event_meta_nodify;
};

inline AudioFileDecoder::AudioFileDecoder():
	m_ifs(nullptr)
{
	if (!m_thread_file_read.start(thread_file_read_proc, this))
		assert(0);
	if (!m_thread_decode.start(thread_decode_proc, this))
		assert(0);
}

inline bool AudioFileDecoder::open(const char *file_path){
	assert(get_status() == st_closed);

	m_ifs = new std::ifstream(file_path, std::ios::binary);

	assert(get_status() == st_opened);
	return static_cast<bool>(*m_ifs);
}

inline void AudioFileDecoder::close(){
	assert(get_status() == st_opened);

	delete m_ifs;
	m_ifs = nullptr;

	assert(get_status() == st_closed);
}

inline BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *AudioFileDecoder::GetPCMPipe() {
	return &m_output_pcm_pipe;
}

inline bool AudioFileDecoder::run(){
	assert(get_status() == st_opened);

	m_event_meta_nodify.reset();

	m_event_enable_running.set();

	if (!m_event_meta_nodify.wait()){
		assert(0);
		return false;
	}

	if (!m_current_decoder)
		return false;

	assert(get_status() == st_running);
	return true;
}

inline void AudioFileDecoder::stop(){
	assert(get_status() == st_running);

	m_input_rawdata_pipe.Clear();
	m_current_decoder->stop();

	m_event_thread_file_read_watting.wait();
	m_event_thread_decode_watting.wait();

	assert(get_status() == st_opened);
}

inline bool AudioFileDecoder::set_pos(int milliseconds){
	assert(get_status() != st_closed);
	enum status status = get_status();
	if (status == st_running)
		stop();

	m_ifs->seekg(static_cast<long>(
				m_current_decoder->GetDataStart() +
				static_cast<unsigned long>(
						 milliseconds
					) * (m_current_decoder->GetBitrate() / 8) / 1000
				));
	if (!m_ifs->good())
		return false;

	m_input_rawdata_pipe.Clear();
	m_output_pcm_pipe.Clear();

	if (status == st_running)
		return run();

	return true;
}

inline enum AudioFileDecoder::status AudioFileDecoder::get_status() const {
	if (m_ifs){
		assert(*m_ifs);
		if (m_event_enable_running.is_setted()){
			assert(!m_event_thread_file_read_watting.is_setted());
			assert(!m_event_thread_decode_watting.is_setted());
			return st_running;
		} else {
			assert(m_event_thread_file_read_watting.is_setted());
			assert(m_event_thread_decode_watting.is_setted());
			return st_opened;
		}
	} else {
		return st_closed;
	}
}





