/*
 * MyEpdUtil.cpp
 *
 *  Created on: 12.01.2022
 *      Author: martin
 */
#include "MyEpdUtil.h"
#include <epd_highlevel.h>

#include <math.h>
#include <utility>

extern EpdiyHighlevelState hl;

#define DEG2RAD 0.0174527F
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
		int w, uint8_t colour, uint8_t *framebuffer) {
	uint8_t seg = 1; // Segments are 3 degrees wide = 120 segments for 360 degrees
	uint8_t inc = 1; // Draw segments every 3 degrees, increase to 6 for segmented ring
	int seg_count = arc_degree / seg;

	// Calculate first pair of coordinates for segment start
	float sx = cos((start_angle - 90) * DEG2RAD);
	float sy = sin((start_angle - 90) * DEG2RAD);
	uint16_t x0 = sx * (rx - w) + x;
	uint16_t y0 = sy * (ry - w) + y;
	uint16_t x1 = sx * rx + x;
	uint16_t y1 = sy * ry + y;

	// Draw colour blocks every inc degrees
	for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
		// Calculate pair of coordinates for segment end
		float sx2 = cos((i + seg - 90) * DEG2RAD);
		float sy2 = sin((i + seg - 90) * DEG2RAD);
		int x2 = sx2 * (rx - w) + x;
		int y2 = sy2 * (ry - w) + y;
		int x3 = sx2 * rx + x;
		int y3 = sy2 * ry + y;
		epd_fill_triangle(x0, y0, x1, y1, x2, y2, colour, framebuffer);
		epd_fill_triangle(x1, y1, x2, y2, x3, y3, colour, framebuffer);
		// Copy segment end to sgement start for next segment
		x0 = x2;
		y0 = y2;
		x1 = x3;
		y1 = y3;
	}
}

// x,y == coords of centre of arc
// start_angle = 0 - 359
// arc_degree = 1 - 360
// rx = x axis radius
// yx = y axis radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same then an arc of a circle is drawnint epd_fill_arc(int x, int y, int start_angle, int arc_degree, int rx, int ry, int w, uint8_t colour, uint8_t *framebuffer)
void create_arc_from_triangles(int x, int y, int start_angle, int arc_degree,
		int rx, int ry, int w, std::vector<Triangle> &triangles) {
	triangles.clear();
	uint8_t seg = 6; // Segments are 3 degrees wide = 120 segments for 360 degrees
	uint8_t inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring
	int seg_count = arc_degree / seg;

	// Calculate first pair of coordinates for segment start
	float sx = cos((start_angle - 90) * DEG2RAD);
	float sy = sin((start_angle - 90) * DEG2RAD);
	uint16_t x0 = sx * (rx - w) + x;
	uint16_t y0 = sy * (ry - w) + y;
	uint16_t x1 = sx * rx + x;
	uint16_t y1 = sy * ry + y;

	// Draw colour blocks every inc degrees
	for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
		// Calculate pair of coordinates for segment end
		float sx2 = cos((i + seg - 90) * DEG2RAD);
		float sy2 = sin((i + seg - 90) * DEG2RAD);
		int x2 = sx2 * (rx - w) + x;
		int y2 = sy2 * (ry - w) + y;
		int x3 = sx2 * rx + x;
		int y3 = sy2 * ry + y;
		triangles.push_back(Triangle(x0, y0, x1, y1, x2, y2));
		triangles.push_back(Triangle(x1, y1, x2, y2, x3, y3));
		// Copy segment end to sgement start for next segment
		x0 = x2;
		y0 = y2;
		x1 = x3;
		y1 = y3;
	}
}




enum EpdDrawError epd_rewrite_string(const EpdFont *font, const char *string,
		EpdRect &r, int x, int y, const EpdFontProperties props) {
	if (r.width != 0)
		epd_fill_rect(r, 0xFF, epd_hl_get_framebuffer(&hl));
	epd_get_text_bounds(font, string, &x, &y, &r.x, &r.y, &r.width, &r.height,
			&props);
	r.x -= (props.flags == EPD_DRAW_ALIGN_RIGHT) ? r.width :
			(props.flags == EPD_DRAW_ALIGN_CENTER) ? r.width / 2 : 0;
	r.y -= r.height;
	r.height += 10;

	return epd_write_string(font, string, &x, &y,
			(uint8_t*) epd_hl_get_framebuffer(&hl), &props);
}


