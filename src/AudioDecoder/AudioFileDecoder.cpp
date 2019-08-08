
#include "stdafx.h"
#include "AudioFileDecoder.h"
#include "Mp3Decoder.h"
#include "WavDecoder.h"

static class AudioDecoders {
public:
	Mp3Decoder mp3;
} decoders;

static BaseAudioDecoder *const Decoder_Array[] = {
	&decoders.mp3,
	//&decoders.wav,
	nullptr
};

BaseAudioDecoder *const* AudioFileDecoder::m_decoders = Decoder_Array;

AudioFileDecoder::AudioFileDecoder():
	m_ifs(nullptr)
{

}

BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *AudioFileDecoder::GetPCMPipe() {
	return &m_output_pcm_pipe;
}

bool AudioFileDecoder::Run(const char *file_path){
	// Open file.
	m_ifs = new std::ifstream(file_path, std::ios::binary);
	if (!*m_ifs)
		goto fail_closefile;

	// Create file reading thread.
	if (!m_thread_file_read.start(thread_file_read_proc, this))
		goto fail_closefile;

	// Create decoding thread.
	{
		if (!m_thread_decode.start(thread_decode_proc, this))
			goto fail_closefile;

		if (!m_event_meta_nodify.wait()){
			assert(0);
			goto fail_closefile;
		}

		// decoding fail when m_current_decoder is nullptr.
		if (!m_current_decoder)
			return false;
	}

	return true;

fail_closefile:
	delete m_ifs;
	m_ifs = nullptr;
	return false;
}

void AudioFileDecoder::Stop(){
	if (!m_ifs)
		return;

	assert(*m_ifs);
	assert(m_current_decoder);

	m_current_decoder->stop();
	m_input_rawdata_pipe.Clear();

	m_thread_file_read.join();
	m_thread_decode.join();

	assert(!m_ifs);
	assert(!m_current_decoder);
}

bool AudioFileDecoder::SetPos(int milliseconds){
	assert(m_ifs);
	assert(*m_ifs);

	m_ifs->seekg(static_cast<long>(
				m_current_decoder->GetDataStart() +
				static_cast<unsigned long>(
						 milliseconds
					) * (m_current_decoder->GetBitrate() / 8) / 1000
				));

	m_input_rawdata_pipe.Clear();
	m_output_pcm_pipe.Clear();
	if (!m_ifs->good())
		return false;

	return true;
}

void *AudioFileDecoder::thread_file_read_proc(void *pthis){
	AudioFileDecoder &obj = *reinterpret_cast<AudioFileDecoder *>(pthis);

	unsigned char buffer[INPUT_RAWDATA_BUFFER_LEN];

	size_t count;
	do {
		assert(obj.m_ifs);
		obj.m_ifs->read(reinterpret_cast<char *>(buffer), INPUT_RAWDATA_BUFFER_LEN);
		std::streamsize gcount = obj.m_ifs->gcount();
		if (gcount == 0)
			break;
		count = static_cast<size_t>(gcount);
		count = obj.m_input_rawdata_pipe.Write(buffer, count);
	} while (count > 0);
	obj.m_input_rawdata_pipe.NotifyEnd();

	obj.m_ifs->close();
	delete obj.m_ifs;
	obj.m_ifs = nullptr;

	return nullptr;
}

void *AudioFileDecoder::thread_decode_proc(void *pthis){
	AudioFileDecoder &obj = *reinterpret_cast<AudioFileDecoder *>(pthis);

	for (obj.m_current_decoder = obj.m_decoders[0]; obj.m_current_decoder; obj.m_current_decoder++)
		if (obj.m_current_decoder->run(&obj.m_input_rawdata_pipe, &obj.m_output_pcm_pipe, &obj.m_event_meta_nodify))
			break;

	if (!obj.m_current_decoder)
		obj.m_event_meta_nodify.set();

	return nullptr;
}

