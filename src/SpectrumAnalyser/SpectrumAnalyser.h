#pragma once
#include "stdafx.h"
#include <pthread.h>
#include "constants.h"

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
class SpectrumAnalyser {
public:
	SpectrumAnalyser();
	~SpectrumAnalyser();

	void SetArguments(
			const double *freqs,
			const int freq_n,
			const double sample_rate,
			const int frame_sample_n
			);

	void Put(double *pcm, int length);
	int Get(double (*sepectrum)[MAX_FREQ_N]);
	int GetCountOfReserveFrame() const;

	void Clear();
	void NotifyEnd();

protected:
	int m_const_frame_sample_n;
	int m_const_freq_n;
	int m_const_reserve_frame_n;

	double m_input_pcm[MAX_FRAME_SAMPLE_N * FRAME_N];
	int m_input_pcm_length;

	struct LD {
		int length;
		double phase[MAX_FREQ_N];
		double bm_sin[MAX_FREQ_N][FRAME_N];
		double bm_cos[MAX_FREQ_N][FRAME_N];
	} ld[2];
	int ld_index;

	double m_output_sepectrum[FRAME_N][MAX_FREQ_N];
	int m_output_sepectrum_length;

	pthread_t m_thread;
	static void *thread_proc(void *pthis);
	void Analyse();

	pthread_cond_t m_cond_put, m_cond_get;
	pthread_mutex_t m_mutex_put, m_mutex_get;

	bool m_end_flag;

	bool m_clear_flag;
	void clear();

protected:
	double *sin_o, *cos_o;
	double m_n_cycle_samples_o[MAX_FREQ_N];
};

#include "SpectrumAnalyser.inl"

