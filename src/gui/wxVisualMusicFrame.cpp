
#include "stdafx.h"
#include "wxVisualMusicFrame.h"
#include "constants.h"

#include <wx/file.h>
#include <wx/dcbuffer.h>

enum wxID
{
	wxID_BASE = 1,

	wxID_PLAYER_PLAY_PAUSE,
	wxID_PLAYER_STOP,
	wxID_PLAYER_SETPOSITION,

	wxID_VIEW_BAR,
	wxID_VIEW_2D,

	wxID_TIMER,
	wxID_TIMER_FPS
};

wxBEGIN_EVENT_TABLE(wxVisualMusicFrame, wxFrame)
	EVT_MENU(wxID_OPEN,  wxVisualMusicFrame::OnOpen)
	EVT_MENU(wxID_EXIT,  wxVisualMusicFrame::OnExit)
	EVT_MENU(wxID_ABOUT, wxVisualMusicFrame::OnAbout)
	EVT_PAINT(wxVisualMusicFrame::OnPaint)
	EVT_TIMER(wxID_TIMER, wxVisualMusicFrame::OnTimer)
	EVT_TIMER(wxID_TIMER_FPS, wxVisualMusicFrame::OnTimerFPS)
wxEND_EVENT_TABLE()

wxVisualMusicFrame::wxVisualMusicFrame(
		const wxString& title,
		const wxPoint& pos,
		const wxSize& size
		):
	wxFrame(nullptr, wxID_ANY, title, pos, size),
	m_fps(0),
	m_timer(this, wxID_TIMER),
	m_timer_fps(this, wxID_TIMER_FPS)
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(wxID_OPEN);
	menuFile->Append(wxID_CLOSE);
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu *menuPlayer = new wxMenu;
	menuPlayer->Append(wxID_PLAYER_PLAY_PAUSE, "&Play\tSpace");
	menuPlayer->Append(wxID_PLAYER_STOP, "&Stop\tEsc");
	menuPlayer->Append(wxID_PLAYER_SETPOSITION, "&Set Position");

	wxMenu *menuView = new wxMenu;
	menuView->Append(wxID_VIEW_BAR, "&Bar\tCtrl-1");
	menuView->Append(wxID_VIEW_2D, "&2D\tCtrl-2");

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuPlayer, "&Player");
	menuBar->Append(menuView, "&View");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();

	SetClientSize(
				VIEW_FREQ_PIXEL_WIDTH * N_FREQ,
				VIEW_TIME_PIXEL_HEIGHT * N_TIME //+ VIEW_CONTROL_SPACING + VIEW_BUTTON_HEIGHT + VIEW_CONTROL_SPACING
			);
/*
	wxPanel* panel = new wxPanel(this);
	wxButton *buttonPlay = new wxButton(
							   panel,
							   wxID_PLAYER_PLAY_PAUSE,
							   L"॥",
							   wxPoint(
								   (VIEW_FREQ_PIXEL_WIDTH * N_FREQ - VIEW_BUTTON_HEIGHT) / 2,
								   VIEW_TIME_PIXEL_HEIGHT * N_TIME + VIEW_CONTROL_SPACING
								   ),
							   wxSize(VIEW_BUTTON_HEIGHT, VIEW_BUTTON_HEIGHT)
							   );

*/

	m_timer.Start(16);
	m_timer_fps.Start(1000);

	m_ColorArray = new color_t[(VIEW_FREQ_PIXEL_WIDTH * N_FREQ) * (VIEW_TIME_PIXEL_HEIGHT * N_TIME)];
}

wxVisualMusicFrame::~wxVisualMusicFrame(){
	delete[] m_ColorArray;
}

void wxVisualMusicFrame::OnOpen(wxCommandEvent& event){
	wxFileDialog dialog(
				this,
				_T("Select a supported audio file. (mp3, wav)"),
				wxEmptyString,
				wxEmptyString,
				"mp3 or wav files (*.mp3;*.wav)|*.mp3;*.wav",
				wxFD_OPEN | wxFD_FILE_MUST_EXIST
				);
	if (dialog.ShowModal() == wxID_CANCEL)
		return;

	m_controller.StartAllThread(static_cast<const char *>(dialog.GetPath()));
	m_controller.OpenWavePlayer();
}

void wxVisualMusicFrame::OnExit(wxCommandEvent& event){
	Close(true);

}
void wxVisualMusicFrame::OnAbout(wxCommandEvent& event){
	wxMessageBox( "This is a wxWidgets' Hello world sample",
					  "About Hello World", wxOK | wxICON_INFORMATION );
}


void wxVisualMusicFrame::OnPaint(wxPaintEvent& event){
	DrawTemplate draw(
				m_ColorArray,
				VIEW_FREQ_PIXEL_WIDTH * N_FREQ,
				VIEW_TIME_PIXEL_HEIGHT * N_TIME,
				VisualMusicController::draw,
				&m_controller
				);

	wxBufferedPaintDC dc(this);
	wxImage image(
				VIEW_FREQ_PIXEL_WIDTH * N_FREQ,
				VIEW_TIME_PIXEL_HEIGHT * N_TIME,
				reinterpret_cast<unsigned char *>(m_ColorArray),
				true
				);
	wxBitmap bitmap(image);
	dc.DrawBitmap(bitmap, 0, 0);

	m_fps++;
}

void wxVisualMusicFrame::OnTimer(wxTimerEvent& event){
	Refresh();
}

void wxVisualMusicFrame::OnTimerFPS(wxTimerEvent& event){
	SetStatusText(wxString::Format("FPS %d", m_fps));
	m_fps = 0;
}


