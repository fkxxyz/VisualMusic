#pragma once
#include "stdafx.h"
#include "WavePlayer.h"
#include <cassert>

#if defined(_WIN32) || defined(__WIN32__)

#include "WinPCMPlayer.h"

inline WavePlayer::WavePlayer()
{

}

inline bool WavePlayer::Open(unsigned int channels, unsigned int sample_rate, enum format format){
	assert(m_status == st_closed);

	m_channels = channels;
	m_format = format;

	m_handle = reinterpret_cast<void *>(new CWinPCMPlayer<>());
	CWinPCMPlayer<> &player = *reinterpret_cast<CWinPCMPlayer<> *>(m_handle);

	WAVEFORMATEX wFormat;
	WORD BitsPerSample = static_cast<WORD>(format);
	wFormat.wFormatTag = BitsPerSample == 32 ? 3 : WAVE_FORMAT_PCM;
	wFormat.nSamplesPerSec = sample_rate;
	wFormat.nChannels = static_cast<WORD>(channels);
	wFormat.wBitsPerSample = BitsPerSample;
	wFormat.nBlockAlign = wFormat.nChannels * (wFormat.wBitsPerSample / 8);
	wFormat.nAvgBytesPerSec = wFormat.nSamplesPerSec * wFormat.nBlockAlign;
	wFormat.cbSize = 0;
	player.Open(&wFormat);

	m_status = st_opened;
	return true;
}

inline bool WavePlayer::Play(void *data, size_t length){
	CWinPCMPlayer<> &player = *reinterpret_cast<CWinPCMPlayer<> *>(m_handle);
	return player.Insert(data, static_cast<DWORD>(length * m_channels * (m_format / 8))) == TRUE;
}

inline bool WavePlayer::Play(unsigned char *data, size_t length){
	assert(m_status == st_opened);
	assert(m_format == fm_uchar);
	return Play(reinterpret_cast<void *>(data), length);
}

inline bool WavePlayer::Play(short int *data, size_t length){
	assert(m_status == st_opened);
	assert(m_format == fm_short);
	return Play(reinterpret_cast<void *>(data), length);
}

inline bool WavePlayer::Play(float *data, size_t length){
	assert(m_status == st_opened);
	assert(m_format == fm_float);
	return Play(reinterpret_cast<void *>(data), length);
}

inline bool WavePlayer::Pause(){
	assert(m_status == st_playing);

	CWinPCMPlayer<> &player = *reinterpret_cast<CWinPCMPlayer<> *>(m_handle);
	return player.Pause() == TRUE;
}

inline bool WavePlayer::Resume(){
	assert(m_status == st_playing);

	CWinPCMPlayer<> &player = *reinterpret_cast<CWinPCMPlayer<> *>(m_handle);
	return player.Resume() == TRUE;
}

inline enum WavePlayer::status WavePlayer::GetStatus(){
	assert(m_status == st_playing);

	CWinPCMPlayer<> &player = *reinterpret_cast<CWinPCMPlayer<> *>(m_handle);
	return static_cast<enum WavePlayer::status>(player.GetStatus());
}

inline bool WavePlayer::Stop(){
	assert(m_status == st_playing);

	CWinPCMPlayer<> &player = *reinterpret_cast<CWinPCMPlayer<> *>(m_handle);
	return player.Reset() == TRUE;
}

inline bool WavePlayer::Close(){
	assert(m_status == st_opened);

	CWinPCMPlayer<> &player = *reinterpret_cast<CWinPCMPlayer<> *>(m_handle);
	return player.Close() == TRUE;
}

#endif
