#pragma once
#include "stdafx.h"


#ifdef USE_WX

class wxVisualMusicFrame : public wxFrame {
public:
	wxVisualMusicFrame(const wxString &title,
					   const wxPoint &pos = wxDefaultPosition,
					   const wxSize &size = wxDefaultSize
											);
	~wxVisualMusicFrame();

protected:
	void OnOpen(wxCommandEvent &event);
	void OnExit(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnTimer(wxTimerEvent& event);
	void OnTimerFPS(wxTimerEvent& event);

	int m_fps;

	wxMenu menuFile;
	wxMenu menuPlayer;
	wxMenu menuView;
	wxMenu menuHelp;
	wxMenuBar menuBar;

	wxPanel panel;
	wxButton buttonPlay;

	VisualMusicController m_controller;

	color_t *m_ColorArray;

	wxTimer m_timer;
	wxTimer m_timer_fps;

	wxDECLARE_EVENT_TABLE();
};

#endif
