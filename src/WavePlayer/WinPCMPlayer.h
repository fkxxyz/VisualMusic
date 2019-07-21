
// WinPCMPlayer.h

#pragma once
#include "stdafx.h"
#include "WinEvent.h"
#include "WinCritialSection.h"
#include "WinMMWaveOut.h"


#if defined(_WIN32) || defined(__WIN32__)

// PCM Player status
enum PCM_STATUS {
	PCM_CLOSED,
	PCM_READY,
	PCM_PLAYING,
	PCM_PAUSE
};

template <int nBufferNumber = 2>
class CWinPCMPlayer {
public:

	CWinPCMPlayer();
	~CWinPCMPlayer();

	// Open the wave device (you must set the parameters in pFormat)
	BOOL Open(WAVEFORMATEX *pFormat);

	// Close the wave device (Can't turn off the device during playing, stop first)
	BOOL Close();

	// Force stop and close, clean up the player
	BOOL Clear();

	// Add a PCM to the play sequence
	BOOL Insert(void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes = 1);

	// Waiting for a certain PCM to finish playing
	BOOL WaitAPcmDone();

	// Waiting to play the player until it is finished
	BOOL Wait();

	// Pause
	BOOL Pause();

	// Resume
	BOOL Resume();

	// Stop playing and clear the timer
	BOOL Reset();

	// Get current status
	enum PCM_STATUS GetStatus();

	// Get the current playing position (bytes)
	DWORD GetBytesPos();

	// Get the current playing position (millseconds)
	DWORD GetTimePos();

	// Get the result of the last error
	MMRESULT GetLastResult();

	// Get the current associated waveform output object
	CWinMMWaveOut *GetMMwaveOut();

private:
	WAVEFORMATEX stWaveFormat;
	CWinMMWaveOut objMMWO;

	WAVEHDR header[nBufferNumber];
	int nIdleHeader;
	// set header
	void SetHeader(PWAVEHDR pHeader, void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes = 1);

	// Critical section object
	// (the function of each action is a critical section,
	//   and multiple threads cannot execute multiple commands at the same time)
	CCriticalSection m_CriticalSection;

	// waveOut Callback function (executed after each wave header is played)
	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	void AHeaderDone(PWAVEHDR pHeader);

	CEvent AHeaderDoneEvent; // Trigger once and then reset when a header has been idle after playback is complete
	CEvent AllHeaderIdle; // Triggered when all headers are idle after playback is complete
	PWAVEHDR WaitAndGetIdleHeader();  // Get an idle header, wait for free if it is not available

	enum PCM_STATUS dwStatus;
	BOOL ValidWaveFormat();
};

#include "WinPCMPlayer.inl"

#endif

