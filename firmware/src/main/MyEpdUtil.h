/*
 * MyEpdUtil.h
 *
 *  Created on: 12.01.2022
 *      Author: martin
 */

#ifndef MYEPDUTIL_H_
#define MYEPDUTIL_H_
#include <epd_driver.h>
#include <vector>
#include <stdio.h>
#include <stdarg.h>

struct Triangle {
  int x0; int y0;
  int x1; int y1;
  int x2; int y2;
  Triangle(int x0_, int y0_, int x1_, int y1_, int x2_, int y2_) :
    x0(x0_), y0(y0_), x1(x1_), y1(y1_), x2(x2_), y2(y2_)
  {}
};

enum EpdDrawError epd_rewrite_string(const EpdFont *font, const char *string,
		EpdRect &r, int x, int y, const EpdFontProperties props =
				epd_font_properties_default());
void create_arc_from_triangles(int x, int y, int start_angle, int arc_degree,
		int rx, int ry, int w, std::vector<Triangle> &triangles);

// #########################################################################
// Draw a circular or elliptical arc with a defined thickness
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// arc_degree = 1 - 360
// rx = x axis radius
// yx = y axis radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same then an arc of a circle is drawnint epd_fill_arc(int x, int y, int start_angle, int arc_degree, int rx, int ry, int w, uint8_t colour, uint8_t *framebuffer)
void epd_fill_arc(int x, int y, int start_angle, int arc_degree, int rx, int ry,
		int w, uint8_t colour, uint8_t *framebuffer);

inline char *format(char *buffer, const char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	vsprintf(buffer, fmt, arg);
	va_end(arg);
	return buffer;
}

#endif /* MYEPDUTIL_H_ */
