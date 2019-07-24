
#include "stdafx.h"
#include "SpectrumAnalyser.h"
#include <cassert>
#include <cstring>
#include <cmath>


template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::SpectrumAnalyser(
		const double freqs[FREQ_N], const double sample_rate, const int time_slice_div):
	m_const_sample_rate(sample_rate),
	m_const_freqs(freqs),
	m_const_time_slice_div(time_slice_div)
{
	assert(MIN_WIN_N_VIB <= MAX_WIN_N_VIB);
	assert(MIN_WIN_N_VIB > 0);

	assert(FRAME_N % 2 == 0);

	// Make sure the frequency array is strictly increasing.
	for (int i = 0; i < FREQ_N - 1; i++)
		assert(freqs[i] < freqs[i+1]); // Please sort it.

	// Calculate the number of samples included in each cycle at each frequency
	for (int i = 0; i < FREQ_N; i++)
		m_n_cycle_samples_o[i] = static_cast<double>(i)/(m_const_sample_rate / m_const_freqs[i])

	// It must be ensured that the lowest frequency can vibrate
	//   for the specified minimum period within a given number of samples.
	assert(FRAME_SAMPLE_N * FRAME_N / 2 >= m_n_cycle_samples_o[0] * MIN_WIN_N_VIB);

	Clear();
	ld_index = 0;

	m_input_pcm_length = 0;
	m_output_sepectrum_length = 0;

	pthread_cond_init(&m_cond_put, nullptr);
	pthread_cond_init(&m_cond_get, nullptr);
	pthread_mutex_init(&m_mutex_put, nullptr);
	pthread_mutex_init(&m_mutex_get, nullptr);

	// Calculate trigonometric optimization values
	sin_o = new double [MAX_N_DIV_CIRCLE * 2];
	assert(sin_o);
	cos_o = sin_o + MAX_N_DIV_CIRCLE;
	for (int i = 0; i < MAX_N_DIV_CIRCLE; i++){
		double angle = (2 * M_PI) * (static_cast<double>(i) / MAX_N_DIV_CIRCLE);
		cos_o[i] = cos(angle);
		sin_o[i] = sin(angle);
	}
}

template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::~SpectrumAnalyser(){
	delete[] sin_o;
}


template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
bool SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::TurnOn(){
	if (pthread_create(&m_thread, nullptr, thread_proc, this))
		return false;
}

template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::TurnOff(){

}


template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::Put(double *pcm, int length){
	assert(pcm);
	assert(length <= FRAME_SAMPLE_N * FRAME_N);

	// It must be ensured that the lowest frequency can vibrate
	//   for the specified minimum period within a given number of samples.
	assert(length >= m_n_cycle_samples_o[0] * MIN_WIN_N_VIB);

	// Make sure length is an integer multiple of FRAME_SAMPLE_N
	assert(length % FRAME_SAMPLE_N == 0);

	pthread_mutex_lock(&m_mutex_put);

	if (m_input_pcm_length)
		pthread_cond_wait(&m_cond_put, &m_mutex_put);
	assert(m_input_pcm_length == 0);

	memcpy(m_input_pcm, pcm, length * sizeof(double));
	m_input_pcm_length = length;

	pthread_mutex_unlock(&m_mutex_put);

	if (length)
		pthread_cond_signal(&m_cond_put);
}

template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
int SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::Get(double *sepectrum[FREQ_N]){
	assert(sepectrum);

	pthread_mutex_lock(&m_mutex_get);

	if (m_output_sepectrum_length == 0)
		pthread_cond_wait(&m_cond_get, &m_mutex_get);
	assert(m_output_sepectrum_length > 0);

	int length = m_output_sepectrum_length;

	memcpy(sepectrum, length, length * sizeof(double) * FREQ_N);
	m_output_sepectrum_length = 0;

	pthread_mutex_unlock(&m_mutex_get);

	pthread_cond_signal(&m_cond_get);

	return length;
}


template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::Clear(){
	pthread_mutex_lock(m_mutex_put);
	pthread_mutex_lock(m_mutex_get);

	m_input_pcm_length = 0;
	ld[0].length = 0;
	ld[0].phase = 0;
	ld[1].length = 0;
	ld[1].phase = 0;

	m_output_sepectrum_length = 0;

	pthread_mutex_unlock(&m_mutex_put);
	pthread_mutex_unlock(&m_mutex_get);

	pthread_cond_signal(&m_cond_get);
}

template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
void *SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::thread_proc(void *pthis){
	SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N> &obj =
			*reinterpret_cast<SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N> *>(pthis);

	pthread_mutex_lock(&obj.m_mutex_put);
	pthread_mutex_lock(&obj.m_mutex_get);

	if (obj.m_input_pcm[obj.m_input_pcm_index].length == 0)
		pthread_cond_wait(&obj.m_cond_put, &obj.m_mutex_put);
	assert(obj.m_input_pcm[obj.m_input_pcm_index].length > 0);

	if (obj.m_output_sepectrum_length > 0)
		pthread_cond_wait(&obj.m_cond_get, &obj.m_mutex_get);
	assert(obj.m_output_sepectrum_length == 0);

	obj.Analyse();

	assert(obj.m_input_pcm[obj.m_input_pcm_index] = 0);
	assert(obj.m_output_sepectrum_length > 0);

	pthread_mutex_unlock(&obj.m_mutex_put);
	pthread_mutex_unlock(&obj.m_mutex_get);

	pthread_cond_signal(&obj.m_cond_put);
	pthread_cond_signal(&obj.m_cond_get);

}

template <int FREQ_N, int FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<FREQ_N, FRAME_SAMPLE_N, FRAME_N>::Analyse(){
	double *bm_sin[FRAME_N] = ld[ld_index].bm_sin;
	double *bm_cos[FRAME_N] = ld[ld_index].bm_cos;
	int length = m_input_pcm_length;
	int n_frame = length / FRAME_SAMPLE_N;
	int nc_frame = n_frame / 2;

	struct LD *ld_l = ld[1 - ld_index];

	double *bm_sin_l[FRAME_N] = ld_l->bm_sin;
	double *bm_cos_l[FRAME_N] = ld_l->bm_cos;
	int len_l = ld_l->length;
	int n_frame_l = len_l / FRAME_SAMPLE_N;
	int nc_frame_l = n_frame_l / 2;

	int output_length = nc_frame_l + nc_frame;

	for (int i_freq = 0; i_freq < FREQ_N; i_freq++){
		double phase_l = ld_l->phase;
		double m_sin[FRAME_SAMPLE_N * FRAME_N];
		double m_cos[FRAME_SAMPLE_N * FRAME_N];
		for (int i = 0; i < length; i++){
			m_sin[i] =
					m_input_pcm[i] *
					sin_o[static_cast<int>(
							(
								static_cast<double>(i) / m_n_cycle_samples_o[i_freq] + phase_l
								) * MAX_N_DIV_CIRCLE
						) % MAX_N_DIV_CIRCLE];
			m_cos[i] =
					m_input_pcm[i] *
					cos_o[static_cast<int>(
							(
								static_cast<double>(i) / m_n_cycle_samples_o[i_freq] + phase_l
								) * MAX_N_DIV_CIRCLE
						) % MAX_N_DIV_CIRCLE];
		}
		ld_l->length = n_frame;
		ld_l->phase = static_cast<double>(length) / m_n_cycle_samples_o[i_freq] + phase_l;

		// Fill all frames
		for (int i = 0; i < n_frame; i++){
			double bm_sin_ = 0.0;
			double bm_cos_ = 0.0;
			for (int j = 0; j < FRAME_SAMPLE_N; j++){
				bm_sin_ += m_sin[i*FRAME_SAMPLE_N+j];
				bm_cos_ += m_cos[i*FRAME_SAMPLE_N+j];
			}
			bm_sin[i_freq][i] = bm_sin_;
			bm_cos[i_freq][i] = bm_cos_;
		}

		// Determine the range of frames for vibration
		int min_n_b = MIN_WIN_N_VIB * m_n_cycle_samples_o[i_freq] / FRAME_SAMPLE_N;
		if (min_n_b < 1) min_n_b = 1;
		assert(min_n_b <= nc_frame);

		int max_n_b = MAX_WIN_N_VIB * m_n_cycle_samples_o[i_freq] / FRAME_SAMPLE_N;
		if (max_n_b > nc_frame) max_n_b = nc_frame;
		if (max_n_b < 1) max_n_b = 1;
		assert(min_n_b <= max_n_b);

		// Recursive summation
		double sum_bm_sin_b[FRAME_N*2][FRAME_N];
		double sum_bm_cos_b[FRAME_N*2][FRAME_N];
		double *sum_bm_sin[FRAME_N] = sum_bm_sin_b + FRAME_N;
		double *sum_bm_cos[FRAME_N] = sum_bm_cos_b + FRAME_N;
		for (int i = 0; i < n_frame; i++){
			sum_bm_sin[i][0] = bm_sin[i_freq][i];
			sum_bm_cos[i][0] = bm_cos[i_freq][i];
		}
		for (int i = -1; i >= -n_frame_l; i--){
			sum_bm_sin[i][0] = bm_sin_l[i_freq][n_frame_l - i];
			sum_bm_cos[i][0] = bm_cos_l[i_freq][n_frame_l - i];
		}
		for (int j = 1; j < max_n_b; j++){
			for (int i = -nc_frame_l; i < n_frame - j; i++){
				sum_bm_sin[i][j] = sum_bm_sin[i][j-1] + sum_bm_sin[i+j][0];
				sum_bm_cos[i][j] = sum_bm_cos[i][j-1] + sum_bm_cos[i+j][0];
			}
		}

		// Square sum
		double bm_q_sum_b[FRAME_N*2][FRAME_N];
		double *bm_q_sum[FRAME_N];
		for (int j = 0; j < max_n_b; j++){
			for (int i = -nc_frame_l; i < n_frame - j; i++){
				double sum_bm_sin_ = sum_bm_sin[i][j];
				double sum_bm_cos_ = sum_bm_cos[i][j];
				bm_q_sum[i][j] = sum_bm_sin_ * sum_bm_sin_ + sum_bm_cos_ * sum_bm_cos_;
			}
		}

		// TODO
		// Find the maximum value of the convolution result under small window movement
		double max_bm_q_sum[FRAME_N][FRAME_N];
		for (int j = min_n_b; j <= max_n_b; j++){
			for (int i = 0; i < output_length; i++){

			}
			double max_mq = 0.0;
			for (int i = -j; i <= 0; i++){
				if (bm_q_sum[i][j] > max_mq)
					max_mq = bm_q_sum[i][j];
			}
			max_bm_q_sum[j] = max_mq;
		}

		// Calculate the amplitude corresponding to the frequency
		double v_freq_a[FRAME_N];
		for (int j = min_n_b; j <= max_n_b; j++)
			v_freq_a[j] = sqrt(max_bm_q_sum[j]) / (j * FRAME_SAMPLE_N / 2);

		// With minimal results as the end result
		double v_freq = 0.0;
		for (int j = min_n_b; j <= max_n_b; j++)
			if (v_freq_a[j] > v_freq)
				v_freq = v_freq_a[j];

		//m_output_sepectrum
	}
	ld_index = 1 - ld_index;
}


