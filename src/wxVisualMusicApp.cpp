
#include "stdafx.h"
#include "wxVisualMusicApp.h"
#include "wxVisualMusicFrame.h"

//wxIMPLEMENT_APP(wxVisualMusicApp);

bool wxVisualMusicApp::OnInit(){
	wxVisualMusicFrame *frame = new wxVisualMusicFrame("VisualMusic");
	frame->Show();
	return true;
}

int main(){
	AudioFileDecoder fd;
	fd.Run("/home/qaz/Music/CloudMusic/Diana Boncheva - Beethoven Virus.mp3");

	int data[OUTPUT_PCM_BUFFER_LEN / sizeof(int)];
	size_t len;
	WavePlayer player;

	do {
		len = fd.GetPCMPipe()->Read(reinterpret_cast<unsigned char *>(data), 1, OUTPUT_PCM_BUFFER_LEN / sizeof(int)) / sizeof(int);
		cout<<"pipe output "<<len<<" samples"<<endl;
	} while (len);
	return 0;
}

