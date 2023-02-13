/*
 * SpeedometerWidget.cpp
 *
 *  Created on: 12.01.2022
 *      Author: martin
 */

#include "SpeedometerWidget.h"
#include <epd_highlevel.h>

extern EpdiyHighlevelState hl;

SpeedometerWidget::SpeedometerWidget(const EpdFont &large, const EpdFont &medium, const EpdFont &small) :
				large_(large), medium_(medium), small_(small), rScala_()
{}

SpeedometerWidget::~SpeedometerWidget()
{}


void SpeedometerWidget::paint (const EpdRect &rect)
{
	char buf[8];
	EpdFontProperties font_props = epd_font_properties_default();
	font_props.flags = EPD_DRAW_ALIGN_RIGHT;
	EpdRect r1 = {
	  .x = rect.x +50,
	  .y = rect.y + small_.ascender,
	  .width = rect.width/5,
	  .height = rect.height - 2*small_.ascender,
	};
	epd_fill_rect(r1, 0x00, epd_hl_get_framebuffer(&hl));
	rScala_.x = r1.x + 5;
	rScala_.y = r1.y + 5;
	rScala_.width = r1.width - 10;
	rScala_.height = r1.height - 10;
	epd_fill_rect(rScala_, 0xFF, epd_hl_get_framebuffer(&hl));
	for(int i=0; i < 9; i++) {
		EpdRect r3 = {
		  .x = r1.x + r1.width-2,
		  .y = r1.y + (r1.height -5)*i /8,
		  .width = 10*(2 - i%2),
		  .height = 5,
		};
		epd_fill_rect(r3, 0x00, epd_hl_get_framebuffer(&hl));
		int xPos = rect.x+rect.width - 10;
		int yPos = r3.y+r3.height/2 + medium_.ascender/2 + medium_.descender/2;
		if(i%2 == 0)
			epd_write_string(&medium_, format(buf, "%02d", 60 - 5*i), &xPos, &yPos, epd_hl_get_framebuffer(&hl), &font_props);

	}
}


void SpeedometerWidget::update(const LiLyGoEPD47 &vario_data)
{
	float speed = vario_data.getSpeed().isValid() ? vario_data.getSpeed().kmph() : 0.F;
	float scalaRel = (speed <= 20.F) ? 0.F : (speed >= 60.F) ? 100.F : (speed - 20.F) * 100 / (60 - 20);

	EpdRect rWhite = {
	  .x = rScala_.x,
	  .y = rScala_.y,
	  .width = rScala_.width,
	  .height = static_cast<int>(rScala_.height * (100 - scalaRel) / 100),
	};

	EpdRect rBlack = {
	  .x = rScala_.x,
	  .y = rScala_.y + rWhite.height,
	  .width = rScala_.width,
	  .height = static_cast<int>(rScala_.height * scalaRel / 100),
	};
	epd_fill_rect(rWhite, 0xFF, epd_hl_get_framebuffer(&hl));
	epd_fill_rect(rBlack, 0x00, epd_hl_get_framebuffer(&hl));

}
