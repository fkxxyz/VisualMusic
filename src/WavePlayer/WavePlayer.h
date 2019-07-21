#pragma once
#include "stdafx.h"
#include <cstddef>

class WavePlayer {
public:
	enum format {
		fm_null = 0,
		fm_uchar = 8,
		fm_short = 16,
		fm_float = 32
	};
	enum status {
		st_closed,
		st_opened,
		st_playing,
		st_pause
	};

	WavePlayer();
	bool Open(unsigned int channels = 2, unsigned int sample_rate = 44100, enum format format = fm_short);
	bool Play(unsigned char *data, size_t length);
	bool Play(short int *data, size_t length);
	bool Play(float *data, size_t length);
	bool Pause();
	bool Resume();
	enum status GetStatus();
	bool Stop();
	bool Close();

protected:
	void *m_handle;
	enum format m_format;
	unsigned int m_channels;

	bool Play(void *data, size_t length);
};

#include "WavePlayer_windows.inl"
#include "WavePlayer_linux.inl"



