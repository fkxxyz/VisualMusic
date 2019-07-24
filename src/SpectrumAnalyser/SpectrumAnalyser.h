#pragma once
#include "stdafx.h"
#include <pthread.h>
#include "constants.h"

template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
class SpectrumAnalyser {
public:
	SpectrumAnalyser(const double freqs[FREQ_N], const double sample_rate, const int time_slice_div);
	~SpectrumAnalyser();

	bool TurnOn();
	void TurnOff();

	void Put(double *pcm, int length);
	int Get(double *sepectrum[FREQ_N]);

	void Clear();

protected:
	const double m_const_sample_rate;
	const double m_const_freqs[FREQ_N];
	const double m_const_time_slice_div;

	double m_input_pcm[FRAME_SAMPLE_N * FRAME_N];
	int m_input_pcm_length;

	struct LD {
		int length;
		double phase;
		double bm_sin[FREQ_N][FRAME_N];
		double bm_cos[FREQ_N][FRAME_N];
	} ld[2];
	int ld_index;

	double m_output_sepectrum[FRAME_N][FREQ_N];
	int m_output_sepectrum_length;

	pthread_t m_thread;
	static void *thread_proc(void *pthis);
	void Analyse();

	pthread_cond_t m_cond_put, m_cond_get;
	pthread_mutex_t m_mutex_put, m_mutex_get;

protected:
	double *sin_o, *cos_o;
	double m_n_cycle_samples_o[FREQ_N];
};

#include "SpectrumAnalyser.inl"

