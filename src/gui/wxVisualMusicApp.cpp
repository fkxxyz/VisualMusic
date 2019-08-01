
#include "stdafx.h"
#include "wxVisualMusicApp.h"
#include "wxVisualMusicFrame.h"


#ifdef USE_WX

wxIMPLEMENT_APP(wxVisualMusicApp);

bool wxVisualMusicApp::OnInit(){
	wxVisualMusicFrame *frame = new wxVisualMusicFrame("VisualMusic");
	frame->Show();
	return true;
}

#endif



#include <iostream>
using namespace std;
#include "WavePlayer/WavePlayer.h"
#include <cmath>
#include <sys/time.h>

#define RATE 44100
#define freq 4000
#define t 1

/*
int main(){
	VisualMusicController control;
	control.OpenFile("/home/qaz/Music/CloudMusic/Diana Boncheva - Beethoven Virus.mp3");
	control.Play();
	sleep(100000);
	return 0;
}

int main(){
	float voice[RATE * t];
	for (int i = 0; i < RATE * t; i++)
		voice[i] = (float)(sin(i * (2 * M_PI * freq / RATE)));
		//voice[i] = (float)(0.75 * sin(i * (2 * M_PI * freq / RATE)) + 0.25 * sin(7 * i * (2 * M_PI * freq / RATE)));


	WavePlayer player;
	player.Open(1, RATE, WavePlayer::fm_float);
	player.Play(voice, RATE * t);

	player.Join();

	player.Close();
	return 0;
}


int main(){
	AudioFileDecoder fd;
	bool r = fd.Run("/home/qaz/Music/CloudMusic/Diana Boncheva - Beethoven Virus.mp3");
	assert(r);

	const size_t max_len = OUTPUT_PCM_BUFFER_LEN / sizeof(int);
	int data[max_len];
	size_t len;
	WavePlayer player;
	r = player.Open(fd.GetChanners(), fd.GetSampleRate(), WavePlayer::fm_short);
	assert(r);

	do {
		len = fd.GetPCMPipe()->Read(reinterpret_cast<unsigned char *>(data), 8, OUTPUT_PCM_BUFFER_LEN) / sizeof(int);

		assert(len <= max_len);

		short f_data[max_len];
		PCMScaler::Scale(f_data, data, len);
		r = player.Play(f_data, len/fd.GetChanners());
		assert(r);
	} while (len);
	player.Join();
	return 0;
}
*/
