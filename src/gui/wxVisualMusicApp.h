#pragma once
#include "stdafx.h"

#ifdef USE_WX

class wxVisualMusicApp: public wxApp {
public:
	virtual bool OnInit();
};

#endif

