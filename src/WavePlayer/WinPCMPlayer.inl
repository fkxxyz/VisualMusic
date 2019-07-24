
#include "stdafx.h"
#include "WinPCMPlayer.h"


#if defined(_WIN32) || defined(__WIN32__)

template <int nBufferNumber>
inline CWinPCMPlayer<nBufferNumber>::CWinPCMPlayer():
	dwStatus(PCM_CLOSED)
{
	assert(nBufferNumber >= 2);
}

template <int nBufferNumber>
inline CWinPCMPlayer<nBufferNumber>::~CWinPCMPlayer(){
	
}

template <int nBufferNumber>
inline void CALLBACK CWinPCMPlayer<nBufferNumber>::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2){
	CWinPCMPlayer<nBufferNumber> &Obj = *(CWinPCMPlayer<nBufferNumber> *)dwInstance;

	switch (uMsg){
	case MM_WOM_OPEN:
		break;
	case MM_WOM_CLOSE:
		break;
	case MM_WOM_DONE:
		Obj.AHeaderDone((PWAVEHDR)dwParam1);
		break;
	}
}


// Open device
template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::Open(WAVEFORMATEX *pFormat){
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);

	assert(dwStatus == PCM_CLOSED);

	stWaveFormat = *pFormat;

	// Check wave format metadata
	if (!ValidWaveFormat())
		return FALSE;

	// Initialize event objects
	if (!AHeaderDoneEvent && !AHeaderDoneEvent.Create())
		return FALSE;
	if (!AllHeaderIdle && !AllHeaderIdle.Create())
		return FALSE;

	// Open wave device
	if (!objMMWO.Open(&stWaveFormat, (WAVEOUTCALLBACKPROC)&waveOutProc, (DWORD_PTR)this))
		return FALSE;

	// Initialize the head of all waves
	ZeroMemory(header, sizeof(header));
	for (int i = 0; i < nBufferNumber; i++){
		if (!objMMWO.PrepareHeader(&header[i], sizeof(WAVEHDR)))
			return FALSE;
	}

	// Deal with event objects
	nIdleHeader = nBufferNumber;
	if (!AHeaderDoneEvent.Reset())
		return FALSE;
	if (!AllHeaderIdle.Set())
		return FALSE;

	dwStatus = PCM_READY;
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::Close(){
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);
	assert(dwStatus == PCM_READY);

	for (int i = 0; i < nBufferNumber; i++){
		if (!objMMWO.UnprepareHeader(&header[i], sizeof(WAVEHDR)))
			return FALSE;
	}

	// Close wave device
	if (!objMMWO.Close())
		return FALSE;

	dwStatus = PCM_CLOSED;
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::Clear(){
	if (dwStatus > PCM_READY)
		if (!Reset()) return FALSE;
	if (dwStatus == PCM_READY)
		if (!Close()) return FALSE;
	return TRUE;
}

template <int nBufferNumber>
inline PWAVEHDR CWinPCMPlayer<nBufferNumber>::WaitAndGetIdleHeader(){
	assert(dwStatus != PCM_CLOSED);
	assert(nIdleHeader >= 0);

	if (nIdleHeader == 0){
		if (!WaitAPcmDone())
			return NULL;
	}

	assert(nIdleHeader != 0);
	for (int i = 0; i < nBufferNumber; i++){
		if (!(header[i].dwFlags & WHDR_INQUEUE)){
			return &header[i];
		}
	}
	assert(0);
	return NULL;
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::Insert(void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes){
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);
	assert(dwStatus != PCM_CLOSED);
	assert(PcmBuffer);
	assert(dwPcmSize > 0);

	// Get a idle header
	PWAVEHDR pHeader = WaitAndGetIdleHeader();

	// Set this idle header
	SetHeader(pHeader, PcmBuffer, dwPcmSize, dwLoopTimes);

	if (!AllHeaderIdle.Reset())
		return FALSE;

	// Add this header to the queue
	if (!objMMWO.Write(pHeader, sizeof(WAVEHDR)))
		return FALSE;
	assert(pHeader->dwFlags & WHDR_INQUEUE);

	// Deal with the count of idle header
	nIdleHeader--;
	assert(nIdleHeader >= 0);

	dwStatus = PCM_PLAYING;
	return TRUE;
}


template <int nBufferNumber>
inline void CWinPCMPlayer<nBufferNumber>::SetHeader(PWAVEHDR pHeader, void *PcmBuffer, DWORD dwPcmSize, DWORD dwLoopTimes){
	assert(!(pHeader->dwFlags & WHDR_INQUEUE));
	assert(PcmBuffer);

	pHeader->lpData = (LPSTR)PcmBuffer;
	pHeader->dwBufferLength = dwPcmSize;
	if (dwLoopTimes < 2){
		// Remove the loop flag when dwLoopTimes has no cycles
		pHeader->dwFlags &= 0x1F ^ (WHDR_BEGINLOOP | WHDR_ENDLOOP);
	} else {
		// Set the cycles when dwLoopTimes has cycles
		pHeader->dwLoops = dwLoopTimes;
		pHeader->dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
	}
}

template <int nBufferNumber>
inline void CWinPCMPlayer<nBufferNumber>::AHeaderDone(PWAVEHDR pHeader){
	// Set the count of idle headers
	nIdleHeader++;
	assert(nIdleHeader <= nBufferNumber);

	// Set related events according to the number of idle heads
	if (nIdleHeader == nBufferNumber){
		dwStatus = PCM_READY;
		if (!AllHeaderIdle.Set())
			assert(0);
	} else { // nIdleHeader < nBufferNumber
		assert(!AllHeaderIdle.IsSeted());
	}
	if (!AHeaderDoneEvent.Set())
		assert(0);
	if (!AHeaderDoneEvent.Reset())
		assert(0);
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::WaitAPcmDone(){
	assert(dwStatus == PCM_PLAYING || dwStatus == PCM_PAUSE);
	if (!AHeaderDoneEvent.Wait()){
		assert(0);
		return FALSE;
	}
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::Wait(){
	assert(dwStatus == PCM_PLAYING || dwStatus == PCM_PAUSE);

	if (!AllHeaderIdle.Wait())
		return FALSE;

	assert(dwStatus == PCM_READY);
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::Pause(){
	assert(dwStatus == PCM_PLAYING);
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);
	if (dwStatus == PCM_PAUSE) return TRUE;

	if (!objMMWO.Pause())
		return FALSE;

	dwStatus = PCM_PAUSE;
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::Resume(){
	assert(dwStatus == PCM_PAUSE);
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);
	if (dwStatus == PCM_PLAYING) return TRUE;

	if (!objMMWO.Restart())
		return FALSE;

	dwStatus = PCM_PLAYING;
	return TRUE;
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::Reset(){
	assert(dwStatus != PCM_CLOSED);
	CCriticalSectionFunction objCriticalFunction(m_CriticalSection);

	if (!objMMWO.Reset())
		return FALSE;

	return dwStatus == PCM_READY;
}

template <int nBufferNumber>
inline PCM_STATUS CWinPCMPlayer<nBufferNumber>::GetStatus(){
	return dwStatus;
}

template <int nBufferNumber>
inline MMRESULT CWinPCMPlayer<nBufferNumber>::GetLastResult(){
	return objMMWO.GetLastResult();
}

template <int nBufferNumber>
inline DWORD CWinPCMPlayer<nBufferNumber>::GetBytesPos(){
	assert(dwStatus != PCM_CLOSED);

	MMTIME stMMTime;
	if (!objMMWO.GetPosition(&stMMTime, sizeof(stMMTime))){
		assert(0);
		return -1;
	}
	return stMMTime.u.cb;
}

template <int nBufferNumber>
inline DWORD CWinPCMPlayer<nBufferNumber>::GetTimePos(){
	return (DWORD)((LONGLONG)GetBytesPos() * 1000 / stWaveFormat.nAvgBytesPerSec);
}

template <int nBufferNumber>
inline CWinMMWaveOut *CWinPCMPlayer<nBufferNumber>::GetMMwaveOut(){
	return &objMMWO;
}

template <int nBufferNumber>
inline BOOL CWinPCMPlayer<nBufferNumber>::ValidWaveFormat(){
	// Must be pcm format
	if (stWaveFormat.wFormatTag != WAVE_FORMAT_PCM && stWaveFormat.wFormatTag != 3)
		return FALSE;

	// Check channels
	if (stWaveFormat.nChannels < 1 || stWaveFormat.nChannels >2)
		return FALSE;

	// Check sampling rate
	if (stWaveFormat.nSamplesPerSec < 20 || stWaveFormat.nSamplesPerSec > 176400)
		return FALSE;

	// Check bits of sample
	if (stWaveFormat.wBitsPerSample != 8 && stWaveFormat.wBitsPerSample != 16 && stWaveFormat.wBitsPerSample != 32)
		return FALSE;

	// Check data block align
	if (
		stWaveFormat.nBlockAlign 
			!=
		(stWaveFormat.wBitsPerSample >> 3) * stWaveFormat.nChannels
		)
		return FALSE;

	// Check bytes per seconds
	if (
		stWaveFormat.nAvgBytesPerSec
			!=
		stWaveFormat.nBlockAlign * stWaveFormat.nSamplesPerSec
		)
		return FALSE;

	// Check extera data
	if (stWaveFormat.cbSize != 0)
		goto lErrBadFormat;

	return TRUE;

lErrBadFormat:
	SetLastError(ERROR_BAD_FORMAT);
	return FALSE;
}



#endif
