
#include "stdafx.h"
#include "BaseAudioDecoder.h"

BaseAudioDecoder::BaseAudioDecoder():
	m_channels(0),
	m_event_stopped(true, true)
{

}

BaseAudioDecoder::~BaseAudioDecoder(){}


