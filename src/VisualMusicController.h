#pragma once
#include "stdafx.h"
#include "AudioDecoder/AudioFileDecoder.h"
#include "WavePlayer/WavePlayer.h"
#include "SpectrumAnalyser/SpectrumAnalyser.h"
#include "DrawTemplate/DrawTemplate.h"
#include <pthread.h>
#include <samplerate.h>

#include "constants.h"

class VisualMusicController {
public:
	VisualMusicController();

	bool OpenFile(const char *audio_file_path);
	void CloseFile();

	bool Play();
	bool Pause();
	bool Resume();
	bool PlayPause();
	bool Stop();

	bool SetPos(int milliseconds);

	static void draw(void *pthis, DrawTemplate *pdraw);

protected:
	WavePlayer m_player;
	AudioFileDecoder m_decoder;
	SpectrumAnalyser<N_FREQ, MAX_FRAME_SAMPLE, MIN_FRAME_N_BUFFER> m_analyser;

	double m_const_freqs[MAX_N_FREQ];
	int m_const_freq_n;

	char tone_name_buffer[1024];
	char *tone_name[MAX_N_FREQ];
	void generate_all_freq();

	void start_all_thread();

	bool open_wave_player();

	pthread_t m_thread_forking;
	static void *thread_forking_proc(void *pthis);
	void *forking();

	pthread_t m_thread_wait_to_play;
	static void *thread_wait_to_play_proc(void *pthis);
	void *wait_to_play();

	pthread_t m_thread_put_spectrum;
	static void *thread_put_spectrum_proc(void *pthis);
	void *put_spectrum();

	BasePipe<unsigned char, MAX_SAMPLE_RATE * sizeof(float) * 2 * 2 * N_TIME / FRAME_RATE> m_pipe_play_pcm;
	BasePipe<double [MAX_N_FREQ], N_TIME> m_pipe_spectrum;

	int spectrum_time_stamp;

	pthread_mutex_t m_mutex_loop;

	sem_t m_sem_start;
	bool m_stop_flag;
};


#include <cassert>
#include <cmath>
#include "AudioDecoder/PCMScaler.h"

inline VisualMusicController::VisualMusicController()
{
	sem_init(&m_sem_start, 0, 0);
	pthread_mutex_init(&m_mutex_loop, nullptr);

	generate_all_freq();

	start_all_thread();
}

inline void VisualMusicController::start_all_thread(){

	if (pthread_create(&m_thread_forking, nullptr, thread_forking_proc, this)){
		assert(0);
		return;
	}

	if (pthread_create(&m_thread_put_spectrum, nullptr, thread_put_spectrum_proc, this)){
		assert(0);
		return;
	}

	if (pthread_create(&m_thread_wait_to_play, nullptr, thread_wait_to_play_proc, this)){
		assert(0);
		return;
	}
}

inline void VisualMusicController::generate_all_freq(){
	const char *octs[12] = {"C", "#C", "D", "#D", "E", "F", "#F", "G", "#G", "A", "#A", "B"};

	// 440 HZ tone is A4
	const double std_freq = 440.0;
	const int std_oct = 9; // A
	const int std_range = 4;

	// Frequency range is 20HZ ~ 20000HZ
	const double min_freq = MIN_FREQ_RANGE;
	const double max_freq = MAX_FREQ_RANGE;

	const double scale = pow(2.0, 1.0/12);

	double f;
	int ii = std_range * 12 + std_oct;
	for (f = std_freq; f > min_freq; f /= scale)
		ii--;

	int i = 0;
	char *pc = tone_name_buffer;
	for (f *= scale; f < max_freq; f*= scale){
		m_const_freqs[i] = f;

		assert(ii >= 0);
		int range = ii / 12;
		int oct = ii % 12;
		tone_name[i] = pc;
		pc += sprintf(pc, "%s%d", octs[oct], range) + 1;

		i++;
		ii++;
	}
	m_const_freq_n = i;
}

inline bool VisualMusicController::OpenFile(const char *audio_file_path){
	if (!m_decoder.Run(audio_file_path)){
		assert(0);
		return false;
	}
	if (!open_wave_player()){
		assert(0);
		return false;
	}
	return true;
}

inline void VisualMusicController::CloseFile(){
	if (!Stop()){
		assert(0);
		return;
	}
	m_decoder.Stop();
}

inline bool VisualMusicController::Play(){
	m_stop_flag = false;
	if (sem_post(&m_sem_start)){
		assert(0);
		return false;
	}
	return true;
}

inline bool VisualMusicController::Pause(){
	if (!m_player.Pause()){
		assert(0);
		return false;
	}
	return true;
}

inline bool VisualMusicController::Resume(){
	if (!m_player.Resume()){
		assert(0);
		return false;
	}
	return true;
}

inline bool VisualMusicController::PlayPause(){
	enum WavePlayer::status status = m_player.GetStatus();
	switch (status){
	case WavePlayer::st_pause:
		if (!m_player.Resume()){
			assert(0);
			return false;
		}
		break;
	case WavePlayer::st_playing:
		if (!m_player.Pause()){
			assert(0);
			return false;
		}
		break;
	case WavePlayer::st_opened:
		if (!Play()){
			assert(0);
			return false;
		}
		break;
	default:
		assert(0);
	}
	return true;
}

inline bool VisualMusicController::Stop(){
	while (!sem_trywait(&m_sem_start))
		;
	m_stop_flag = true;
	return true;
}

inline bool VisualMusicController::SetPos(int milliseconds){
	return m_decoder.SetPos(milliseconds);
}

inline void *VisualMusicController::thread_forking_proc(void *pthis){
	return reinterpret_cast<VisualMusicController *>(pthis)->forking();
}
inline void *VisualMusicController::forking(){
	while (1){

		sem_wait(&m_sem_start);
		sem_post(&m_sem_start);

		unsigned int sample_rate = m_decoder.GetSampleRate();
		assert(sample_rate <= MAX_SAMPLE_RATE);
		unsigned int channers = m_decoder.GetChanners();
		assert(channers <= 2);
		size_t sample_size = m_decoder.GetSizeofSample();
		BaseAudioDecoder::sample_type type = m_decoder.GetTypeofSample();
		m_analyser.SetArguments(
					m_const_freqs,
					m_const_freq_n,
					sample_rate,
					sample_rate / FRAME_RATE);
		const size_t MAX_READ_N_SAMPLE = (static_cast<size_t>(
											  FRAME_RATE / MIN_FREQ * MIN_WIN_N_VIB * 2
											  ) + 1) * static_cast<size_t>(
											 MAX_SAMPLE_RATE / FRAME_RATE
											 );
		size_t length = (static_cast<size_t>(
							 FRAME_RATE / MIN_FREQ * MIN_WIN_N_VIB * 2
							 ) + 1) * static_cast<size_t>(
							sample_rate / FRAME_RATE
							);

		spectrum_time_stamp = m_analyser.GetCountOfReserveFrame();

		while (1){
			// Read pcm from pcm pipe
			unsigned char pcm_data_buffer[MAX_READ_N_SAMPLE * sizeof(double) * 2];
			size_t read_count = m_decoder.GetPCMPipe()->Read(
									pcm_data_buffer,
									length * channers * sample_size,
									length * channers * sample_size
									);
			if (m_stop_flag){
				m_player.Pause();
				m_analyser.Clear();
				cout<<"m_analyser clear."<<endl;
				m_pipe_spectrum.Clear();
				cout<<"m_pipe_spectrum clear."<<endl;
				m_pipe_play_pcm.Clear();
				cout<<"m_pipe_play_pcm clear."<<endl;
				m_player.Stop();
				m_decoder.SetPos(0);
				break;
			}
			if (read_count < length * channers * sample_size){
				m_pipe_play_pcm.NotifyEnd();
				break;
			}

			// Convert to double
			double pcm_data_double[MAX_READ_N_SAMPLE * 2];
			PCMScaler::Scale(pcm_data_double, pcm_data_buffer, length * channers, type);
			if (channers == 2)
				for (size_t i = 0; i < length; i++)
					pcm_data_double[i] = (pcm_data_double[i*2] + pcm_data_double[i*2+1]) / 2;

			//cout<<"read "<<length<<" samples."<<endl;

			// Put into the analyser.
			m_analyser.Put(pcm_data_double, static_cast<int>(length));

			// Convert to the appropriate format.
			float pcm_data_float[MAX_READ_N_SAMPLE * 2];
			switch (type){
			case BaseAudioDecoder::st_uchar:
			case BaseAudioDecoder::st_short:
			case BaseAudioDecoder::st_float:
				m_pipe_play_pcm.Write(
							pcm_data_buffer,
							length * channers * sample_size
							);
				break;
			case BaseAudioDecoder::st_int:
				PCMScaler::Scale(pcm_data_float, pcm_data_buffer, length * channers, type);
				m_pipe_play_pcm.Write(
							reinterpret_cast<unsigned char *>(pcm_data_float),
							length * channers * sizeof(float)
							);
				break;
			default:
				assert(0);
			}
		}

		Stop();
	}
}

inline void *VisualMusicController::thread_wait_to_play_proc(void *pthis){
	return reinterpret_cast<VisualMusicController *>(pthis)->wait_to_play();
}
inline void *VisualMusicController::wait_to_play(){
	while (1){

		// Wait until the pipe is full
		while (m_pipe_spectrum.GetLength() < N_TIME)
			Sleep(15);

		BaseAudioDecoder::sample_type type = m_decoder.GetTypeofSample();
		unsigned int channers = m_decoder.GetChanners();

		const size_t buffer_len = MAX_SAMPLE_RATE * sizeof(float) * 2 * N_TIME / FRAME_RATE;
		unsigned char pcm_data_buffer[buffer_len];
		while (1){
			size_t read_len = m_pipe_play_pcm.Read(pcm_data_buffer, 8, buffer_len);
			if (read_len == 0)
				break;
			bool result;
			switch (type){
			case BaseAudioDecoder::st_uchar:
				assert(read_len % channers == 0);
				result = m_player.Write(
							 pcm_data_buffer,
							 read_len / channers
							 );
				assert(result);
				break;
			case BaseAudioDecoder::st_short:
				assert(read_len % (channers * sizeof(short)) == 0);
				result = m_player.Write(
							 reinterpret_cast<short *>(pcm_data_buffer),
							 read_len / channers / sizeof(short)
							 );
				assert(result);
				break;
			case BaseAudioDecoder::st_float:
			case BaseAudioDecoder::st_int:
				assert(read_len % (channers * sizeof(float)) == 0);
				result = m_player.Write(
							 reinterpret_cast<float *>(pcm_data_buffer),
							 read_len / channers / sizeof(float)
							 );
				assert(result);
				break;
			default:
				assert(0);
			}
		}
	}
}

inline void *VisualMusicController::thread_put_spectrum_proc(void *pthis){
	return reinterpret_cast<VisualMusicController *>(pthis)->put_spectrum();
}
inline void *VisualMusicController::put_spectrum(){
	double spectrum[MIN_FRAME_N_BUFFER][N_FREQ];
	while (1){

		sem_wait(&m_sem_start);
		sem_post(&m_sem_start);

		while (1){
			// Get analysis results
			int frames = m_analyser.Get(spectrum);
			if (!frames)
				break;

			cout<<"m_analyser get "<<frames<<" frames."<<endl;

			// Put into the pipe for drawing.
			m_pipe_spectrum.Write(spectrum, static_cast<size_t>(frames));
		}
	}
}

inline bool VisualMusicController::open_wave_player(){
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
	unsigned int sample_rate = m_decoder.GetSampleRate();
	assert(sample_rate <= MAX_SAMPLE_RATE);
	unsigned int channers = m_decoder.GetChanners();
	assert(channers <= 2);
	return m_player.Open(channers, sample_rate, format);
}


inline void VisualMusicController::draw(void *pthis, DrawTemplate *pdraw){
	VisualMusicController &obj = *reinterpret_cast<VisualMusicController *>(pthis);
	pdraw->Rectangle(
				color(0, 0, 0),
				0,
				0,
				VIEW_FREQ_PIXEL_WIDTH * N_FREQ,
				VIEW_TIME_PIXEL_HEIGHT * N_TIME
				);
	if (obj.m_player.GetStatus() == WavePlayer::st_playing){
		int current_time_stamp = static_cast<int>(
									 static_cast<double>(
										 obj.m_player.GetPos()
										 ) / obj.m_decoder.GetSampleRate() * FRAME_RATE
									 );
		cout<<current_time_stamp<<endl;

		if (current_time_stamp <= obj.spectrum_time_stamp)
			return;

		size_t skip_len = static_cast<size_t>(current_time_stamp - obj.spectrum_time_stamp - 1);
		obj.m_pipe_spectrum.Read(nullptr, skip_len, skip_len);

		double spectrum_data[MAX_N_FREQ];
		obj.m_pipe_spectrum.Read(&spectrum_data, 1, 1);
		obj.spectrum_time_stamp += current_time_stamp - obj.spectrum_time_stamp;

		for (int f = 0; f < N_FREQ; f++){
			double x = spectrum_data[f];
			double y = x;//scale_func(x, sc_k[f]);
			int v = static_cast<int>(y * N_TIME * VIEW_TIME_PIXEL_HEIGHT);
			pdraw->Rectangle(
						color(0, 127, 255),
						f * VIEW_FREQ_PIXEL_WIDTH,
						N_TIME * VIEW_TIME_PIXEL_HEIGHT - v,
						(f + 1) * VIEW_FREQ_PIXEL_WIDTH,
						N_TIME * VIEW_TIME_PIXEL_HEIGHT
						);
		}
	}
}





