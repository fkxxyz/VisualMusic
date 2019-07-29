#pragma once

#include <iostream>
using namespace std;

#ifdef linux

#include <unistd.h>
#include <cassert>
inline void Sleep(unsigned long milliseconds){
	int result = usleep(static_cast<useconds_t>(milliseconds * 1000));
	assert(result == 0);
}

#define USE_WX

#elif defined(_WIN32) || defined(__WIN32__)

#include <Windows.h>

#define USE_MFC

#endif


#ifdef USE_WX

#include <wx/wx.h>

#endif

#include "VisualMusicController.h"



