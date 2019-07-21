
#include "stdafx.h"
#include "AudioFileDecoder.h"

AudioFileDecoder::AudioFileDecoder():
	m_ifs(nullptr),
	m_decoder(nullptr)
{

}

BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *AudioFileDecoder::GetPCMPipe() {
	return &m_output_pcm_pipe;
}

bool AudioFileDecoder::Run(const char *file_path){
	m_ifs = new std::ifstream(file_path, std::ios::binary);
	if (!*m_ifs){
		delete m_ifs;
		m_ifs = nullptr;
		return false;
	}

	pthread_create(&m_thread_file_read, nullptr, thread_file_read_proc, this);
	pthread_create(&m_thread_decode, nullptr, thread_decode_proc, this);

	return true;
}

void AudioFileDecoder::Stop(){
	if (!m_ifs)
		return;

	assert(*m_ifs);
	assert(m_thread_file_read);
	assert(m_thread_decode);
	assert(m_decoder);

	m_decoder->stop();
	m_input_rawdata_pipe.Clear();

	void *status;
	pthread_join(m_thread_file_read, &status);
	pthread_join(m_thread_decode, &status);

	assert(!m_ifs);
	assert(!m_decoder);
}

void *AudioFileDecoder::thread_file_read_proc(void *pthis){
	AudioFileDecoder &obj = *reinterpret_cast<AudioFileDecoder *>(pthis);

	unsigned char buffer[INPUT_RAWDATA_BUFFER_LEN];

	size_t count;
	do {
		obj.m_ifs->read(reinterpret_cast<char *>(buffer), INPUT_RAWDATA_BUFFER_LEN);
		std::streamsize gcount = obj.m_ifs->gcount();
		if (gcount == 0)
			break;
		count = static_cast<size_t>(gcount);
		cout<<"file read "<<count<<" bytes"<<endl;
		count = obj.m_input_rawdata_pipe.Write(buffer, count);
	} while (count > 0);
}

void *AudioFileDecoder::thread_decode_proc(void *pthis){
	AudioFileDecoder &obj = *reinterpret_cast<AudioFileDecoder *>(pthis);

	obj.m_decoder = new Mp3Decoder();
	if (obj.m_decoder->run(&obj.m_input_rawdata_pipe, &obj.m_output_pcm_pipe))
		goto end_clean;

	/*
	delete pthis->m_decoder;
	pthis->m_decoder = new WavDecoder();
	if (pthis->m_decoder->run(&pthis->m_input_rawdata_pipe, &pthis->m_output_pcm_pipe))
		goto end_clean;
	*/

end_clean:
	obj.m_ifs->close();
	delete obj.m_ifs;
	obj.m_ifs = nullptr;

	delete obj.m_decoder;
	obj.m_decoder = nullptr;
}

