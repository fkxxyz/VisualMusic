#pragma once
#include "stdafx.h"

#include "colordef.h"

class DrawTemplate {
public:
	DrawTemplate(color_t *data, int width, int height, void (*draw_func)(void *, DrawTemplate *), void *draw_param);

	void Rectangle(color_t color, int x1, int y1, int x2, int y2);

protected:
	color_t *m_data;
	int m_width;
	int m_height;
};

inline DrawTemplate::DrawTemplate(
		color_t *data,
		int width,
		int height,
		void (*draw_func)(void *, DrawTemplate *),
		void *draw_param
		):
	m_data(data),
	m_width(width),
	m_height(height)
{
	draw_func(draw_param, this);
}

inline void DrawTemplate::Rectangle(color_t color, int x1, int y1, int x2, int y2){
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 > m_width) x2 = m_width;
	if (y2 > m_height) y2 = m_height;
	for (int x = x1; x < x2; x++)
		for (int y = y1; y < y2; y++)
			m_data[x + y * m_width] = color;
}



