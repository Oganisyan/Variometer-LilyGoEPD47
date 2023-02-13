/*
 * AnalogVarioWidget.cpp
 *
 *  Created on: 12.01.2022
 *      Author: martin
 */

#include "AnalogVarioWidget.h"
#include <epd_driver.h>
#include <epd_highlevel.h>

extern EpdiyHighlevelState hl;

AnalogVarioWidget::AnalogVarioWidget(const EpdFont &large, const EpdFont &medium, const EpdFont &small) :
		large_(large), medium_(medium), small_(small), rectCompass_({.x=0, .y=0, .width=0, .height=0,})
{}

AnalogVarioWidget::~AnalogVarioWidget() {
}

void AnalogVarioWidget::calcPositons(const EpdRect &rectVario)
{
	EpdRect rV = {
			.x = rectVario.x+50,
			.y = rectVario.y+25,
			.width = rectVario.width-50,
			.height = rectVario.height-50,
	};
	varioArcBasePoints[VARIO_ARC_1START_ANGLE] = VARIO_ARC_START_ANGLE;
	varioArcBasePoints[VARIO_ARC_1_ANGLE] = VARIO_ARC_ANGLE;
	varioArcBasePoints[VARIO_RX] = rV.width / 2;
	varioArcBasePoints[VARIO_RY] = rV.height / 2;

	varioArcBasePoints[VARIO_ARC_1X] = rV.x + varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_ARC_1Y] = rV.y + varioArcBasePoints[VARIO_RY];
	varioArcBasePoints[VARIO_ARC_1RX] = varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_ARC_1RY] = varioArcBasePoints[VARIO_RY];
	varioArcBasePoints[VARIO_ARC_1W] = varioArcBasePoints[VARIO_RX] / 2;
	varioArcBasePoints[VARIO_ARC_2X] = rV.x + varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_ARC_2Y] = rV.y + varioArcBasePoints[VARIO_RY];
	varioArcBasePoints[VARIO_ARC_2START_ANGLE] =
			varioArcBasePoints[VARIO_ARC_1START_ANGLE] + 1;
	varioArcBasePoints[VARIO_ARC_2_ANGLE] =
			varioArcBasePoints[VARIO_ARC_1_ANGLE] - 2;
	varioArcBasePoints[VARIO_ARC_2RX] = varioArcBasePoints[VARIO_ARC_1RX] - 3;
	varioArcBasePoints[VARIO_ARC_2RY] = varioArcBasePoints[VARIO_ARC_1RY] - 3;
	varioArcBasePoints[VARIO_ARC_2W] = varioArcBasePoints[VARIO_RX] / 2 - 6;

	varioArcBasePoints[VARIO_0X] = rV.x - 10;
	varioArcBasePoints[VARIO_0Y] = rV.y + varioArcBasePoints[VARIO_RY] + 14;
	varioArcBasePoints[VARIO_1PX] = rV.x + 0.15F * varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_1PY] = rV.y + 0.4F * varioArcBasePoints[VARIO_RY];
	varioArcBasePoints[VARIO_1MX] = rV.x + 0.15F * varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_1MY] = rV.y + 1.6F * varioArcBasePoints[VARIO_RY]
			+ 28;
	varioArcBasePoints[VARIO_2PX] = rV.x + 0.6F * varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_2PY] = rV.y + 0.1F * varioArcBasePoints[VARIO_RY];
	varioArcBasePoints[VARIO_2MX] = rV.x + 0.6F * varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_2MY] = rV.y + 1.9F * varioArcBasePoints[VARIO_RY]
			+ 28;
	varioArcBasePoints[VARIO_3PX] = rV.x + 1.2F * varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_3PY] = rV.y;
	varioArcBasePoints[VARIO_3MX] = rV.x + 1.2F * varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_3MY] = rV.y + 2 * varioArcBasePoints[VARIO_RY]
			+ 28;
	varioArcBasePoints[VARIO_4PX] = rV.x + 1.8F * varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_4PY] = rV.y + 0.3F * varioArcBasePoints[VARIO_RY];
	varioArcBasePoints[VARIO_4MX] = rV.x + 1.8F * varioArcBasePoints[VARIO_RX];
	varioArcBasePoints[VARIO_4MY] = rV.y + 1.7F * varioArcBasePoints[VARIO_RY]
			+ 28;

	int arc = varioArcBasePoints[VARIO_ARC_2_ANGLE] / SCALA_SIZE;
	for (int i = 0; i < SCALA_SIZE / 2; i++) {
		create_arc_from_triangles(varioArcBasePoints[VARIO_ARC_2X],
				varioArcBasePoints[VARIO_ARC_2Y],      						// x, y
				270 + i * arc + 1,                                     		// start
				arc - 1,                                                  	// arc
				varioArcBasePoints[VARIO_ARC_2RX] - 4,
				varioArcBasePoints[VARIO_ARC_2RY] - 4,
				varioArcBasePoints[VARIO_ARC_2W] - 8, varioArcs[i].first);
		create_arc_from_triangles(varioArcBasePoints[VARIO_ARC_2X],
				varioArcBasePoints[VARIO_ARC_2Y],      						// x, y
				270 - (i + 1) * arc - 1,                               		// start
				arc - 1,                                                 	// arc
				varioArcBasePoints[VARIO_ARC_2RX] - 4,
				varioArcBasePoints[VARIO_ARC_2RY] - 4,
				varioArcBasePoints[VARIO_ARC_2W] - 8,
				varioArcs[i + SCALA_SIZE / 2].first);
	}

}
void AnalogVarioWidget::paint (const EpdRect &rectVario)
{
	calcPositons(rectVario);
	epd_hl_set_all_white(&hl);



	epd_fill_arc(varioArcBasePoints[VARIO_ARC_1X],
			varioArcBasePoints[VARIO_ARC_1Y],
			varioArcBasePoints[VARIO_ARC_1START_ANGLE],
			varioArcBasePoints[VARIO_ARC_1_ANGLE],
			varioArcBasePoints[VARIO_ARC_1RX],
			varioArcBasePoints[VARIO_ARC_1RY], varioArcBasePoints[VARIO_ARC_1W],
			0x00, epd_hl_get_framebuffer(&hl));
	epd_fill_arc(varioArcBasePoints[VARIO_ARC_2X],
			varioArcBasePoints[VARIO_ARC_2Y],
			varioArcBasePoints[VARIO_ARC_2START_ANGLE],
			varioArcBasePoints[VARIO_ARC_2_ANGLE],
			varioArcBasePoints[VARIO_ARC_2RX],
			varioArcBasePoints[VARIO_ARC_2RY], varioArcBasePoints[VARIO_ARC_2W],
			0xFF, epd_hl_get_framebuffer(&hl));

	EpdFontProperties font_props = epd_font_properties_default();
	font_props.flags = EPD_DRAW_ALIGN_RIGHT;

	int pX = varioArcBasePoints[VARIO_0X];
	int pY = varioArcBasePoints[VARIO_0Y];
	epd_write_string(&small_, "0", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);

	pX = varioArcBasePoints[VARIO_1PX];
	pY = varioArcBasePoints[VARIO_1PY];
	epd_write_string(&small_, "1", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);

	pX = varioArcBasePoints[VARIO_1MX];
	pY = varioArcBasePoints[VARIO_1MY];
	epd_write_string(&small_, "1", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);

	pX = varioArcBasePoints[VARIO_2PX];
	pY = varioArcBasePoints[VARIO_2PY];
	epd_write_string(&small_, "2", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);

	pX = varioArcBasePoints[VARIO_2MX];
	pY = varioArcBasePoints[VARIO_2MY];
	epd_write_string(&small_, "2", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);

	pX = varioArcBasePoints[VARIO_3PX];
	pY = varioArcBasePoints[VARIO_3PY];
	epd_write_string(&small_, "3", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);

	pX = varioArcBasePoints[VARIO_3MX];
	pY = varioArcBasePoints[VARIO_3MY];
	epd_write_string(&small_, "3", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);

	pX = varioArcBasePoints[VARIO_4PX];
	pY = varioArcBasePoints[VARIO_4PY];
	epd_write_string(&small_, "4", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);

	pX = varioArcBasePoints[VARIO_4MX];
	pY = varioArcBasePoints[VARIO_4MY];
	epd_write_string(&small_, "4", &pX, &pY, epd_hl_get_framebuffer(&hl),
			&font_props);
}
void AnalogVarioWidget::update(const LiLyGoEPD47 &vario_data)
{
	EpdFontProperties font_props = epd_font_properties_default();
	font_props.flags = EPD_DRAW_ALIGN_CENTER;
	if (vario_data.getCourse().isValid()) { //Speed > 4kmh
		float tmp = vario_data.getCourse().deg() / 15.F;
		int degris = tmp + 0.5F - (tmp < 0);

		// Compass
		epd_rewrite_string(&large_,
				//d.vario_data.speed < 3.F ? u8"\ue038" : //speed ~ 0
				degris < 1 ? u8"\ue020" : degris < 2 ? u8"\ue021" :degris < 3 ? u8"\ue022" : degris < 4 ? u8"\ue023" :
				degris < 5 ? u8"\ue024" : degris < 6 ? u8"\ue025" :degris < 7 ? u8"\ue026" : degris < 8 ? u8"\ue027" :
				degris < 9 ? u8"\ue028" : degris < 10 ? u8"\ue029" :degris < 11 ? u8"\ue02a" : degris < 12 ? u8"\ue02b" :
				degris < 13 ? u8"\ue02c" : degris < 14 ? u8"\ue02d" :degris < 15 ? u8"\ue02e" : degris < 16 ? u8"\ue02f" :
				degris < 17 ? u8"\ue030" : degris < 18 ? u8"\ue031" :degris < 19 ? u8"\ue032" : degris < 20 ? u8"\ue033" :
				degris < 21 ? u8"\ue034" : degris < 22 ? u8"\ue035" :degris < 23 ? u8"\ue036" : u8"\ue037",
				rectCompass_, varioArcBasePoints[VARIO_ARC_1X],
				varioArcBasePoints[VARIO_ARC_1Y] + large_.ascender/2 + large_.descender/2, font_props);
	} else {
		epd_rewrite_string(&large_,
			"",
			rectCompass_, varioArcBasePoints[VARIO_ARC_1X],
			varioArcBasePoints[VARIO_ARC_1Y] + large_.ascender/2 - large_.descender/2, font_props);
	}

	int scala = static_cast<int>(vario_data.getVario() * 2.5F);
	if (scala < -10)
		scala = -10;
	if (scala > 10)
		scala = 10;
	for (int i = 0; i < SCALA_SIZE; i++) {
		uint8_t color =
				(i < SCALA_SIZE / 2 && scala > 0 && i < scala)
						|| (i >= SCALA_SIZE / 2 && scala < 0
								&& (i - SCALA_SIZE / 2) < -scala) ? 0x00 : 0xFF;
		if (varioArcs[i].second != color) {
			varioArcs[i].second = color;
			std::vector<Triangle> &t = varioArcs[i].first;
			for (int j = 0; j < t.size(); j++) {
				epd_fill_triangle(t[j].x0, t[j].y0, t[j].x1, t[j].y1, t[j].x2,
						t[j].y2, color, epd_hl_get_framebuffer(&hl));
			}
		}
	}

}
