#pragma once
#include "stdafx.h"
#include "BaseAudioDecoder.h"
#include <mad.h>

class Mp3Decoder : public BaseAudioDecoder {
public:
	virtual bool run(
			BasePipe<unsigned char, INPUT_RAWDATA_BUFFER_LEN> *input_rawdata_pipe,
			BasePipe<unsigned char, OUTPUT_PCM_BUFFER_LEN> *output_pcm_pipe,
			event_t *event_meta_nodify
			);
protected:
	static mad_flow mad_input_func(void *data, struct mad_stream *stream);
	static mad_flow mad_header_func(void *data, struct mad_header const *header);
	static mad_flow mad_output_func(void *data, struct mad_header const *header, struct mad_pcm *pcm);
	static mad_flow mad_error_func(void *data, struct mad_stream *stream, struct mad_frame *frame);

	unsigned char m_read_buffer[INPUT_RAWDATA_BUFFER_LEN];
	event_t *m_event_meta_nodify;
	bool m_is_first_frame;
};

