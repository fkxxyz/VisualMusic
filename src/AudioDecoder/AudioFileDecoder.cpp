
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


void *AudioFileDecoder::thread_file_read_proc(void *pthis){
	AudioFileDecoder &obj = *reinterpret_cast<AudioFileDecoder *>(pthis);

	while (1){
		obj.m_event_thread_file_read_watting.set();
		obj.m_event_enable_running.wait();
		obj.m_event_thread_file_read_watting.reset();

		assert(obj.m_ifs);
		while (1) {

			// Read from the file.
			unsigned char buffer[INPUT_RAWDATA_BUFFER_LEN];
			obj.m_ifs->read(reinterpret_cast<char *>(buffer), INPUT_RAWDATA_BUFFER_LEN);
			std::streamsize gcount = obj.m_ifs->gcount();
			if (gcount == 0) break;

			// Write to the pipe.
			size_t count = static_cast<size_t>(gcount);
			count = obj.m_input_rawdata_pipe.Write(buffer, count);
			if (count == 0) break;
		}
		obj.m_input_rawdata_pipe.NotifyEnd();

		obj.m_event_enable_running.reset();
	}

	return nullptr;
}

void *AudioFileDecoder::thread_decode_proc(void *pthis){
	AudioFileDecoder &obj = *reinterpret_cast<AudioFileDecoder *>(pthis);

	while (1){
		obj.m_event_thread_decode_watting.set();
		obj.m_event_enable_running.wait();
		obj.m_event_thread_decode_watting.reset();

		for (
			 obj.m_current_decoder = obj.m_decoders[0];
			 obj.m_current_decoder;
			 obj.m_current_decoder++
			 )
			if (obj.m_current_decoder->run(
					&obj.m_input_rawdata_pipe,
					&obj.m_output_pcm_pipe,
					&obj.m_event_meta_nodify
					)
				)
				break;

		obj.m_event_meta_nodify.set();

		obj.m_event_enable_running.reset();
	}
	return nullptr;
}

