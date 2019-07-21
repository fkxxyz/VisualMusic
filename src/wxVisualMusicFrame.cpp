
#include "stdafx.h"
#include "wxVisualMusicFrame.h"
#include <wx/file.h>
#include "constants.h"

enum wxID
{
	wxID_BASE = 1,

	wxID_PLAYER_PLAY_PAUSE,
	wxID_PLAYER_STOP,
	wxID_PLAYER_SETPOSITION,

	wxID_VIEW_BAR,
	wxID_VIEW_2D,
};

wxBEGIN_EVENT_TABLE(wxVisualMusicFrame, wxFrame)
	EVT_MENU(wxID_OPEN,  wxVisualMusicFrame::OnOpen)
	EVT_MENU(wxID_EXIT,  wxVisualMusicFrame::OnExit)
	EVT_MENU(wxID_ABOUT, wxVisualMusicFrame::OnAbout)
	EVT_PAINT(wxVisualMusicFrame::OnPaint)
wxEND_EVENT_TABLE()

wxVisualMusicFrame::wxVisualMusicFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	:wxFrame(NULL, wxID_ANY, title, pos, size)
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
				VIEW_TIME_PIXEL_HEIGHT * N_TIME + VIEW_CONTROL_SPACING + VIEW_BUTTON_HEIGHT + VIEW_CONTROL_SPACING
			);

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

	//m_file_decoder.Run(static_cast<const char*>(dialog.GetPath()));
}

void wxVisualMusicFrame::OnExit(wxCommandEvent& event){
	Close(true);

}
void wxVisualMusicFrame::OnAbout(wxCommandEvent& event){
	wxMessageBox( "This is a wxWidgets' Hello world sample",
					  "About Hello World", wxOK | wxICON_INFORMATION );
}

void wxVisualMusicFrame::OnPaint(wxPaintEvent& event){

}


