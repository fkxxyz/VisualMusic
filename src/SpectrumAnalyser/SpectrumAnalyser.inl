
#include "stdafx.h"
#include "SpectrumAnalyser.h"
#include <cassert>
#include <cstring>
#include <cmath>


template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::SpectrumAnalyser()
{
	assert(MIN_WIN_N_VIB <= MAX_WIN_N_VIB);
	assert(MIN_WIN_N_VIB > 0);

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

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::~SpectrumAnalyser(){
	delete[] sin_o;
}


template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
bool SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::TurnOn(){
	if (pthread_create(&m_thread, nullptr, thread_proc, this))
		return false;
}

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::TurnOff(){

}

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::SetArguments(
		const double *freqs,
		const int freq_n,
		const double sample_rate,
		const int frame_sample_n
		)
{

	pthread_mutex_lock(&m_mutex_put);
	pthread_mutex_lock(&m_mutex_get);

	m_const_frame_sample_n = frame_sample_n;
	assert(m_const_frame_sample_n <= MAX_FRAME_SAMPLE_N);

	m_const_freq_n = freq_n;

	pthread_mutex_unlock(&m_mutex_put);
	pthread_mutex_unlock(&m_mutex_get);

	Clear();

	// Make sure the frequency array is strictly increasing.
	for (int i = 0; i < m_const_freq_n - 1; i++)
		assert(freqs[i] < freqs[i+1]); // Please sort it.

	// Calculate the number of samples included in each cycle at each frequency
	for (int i = 0; i < m_const_freq_n; i++)
		m_n_cycle_samples_o[i] = sample_rate / freqs[i];

	// It must be ensured that the lowest frequency can vibrate
	//   for the specified minimum period within a given number of samples.
	assert(m_const_frame_sample_n * FRAME_N >= m_n_cycle_samples_o[0] * MIN_WIN_N_VIB * 2);

	m_const_reserve_frame_n = static_cast<int>(m_n_cycle_samples_o[0] / m_const_frame_sample_n * MIN_WIN_N_VIB);
}

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
int SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::GetCountOfReserveFrame() const {
	return m_const_reserve_frame_n;
}

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::Put(double *pcm, int length){
	assert(pcm);
	assert(length <= m_const_frame_sample_n * FRAME_N);

	// It must be ensured that the lowest frequency can vibrate
	//   for the specified minimum period within a given number of samples.
	assert(length >= static_cast<int>(m_n_cycle_samples_o[0] * MIN_WIN_N_VIB * 2));

	// Make sure length is an integer multiple of FRAME_SAMPLE_N
	assert(length % m_const_frame_sample_n == 0);

	pthread_mutex_lock(&m_mutex_put);

	while (m_input_pcm_length)
		pthread_cond_wait(&m_cond_put, &m_mutex_put);

	m_end_flag = false;

	memcpy(m_input_pcm, pcm, length * sizeof(double));
	m_input_pcm_length = length;

	pthread_mutex_unlock(&m_mutex_put);

	if (length)
		pthread_cond_signal(&m_cond_put);
}

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
int SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::Get(double (*sepectrum)[MAX_FREQ_N]){
	assert(sepectrum);

	pthread_mutex_lock(&m_mutex_get);

	while (m_output_sepectrum_length == 0){
		if (m_end_flag){
			pthread_mutex_unlock(&m_mutex_get);
			return 0;
		}
		pthread_cond_wait(&m_cond_get, &m_mutex_get);
	}

	int length = m_output_sepectrum_length;

	memcpy(sepectrum, m_output_sepectrum, length * sizeof(double) * MAX_FREQ_N);
	m_output_sepectrum_length = 0;

	pthread_mutex_unlock(&m_mutex_get);

	pthread_cond_signal(&m_cond_get);

	return length;
}


template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::Clear(){
	pthread_mutex_lock(&m_mutex_put);
	pthread_mutex_lock(&m_mutex_get);

	ld_index = 0;

	m_input_pcm_length = 0;

	ld[0].length = 0;
	memset(ld[0].phase, 0, m_const_freq_n * sizeof(double));
	ld[1].length = 0;
	memset(ld[1].phase, 0, m_const_freq_n * sizeof(double));

	m_output_sepectrum_length = 0;

	pthread_mutex_unlock(&m_mutex_put);
	pthread_mutex_unlock(&m_mutex_get);

	pthread_cond_signal(&m_cond_get);
}

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::NotifyEnd(){
	m_end_flag = true;
	pthread_cond_signal(&m_cond_get);
}

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
void *SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::thread_proc(void *pthis){
	SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N> &obj =
			*reinterpret_cast<SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N> *>(pthis);

	while (1){
		pthread_mutex_lock(&obj.m_mutex_put);
		pthread_mutex_lock(&obj.m_mutex_get);

		if (obj.m_output_sepectrum_length > 0)
			pthread_cond_wait(&obj.m_cond_get, &obj.m_mutex_get);
		assert(obj.m_output_sepectrum_length == 0);

		if (obj.m_input_pcm_length == 0)
			pthread_cond_wait(&obj.m_cond_put, &obj.m_mutex_put);
		assert(obj.m_input_pcm_length > 0);

		obj.Analyse();

		assert(obj.m_input_pcm_length == 0);
		assert(obj.m_output_sepectrum_length > 0);

		pthread_mutex_unlock(&obj.m_mutex_get);
		pthread_mutex_unlock(&obj.m_mutex_put);

		pthread_cond_signal(&obj.m_cond_get);
		pthread_cond_signal(&obj.m_cond_put);
	}
}

template <int MAX_FREQ_N, int MAX_FRAME_SAMPLE_N, int FRAME_N>
void SpectrumAnalyser<MAX_FREQ_N, MAX_FRAME_SAMPLE_N, FRAME_N>::Analyse(){
	double (*bm_sin)[FRAME_N] = ld[ld_index].bm_sin;
	double (*bm_cos)[FRAME_N] = ld[ld_index].bm_cos;
	int length = m_input_pcm_length;
	int n_frame = length / m_const_frame_sample_n;
	int nc_frame = n_frame - m_const_reserve_frame_n;
	ld[ld_index].length = n_frame;

	struct LD *ld_l = &ld[1 - ld_index];

	double (*bm_sin_l)[FRAME_N] = ld_l->bm_sin;
	double (*bm_cos_l)[FRAME_N] = ld_l->bm_cos;
	int n_frame_l = ld_l->length;
	assert(n_frame_l >= 0);

	int output_length, output_start_frame;
	if (n_frame_l < m_const_reserve_frame_n)
		output_start_frame = m_const_reserve_frame_n - n_frame_l;
	else
		output_start_frame = -m_const_reserve_frame_n;
	output_length = nc_frame - output_start_frame;
	assert(output_length <= FRAME_N);

	for (int i_freq = 0; i_freq < m_const_freq_n; i_freq++){
		double phase_l = ld_l->phase[i_freq];
		double m_sin[m_const_frame_sample_n * FRAME_N];
		double m_cos[m_const_frame_sample_n * FRAME_N];
		for (int i = 0; i < length; i++){
			int phase_index = static_cast<int>(
								  (
									  static_cast<double>(i) / m_n_cycle_samples_o[i_freq] + phase_l
									  ) * MAX_N_DIV_CIRCLE
							  ) % MAX_N_DIV_CIRCLE;
			m_sin[i] = m_input_pcm[i] * sin_o[phase_index];
			m_cos[i] = m_input_pcm[i] * cos_o[phase_index];
		}
		double new_phase = static_cast<double>(length) / m_n_cycle_samples_o[i_freq] + phase_l;
		new_phase = new_phase - static_cast<int>(new_phase);
		ld_l->phase[i_freq] = new_phase;

		// Fill all frames
		for (int i = 0; i < n_frame; i++){
			double bm_sin_ = 0.0;
			double bm_cos_ = 0.0;
			for (int j = 0; j < m_const_frame_sample_n; j++){
				bm_sin_ += m_sin[i*m_const_frame_sample_n+j];
				bm_cos_ += m_cos[i*m_const_frame_sample_n+j];
			}
			bm_sin[i_freq][i] = bm_sin_;
			bm_cos[i_freq][i] = bm_cos_;
		}


		if (i_freq == DEBUG_I_FREQ && DEBUG_BLOCK == 50){
			for (int i = -1; i >= -n_frame_l; i--){
				cout<<ld[1-ld_index].bm_sin[i_freq][i]<<' ';
			}
			cout<<endl;
			for (int i = 0; i < n_frame; i++){
				cout<<ld[ld_index].bm_sin[i_freq][i]<<' ';
			}
			cout<<endl;
			cout<<endl;
		}



		if (i_freq == DEBUG_I_FREQ && DEBUG_BLOCK == 5){
			for (int i = -1; i >= -n_frame_l; i--){
				cout<<bm_sin_l[i_freq][i]<<' ';
			}
			cout<<endl;
			for (int i = 0; i < n_frame; i++){
				cout<<bm_sin[i_freq][i]<<' ';
			}
			cout<<endl;
			cout<<endl;
		}


		// Determine the range of frames for vibration
		int min_n_b = MIN_WIN_N_VIB * m_n_cycle_samples_o[i_freq] / m_const_frame_sample_n;
		if (min_n_b < 1) min_n_b = 1;
		assert(min_n_b <= nc_frame);

		int max_n_b = MAX_WIN_N_VIB * m_n_cycle_samples_o[i_freq] / m_const_frame_sample_n;
		if (max_n_b > nc_frame) max_n_b = nc_frame;
		if (max_n_b < 1) max_n_b = 1;
		assert(min_n_b <= max_n_b);

		// Recursive summation
		double sum_bm_sin_b[FRAME_N*2][FRAME_N];
		double sum_bm_cos_b[FRAME_N*2][FRAME_N];
		double (*sum_bm_sin)[FRAME_N] = sum_bm_sin_b + FRAME_N;
		double (*sum_bm_cos)[FRAME_N] = sum_bm_cos_b + FRAME_N;
		for (int i = 0; i < n_frame; i++){
			sum_bm_sin[i][0] = bm_sin[i_freq][i];
			sum_bm_cos[i][0] = bm_cos[i_freq][i];
		}
		for (int i = -1; i >= -n_frame_l; i--){
			sum_bm_sin[i][0] = bm_sin_l[i_freq][n_frame_l + i];
			sum_bm_cos[i][0] = bm_cos_l[i_freq][n_frame_l + i];
		}
		for (int j = 1; j < max_n_b; j++){
			for (int i = -n_frame_l; i < nc_frame; i++){
				sum_bm_sin[i][j] = sum_bm_sin[i][j-1] + sum_bm_sin[i+j][0];
				sum_bm_cos[i][j] = sum_bm_cos[i][j-1] + sum_bm_cos[i+j][0];
			}
		}


		if (i_freq == DEBUG_I_FREQ && DEBUG_BLOCK == 4){
			for (int i = -n_frame_l; i < nc_frame; i++){
				cout<<i<<'\t';
				for (int j = 0; j < max_n_b; j++)
					cout<<sum_bm_sin[i][j]<<' ';
				cout<<endl;
				cout<<i<<'\t';
				for (int j = 0; j < max_n_b; j++)
					cout<<sum_bm_cos[i][j]<<' ';
				cout<<endl;
			}
			cout<<endl;
		}


		// Square sum
		double bm_q_sum_b[FRAME_N*2][FRAME_N];
		double (*bm_q_sum)[FRAME_N] = bm_q_sum_b + FRAME_N;
		for (int j = 0; j < max_n_b; j++){
			for (int i = -n_frame_l; i < nc_frame; i++){
				double sum_bm_sin_ = sum_bm_sin[i][j];
				double sum_bm_cos_ = sum_bm_cos[i][j];
				bm_q_sum[i][j] = sum_bm_sin_ * sum_bm_sin_ + sum_bm_cos_ * sum_bm_cos_;
			}
		}


		if (i_freq == DEBUG_I_FREQ && DEBUG_BLOCK == 3){
			for (int i = -n_frame_l; i < nc_frame; i++){
				cout<<i<<'\t';
				for (int j = 0; j < max_n_b; j++)
					cout<<bm_q_sum[i][j]<<' ';
				cout<<endl;
			}
			cout<<endl;
		}


		// Find the maximum value of the convolution result under small window movement
		double max_bm_q_sum[FRAME_N][FRAME_N];
		for (int j = min_n_b - 1; j < max_n_b; j++){
			for (int i = output_start_frame; i < nc_frame; i++){
				double max_mq = 0.0;
				for (int k = i - j; k <= i; k++){
					if (bm_q_sum[k][j] > max_mq)
						max_mq = bm_q_sum[k][j];
				}
				max_bm_q_sum[i-output_start_frame][j] = max_mq;
			}
		}


		if (i_freq == DEBUG_I_FREQ && DEBUG_BLOCK == 2){
			for (int i = output_start_frame; i < nc_frame; i++){
				cout<<i<<'\t';
				for (int j = min_n_b - 1; j < max_n_b; j++)
					cout<<max_bm_q_sum[i-output_start_frame][j]<<' ';
				cout<<endl;
			}
			cout<<endl;
		}


		// Calculate the amplitude corresponding to the frequency
		double v_freq_a[FRAME_N][FRAME_N];
		for (int j = min_n_b - 1; j < max_n_b; j++)
			for (int i = 0; i < output_length; i++)
				v_freq_a[i][j] = sqrt(max_bm_q_sum[i][j]) * 2 / ((j + 1) * m_const_frame_sample_n);


		if (i_freq == DEBUG_I_FREQ && DEBUG_BLOCK == 1){
			for (int i = 0; i < output_length; i++){
				cout<<i<<'\t';
				for (int j = min_n_b - 1; j < max_n_b; j++)
					cout<<v_freq_a[i][j]<<' ';
				cout<<endl;
			}
			cout<<endl;
		}


		// With minimal results as the end result
		for (int i = 0; i < output_length; i++){
			double v_freq = 0.0;
			for (int j = min_n_b - 1; j < max_n_b; j++)
				if (v_freq_a[i][j] > v_freq)
					v_freq = v_freq_a[i][j];
			m_output_sepectrum[i][i_freq] = v_freq;
		}


		if (i_freq == DEBUG_I_FREQ && DEBUG_BLOCK == 0){
			for (int i = 0; i < output_length; i++)
				cout<<m_output_sepectrum[i][i_freq]<<endl;
		}


	}
	m_output_sepectrum_length = output_length;
	m_input_pcm_length = 0;
	ld_index = 1 - ld_index;
}


