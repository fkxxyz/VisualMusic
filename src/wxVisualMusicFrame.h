#pragma once
#include "stdafx.h"
#include <wx/wx.h>
#include "AudioFileDecoder.h"
#include "WavePlayer/WavePlayer.h"

class wxVisualMusicFrame : public wxFrame {
public:
	wxVisualMusicFrame(const wxString& title,
					   const wxPoint& pos = wxDefaultPosition,
					   const wxSize& size = wxDefaultSize
											);
private:
	void OnOpen(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnPaint(wxPaintEvent& event);

private:
	wxMenu menuFile;
	wxMenu menuPlayer;
	wxMenu menuView;
	wxMenu menuHelp;
	wxMenuBar menuBar;

	wxPanel panel;
	wxButton buttonPlay;

	AudioFileDecoder m_file_decoder;

	wxDECLARE_EVENT_TABLE();
};
