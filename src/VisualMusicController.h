#pragma once
#include "stdafx.h"
#include "AudioDecoder/AudioFileDecoder.h"
#include "WavePlayer/WavePlayer.h"
#include <cassert>

class VisualMusicController {
public:
	bool StartDecoderThread(const char *audio_file_path){
		return m_decoder.Run(audio_file_path);
	}

	bool OpenWavePlayer(){
		assert(m_decoder.GetChanners());
		WavePlayer::format format;
		BaseAudioDecoder::sample_type type = m_decoder.GetTypeofSample();
		switch (type){
		case BaseAudioDecoder::st_uchar:
			format = WavePlayer::fm_uchar;
			break;
		case BaseAudioDecoder::st_short:
			format = WavePlayer::fm_short;
			break;
		case BaseAudioDecoder::st_int:
			format = WavePlayer::fm_float;
			break;
		case BaseAudioDecoder::st_float:
			format = WavePlayer::fm_float;
			break;
		default:
			assert(0);
		}
		return m_player.Open(m_decoder.GetChanners(), m_decoder.GetSampleRate(), format);
	}


protected:
	WavePlayer m_player;
	AudioFileDecoder m_decoder;
};

