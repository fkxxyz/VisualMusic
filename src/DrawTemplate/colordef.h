#pragma once
#include "stdafx.h"

#ifdef USE_WX

typedef struct _color_t {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} color_t;


inline color_t color(unsigned char red, unsigned char green, unsigned char blue){
	color_t c = {red, green, blue};
	return c;
}


#endif
