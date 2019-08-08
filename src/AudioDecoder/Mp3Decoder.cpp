
#include "stdafx.h"
#include "Mp3Decoder.h"
#include <cassert>
#include <cstddef>
#include <cstring>

bool Mp3Decoder::run(
		BasePipe<unsigned char, INPUT_RAWDATA_BUFFER_LEN> *input_rawdata_pipe,
		BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *output_pcm_pipe,
		event_t *event_meta_nodify
		)
{
	m_input_rawdata_pipe = input_rawdata_pipe;
	m_output_pcm_pipe = output_pcm_pipe;
	m_event_meta_nodify = event_meta_nodify;

	struct mad_decoder decoder;
	mad_decoder_init(&decoder, this, mad_input_func, nullptr, nullptr, mad_output_func, mad_error_func, nullptr);
	int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	mad_decoder_finish(&decoder);

	m_output_pcm_pipe->NotifyEnd();

	m_stop_flag = false;

	return m_channels && result == MAD_ERROR_NONE;
}

mad_flow Mp3Decoder::mad_input_func(void *data, struct mad_stream *stream){
	Mp3Decoder &mp3_decoder = *reinterpret_cast<Mp3Decoder *>(data);

	if (mp3_decoder.m_stop_flag)
		return MAD_FLOW_STOP;

	size_t rem_size = static_cast<size_t>(stream->bufend - stream->next_frame);
	memcpy(mp3_decoder.m_read_buffer, stream->next_frame, rem_size);

	size_t count = mp3_decoder.m_input_rawdata_pipe->Read(
					   mp3_decoder.m_read_buffer + rem_size,
					   1,
					   INPUT_RAWDATA_BUFFER_LEN - rem_size
					   );

	mad_stream_buffer(stream, mp3_decoder.m_read_buffer, rem_size + count);

	if (count == 0)
		return MAD_FLOW_STOP;

	return MAD_FLOW_CONTINUE;
}

mad_flow Mp3Decoder::mad_header_func(void *, struct mad_header const *){
	return MAD_FLOW_CONTINUE;
}

mad_flow Mp3Decoder::mad_output_func(void *data, struct mad_header const *header, struct mad_pcm *pcm){
	Mp3Decoder &mp3_decoder = *reinterpret_cast<Mp3Decoder *>(data);

	assert(pcm->channels == 1 || pcm->channels == 2);

	if (mp3_decoder.m_stop_flag)
		return MAD_FLOW_STOP;

	if (mp3_decoder.m_channels == 0){
		mp3_decoder.m_channels = pcm->channels;
		mp3_decoder.m_sample_rate = pcm->samplerate;
		mp3_decoder.m_sizeof_sample = sizeof(int);
		mp3_decoder.m_typeof_sample = st_int;

		mp3_decoder.m_data_start = 0;
		mp3_decoder.m_bitrate = header->bitrate;

		mp3_decoder.m_event_meta_nodify->set();
	}


	if (pcm->channels == 1)
		mp3_decoder.m_output_pcm_pipe->Write(
					reinterpret_cast<unsigned char *>(pcm->samples[0]),
					pcm->length * sizeof(mad_fixed_t)
					);
	else {
		mad_fixed_t samples[2*1152];
		mad_fixed_t *p1 = pcm->samples[0], *p2 = pcm->samples[1],
				*p1_end = p1 + pcm->length,
				*ps = samples;
		while (p1 < p1_end){
			*(ps++) = *p2;
			*(ps++) = *p1;
			p1++;
			p2++;
		}
		mp3_decoder.m_output_pcm_pipe->Write(
					reinterpret_cast<unsigned char *>(samples),
					pcm->length * 2 * sizeof(mad_fixed_t)
					);
	}
	return MAD_FLOW_CONTINUE;
}

mad_flow Mp3Decoder::mad_error_func(void *, struct mad_stream *stream, struct mad_frame *){
	if (stream->error){
		if(!MAD_RECOVERABLE(stream->error)){
			assert(0);
			return MAD_FLOW_STOP;
		} else{
			//assert(0);
			return MAD_FLOW_CONTINUE;
		}
	}
	return MAD_FLOW_CONTINUE;
}


