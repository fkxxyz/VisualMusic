#pragma once
#include "stdafx.h"
#include "WavePlayer.h"

#ifdef linux
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <cassert>
#include <unistd.h>

namespace WavePlayer_linux {
	typedef struct {
		size_t written;
		size_t last_pos;
		__int64_t last_pos_us;
	} pdata_t;

	inline __int64_t get_current_us(){
		struct timeval tv;
		struct timezone tz;
		gettimeofday(&tv,&tz);
		return tv.tv_sec * 1000000 + tv.tv_usec;
	}
}

inline WavePlayer::WavePlayer()
	:m_handle(nullptr)
{

}

inline bool WavePlayer::Open(unsigned int channels, unsigned int sample_rate, enum format format){
	WavePlayer_linux::pdata_t *pdata = reinterpret_cast<WavePlayer_linux::pdata_t *>(m_pdata);
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
	case fm_double:
		pcm_format = SND_PCM_FORMAT_FLOAT64_LE;
		break;
	default:
		return false;
	}
	m_format = format;
	m_sample_rate = sample_rate;

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
	pdata->written = 0;
	pdata->last_pos = 0;
	pdata->last_pos_us = WavePlayer_linux::get_current_us();
	return true;

fail_free:
	snd_pcm_hw_params_free(m_params);
	return false;
}

inline bool WavePlayer::Write(void *data, size_t length){
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	WavePlayer_linux::pdata_t *pdata = reinterpret_cast<WavePlayer_linux::pdata_t *>(m_pdata);
	snd_pcm_sframes_t result = snd_pcm_writei(handle, data, length);
	if (result < 0){
		snd_pcm_recover(handle, static_cast<int>(result), 0);
		result = snd_pcm_writei(handle, data, length);
		if (result < 0){
			assert(0);
		}
	}
	pdata->written += length;
	return result == static_cast<snd_pcm_sframes_t>(length);
}

inline bool WavePlayer::Write(unsigned char *data, size_t length){
	enum status status = GetStatus();
	assert(status == st_opened || status == st_playing);
	assert(m_format == fm_uchar);
	return Write(reinterpret_cast<void *>(data), length);
}

inline bool WavePlayer::Write(short int *data, size_t length){
	enum status status = GetStatus();
	assert(status == st_opened || status == st_playing);
	assert(m_format == fm_short);
	return Write(reinterpret_cast<void *>(data), length);
}

inline bool WavePlayer::Write(float *data, size_t length){
	enum status status = GetStatus();
	assert(status == st_opened || status == st_playing);
	assert(m_format == fm_float);
	return Write(reinterpret_cast<void *>(data), length);
}

inline bool WavePlayer::Write(double *data, size_t length){
	enum status status = GetStatus();
	assert(status == st_opened || status == st_playing);
	assert(m_format == fm_double);
	return Write(reinterpret_cast<void *>(data), length);
}

inline size_t WavePlayer::GetPos(){
	assert(m_handle);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	WavePlayer_linux::pdata_t *pdata = reinterpret_cast<WavePlayer_linux::pdata_t *>(m_pdata);

	snd_pcm_sframes_t sframes;
	if (snd_pcm_delay(handle, &sframes) == 0)
		return pdata->written - static_cast<size_t>(sframes);
	else
		return static_cast<size_t>(-1);
}

inline bool WavePlayer::Join(){
	assert(GetStatus() == st_playing);

	const int test_interval = 16; // millseconds
	const double rate_err = 0.95;

	WavePlayer_linux::pdata_t *pdata = reinterpret_cast<WavePlayer_linux::pdata_t *>(m_pdata);

	while (1){
		__int64_t cur_us = WavePlayer_linux::get_current_us();
		size_t cur_pos = GetPos();

		if (pdata->last_pos)
			if (cur_us - pdata->last_pos_us > test_interval * 1000)
				if (static_cast<double>(cur_pos - pdata->last_pos) / (static_cast<double>(cur_us - pdata->last_pos_us)/1000000) < m_sample_rate * rate_err)
					break;

		pdata->last_pos = cur_pos;
		pdata->last_pos_us = cur_us;

		usleep(test_interval * 1000);
	}
	return true;
}

inline enum WavePlayer::status WavePlayer::GetStatus(){
	if (!m_handle)
		return st_closed;

	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	snd_pcm_state_t state = snd_pcm_state(handle);
	switch (state){
	case SND_PCM_STATE_OPEN: case SND_PCM_STATE_PREPARED: case SND_PCM_STATE_XRUN:
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
	assert(GetStatus() == st_playing);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	return snd_pcm_pause(handle, 1) == 0;
}

inline bool WavePlayer::Resume(){
	assert(GetStatus() == st_pause);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	return snd_pcm_pause(handle, 0) == 0;
	//return snd_pcm_resume(handle) == 0;
}

inline bool WavePlayer::Stop(){
	assert(GetStatus() == st_playing);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	if (snd_pcm_drop(handle) != 0)
		return false;
	return snd_pcm_prepare(handle) == 0;
}

inline bool WavePlayer::Close(){
	assert(GetStatus() == st_opened);
	assert(m_handle);
	snd_pcm_t *handle = reinterpret_cast<snd_pcm_t *>(m_handle);
	return snd_pcm_close(handle) == 0;
}


#endif
