#pragma once
#include "stdafx.h"
#include "WavePlayer.h"
#include <cassert>


#ifdef linux
#include <alsa/asoundlib.h>


inline WavePlayer::WavePlayer()
	:m_handle(nullptr)
{

}

inline bool WavePlayer::Open(unsigned int channels, unsigned int sample_rate, enum format format){
	assert(m_handle == nullptr);

	m_channels = channels;

	snd_pcm_format_t pcm_format;
	switch (format){
	case fm_uchar:
		pcm_format = SND_PCM_FORMAT_S8;
		break;
	case fm_short:
		pcm_format = SND_PCM_FORMAT_S16_LE;
		break;
	case fm_float:
		pcm_format = SND_PCM_FORMAT_FLOAT_LE;
		break;
	default:
		return false;
	}
	m_format = format;

	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);

	if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
		return false;
	m_handle = handle;

	snd_pcm_hw_params_t *m_params;
	snd_pcm_hw_params_malloc(&m_params);

	if (snd_pcm_hw_params_any(handle, m_params) < 0)
		goto fail_free;

	if (snd_pcm_hw_params_set_access(handle, m_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
		goto fail_free;

	if (snd_pcm_hw_params_set_format(handle, m_params, pcm_format) < 0)
		goto fail_free;

	if (snd_pcm_hw_params_set_channels(handle, m_params, channels) < 0)
		goto fail_free;

	{
		int dir = 0;
		if (snd_pcm_hw_params_set_rate_near(handle, m_params, &sample_rate, &dir) < 0)
			goto fail_free;
	}

	if (snd_pcm_hw_params(handle, m_params) < 0)
		goto fail_free;

	snd_pcm_hw_params_free(m_params);
	return true;

fail_free:
	snd_pcm_hw_params_free(m_params);
	return false;
}

inline bool WavePlayer::Play(void *data, size_t length){
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	return snd_pcm_writei(handle, data, length) > 0;
}

inline bool WavePlayer::Play(unsigned char *data, size_t length){
	assert(m_handle);
	assert(m_format == fm_uchar);
	return Play(reinterpret_cast<void *>(data), length);
}

inline bool WavePlayer::Play(short int *data, size_t length){
	assert(m_handle);
	assert(m_format == fm_short);
	return Play(reinterpret_cast<void *>(data), length);
}

inline bool WavePlayer::Play(float *data, size_t length){
	assert(m_handle);
	assert(m_format == fm_float);
	return Play(reinterpret_cast<void *>(data), length);
}

inline enum WavePlayer::status WavePlayer::GetStatus(){
	if (!m_handle)
		return st_closed;

	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	switch (snd_pcm_state(handle)){
	case SND_PCM_STATE_OPEN:
		return st_opened;
	case SND_PCM_STATE_RUNNING:
		return st_playing;
	case SND_PCM_STATE_PAUSED:
		return st_pause;
	default:
		assert(0);
	}
}

inline bool WavePlayer::Pause(){
	assert(m_handle);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	return snd_pcm_wait(handle, -1) == 0;
}

inline bool WavePlayer::Resume(){
	assert(m_handle);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	return snd_pcm_resume(handle) == 0;
}

inline bool WavePlayer::Stop(){
	assert(m_handle);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	return snd_pcm_drop(handle) == 0;
}

inline bool WavePlayer::Close(){
	assert(m_handle);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	return snd_pcm_close(handle) == 0;
}


#endif
