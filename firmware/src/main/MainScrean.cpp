#include "MainScrean.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <math.h>
#include <vector>
#include <utility>

extern EpdiyHighlevelState hl;


void MainScrean::paintCallBack(void *parameter) {
	for (;;) {
		static_cast<MainScrean*>(parameter)->paint();
        taskDelay(50);
	}
}

MainScrean::MainScrean(const EpdFont &large, const EpdFont &medium, const EpdFont &small) :
		mutex_(portMUX_INITIALIZER_UNLOCKED), large_(large), medium_(medium), small_(small),
		textRects(),
		rectVario_(), rectSpeed_(), rectHeader_(), rectControl_(),
		rectTabs_(),
		vario_(large, medium, small),
		speed_(large, medium, small),
		data_()
{
}


data_t MainScrean::getData(bool forPaint) {
	taskENTER_CRITICAL(&mutex_);
	data_t d = data_;
	if (forPaint) {
		data_.paintScrean_ = false;
		data_.updateScrean_ = false;
	}
	taskEXIT_CRITICAL(&mutex_);
	return d;
}

void MainScrean::setData(const data_t &data) {
	taskENTER_CRITICAL(&mutex_);
	data_ = data;
	taskEXIT_CRITICAL(&mutex_);
}

TaskHandle_t MainScrean::begin() {
	// Set Update Flags
	data_t d = getData();
	d.paintScrean_ = true;
	d.updateScrean_ = true;
	setData(d);
	TaskHandle_t xHandle = NULL;
	xTaskCreatePinnedToCore(MainScrean::paintCallBack, // Function that should be called
			"Update Screan",       // Name of the task (for debugging)
			8 * 1024,                // Stack size (bytes)
			this,                  // Parameter to pass
			20,                     // Task priority
			&xHandle,              // Task handle
			1                      // Core you want to run the task on (0 or 1)
			);
	if (!xHandle) {
		printf("\n Error: Cant Create Update Screan Task\n");
	} else {
		printf("Start Widget...\n");
	}
	return xHandle;
}

//////////////////////////////////////////////////////////////////
// Core 1 Tasks

int repaintHitCount = 0;

void MainScrean::paint() {
	data_t d = getData(true);
	if (d.paintScrean_ || d.updateScrean_) {
		bool update = false;
		if (d.paintScrean_ || (repaintHitCount > 10 && d.updateScrean_)) {
			update = true;
			repaintHitCount = 0;
			memset(hl.front_fb, 0xff, EPD_WIDTH / 2 * EPD_HEIGHT);
			calcEpdRects();
		}
		if (d.updateScrean_) {
			update = true;
			repaintHitCount++;
			updateScrean(d);
		}
		if(update) {
			epd_hl_update_screen(&hl, MODE_DU, epd_ambient_temperature());
		}
	}
	taskDelay(100);
}

void MainScrean::paintTab(const EpdRect &r, const char *info, const char *value,
		const EpdFont &vFont) {
	EpdFontProperties font_props = epd_font_properties_default();
	font_props.flags = EPD_DRAW_ALIGN_RIGHT;
	epd_fill_rect(r, 0xFF, epd_hl_get_framebuffer(&hl));
	int xPos = r.x + r.width - 15;
	int yPos = r.y + small_.ascender + 5;
	epd_write_string(&small_, info, &xPos, &yPos, epd_hl_get_framebuffer(&hl), &font_props);
	yPos = r.y + small_.ascender - small_.descender + vFont.ascender + 5;
	epd_write_string(&vFont, value, &xPos, &yPos, epd_hl_get_framebuffer(&hl), &font_props);
}


void MainScrean::paintHeader(const data_t &d) {
	char tmpBuf[32];
	EpdFontProperties font_props = epd_font_properties_default();
	epd_fill_rect(rectHeader_, 0xFF, epd_hl_get_framebuffer(&hl));
	font_props.flags = EPD_DRAW_ALIGN_LEFT;
	int xPos = rectHeader_.x + 15;
	int yPos = rectHeader_.y + rectHeader_.height/2
			+ small_.ascender/2 - small_.descender/2;
	epd_write_string(&small_,
			format(tmpBuf, "%s %s %s %d%%",
					d.vario_data.getLocation().isValid() ? "\ue003": "\ue002", 	// GPS Symbol
					d.vario_data.isTrackingEnabled() ? u8"\ue017" : "",			// Tracking Symbol
					d.vario_data.isWifiEnabled() ? u8"\ue011"     : "",	 		// W-Lan Symbol
					d.vario_data.getSpeaker()), 								// Speaker
			&xPos, &yPos, epd_hl_get_framebuffer(&hl), &font_props);
	font_props.flags = EPD_DRAW_ALIGN_RIGHT;
	xPos = rectHeader_.x + rectHeader_.width - 15;
	yPos = rectHeader_.y + rectHeader_.height/2
				+ small_.ascender/2 - small_.descender/2;

	epd_write_string(&small_,
			format(tmpBuf, "%02d:%02d:%02d %s",
					d.vario_data.getTime().hour(),   // Time
					d.vario_data.getTime().minute(),
					d.vario_data.getTime().second(),
					// Batary
					(d.vario_data.getBatVolt() <= 3.55F) ? u8"\ue004" ://   0%
					(d.vario_data.getBatVolt() <= 3.65F) ? u8"\ue005" ://  20%
					(d.vario_data.getBatVolt() <= 3.79F) ? u8"\ue006" ://  40%
					(d.vario_data.getBatVolt() <= 3.93F) ? u8"\ue007" ://  60%
					(d.vario_data.getBatVolt() <= 4.05F) ? u8"\ue008" ://  80%
					(d.vario_data.getBatVolt() <= 4.30F) ? u8"\ue009" :// 100%
					u8"\ue00a"), 								// Speaker
			&xPos, &yPos, epd_hl_get_framebuffer(&hl), &font_props);

	const EpdRect &r = rectControl_;
	epd_fill_rect(r, 0xFF, epd_hl_get_framebuffer(&hl));
	if (d.isMenuActiv_) {
		xPos = rectControl_.x + rectControl_.width - 15;
		yPos = rectControl_.y + rectControl_.height/2
						+ large_.ascender/2;
		// Paint Menu Buttons
		font_props.flags = EPD_DRAW_ALIGN_RIGHT;
		epd_rewrite_string(&large_, u8"\ue000",
				textRects[BUTTON_SHUTDOWN], xPos,  yPos, font_props);

		font_props.flags = EPD_DRAW_ALIGN_LEFT;
		xPos = rectControl_.x + 15;
		yPos = rectControl_.y + rectControl_.height/2
						+ large_.ascender/2;
		epd_rewrite_string(&large_, u8"\ue018",
				textRects[BUTTON_MENU], xPos, yPos, font_props);
		xPos += textRects[BUTTON_MENU].width +15;
		epd_rewrite_string(&large_, u8"\ue015",
				textRects[BUTTON_USE_GPS_ALT], xPos, yPos, font_props);
		xPos += textRects[BUTTON_USE_GPS_ALT].width +15;
		epd_rewrite_string(&large_, u8"\ue016",
				textRects[BUTTON_RESET_A2], xPos, yPos, font_props);
		xPos += textRects[BUTTON_RESET_A2].width +15;
		epd_rewrite_string(&large_, u8"\ue017",
				textRects[BUTTON_TRACKING], xPos, yPos, font_props);

	} else {
		// Paint Message
		// message
		xPos = rectControl_.x + rectControl_.width/2;
		yPos = rectControl_.y + rectControl_.height/2
						+ small_.ascender/2 - small_.descender/2;
		font_props.flags = EPD_DRAW_ALIGN_CENTER;
		epd_write_string(&small_, d.message_.c_str(), &xPos, &yPos,
				epd_hl_get_framebuffer(&hl), &font_props);
	}


}

void MainScrean::calcEpdRects() {
	switch(epd_get_rotation()) {
	case EPD_ROT_PORTRAIT:
    case EPD_ROT_INVERTED_PORTRAIT:
	{
		int yStart = 90;
		int yDelta = (epd_rotated_display_height() - 2 * yStart)/6;
		int xDelta = epd_rotated_display_width() /3;
		rectHeader_.x = 0;
		rectHeader_.y = 0;
		rectHeader_.width  = epd_rotated_display_width();
		rectHeader_.height = yStart;

		rectControl_.x = 0;
		rectControl_.y = epd_rotated_display_height()-yStart;
		rectControl_.width  = epd_rotated_display_width();
		rectControl_.height = yStart;

		rectVario_.x = 0;
		rectVario_.y = yStart;
		rectVario_.width = xDelta * 2;
		rectVario_.height = yDelta * 3;

		rectSpeed_.x = xDelta*2;
		rectSpeed_.y = yStart + yDelta*3;
		rectSpeed_.width = xDelta;
		rectSpeed_.height = yDelta*3;

		for(int i = 0; i < 9; i++) {
			rectTabs_[i].x = (i < 3) ? xDelta * 2 :
					        (i < 6) ? 0 : xDelta;
			rectTabs_[i].y = (i < 6) ? yStart + yDelta * i :
									  yStart + yDelta * (i-3);
			rectTabs_[i].width = xDelta;
			rectTabs_[i].height = yDelta;
		}
	}
	break;
    default:
	{
		int yStart = 90;
		int yDelta = (epd_rotated_display_height() - yStart)/3;
		int xDelta = epd_rotated_display_width() /6;
		rectHeader_.x = 0;
		rectHeader_.y = 0;
		rectHeader_.width  = epd_rotated_display_width()/2;
		rectHeader_.height = yStart;

		rectControl_.x = epd_rotated_display_width()/2;
		rectControl_.y = 0;
		rectControl_.width  = epd_rotated_display_width()/2;
		rectControl_.height = yStart;

		rectVario_.x = 0;
		rectVario_.y = yStart;
		rectVario_.width = xDelta * 2;
		rectVario_.height = yDelta * 3;

		rectSpeed_.x = xDelta * 5;
		rectSpeed_.y = yStart;
		rectSpeed_.width = xDelta;
		rectSpeed_.height = yDelta*3;

		for(int i = 0; i < 9; i++) {
			rectTabs_[i].x = (i < 3) ? xDelta * 2 :
					        (i < 6) ? xDelta * 3 : xDelta *4;
			rectTabs_[i].y = yStart + yDelta * (i%3);
			rectTabs_[i].width = xDelta;
			rectTabs_[i].height = yDelta;
		}

	}
	break;
	}
	if(rectVario_.width > rectVario_.height) {
		rectVario_.x += (rectVario_.width - rectVario_.height)/2;
		rectVario_.width = rectVario_.height;
	} else if(rectVario_.width < rectVario_.height) {
		rectVario_.y -= (rectVario_.width - rectVario_.height)/2;
		rectVario_.height = rectVario_.width;
	}
	epd_draw_rect(rectHeader_, 0x00, epd_hl_get_framebuffer(&hl));
	epd_draw_rect(rectControl_, 0x00, epd_hl_get_framebuffer(&hl));
	epd_draw_rect(rectVario_, 0x00, epd_hl_get_framebuffer(&hl));
	epd_draw_rect(rectSpeed_, 0x00, epd_hl_get_framebuffer(&hl));
	for(int i=0; i < 9; i++)
		epd_draw_rect(rectTabs_[i], 0x00, epd_hl_get_framebuffer(&hl));

	vario_.paint(rectVario_);
	speed_.paint(rectSpeed_);
}


void MainScrean::updateScrean(const data_t &d) {

	char tmpBuf[32];
	//	EpdFontProperties font_props = epd_font_properties_default();
	paintHeader(d);

	// Vario
	paintTab(rectTabs_[0], u8"Int.\ue014",
			format(tmpBuf, std::isnan(d.vario_data.getVario()) ? "--" :
					(d.vario_data.getVario() <= -10.F || d.vario_data.getVario() >= 10.F) ?
					"%.0f" : "%.1f", std::abs(d.vario_data.getVario())), large_);
	// Altitude
	paintTab(rectTabs_[1], "Alt1 m", format(tmpBuf, "%.0f", d.vario_data.getAltitude()), large_);

	// Altitude GPS
	paintTab(rectTabs_[2], "Alt2 m", format(tmpBuf, "%.0f", d.vario_data.getAltitude2()), large_);

	// Speed
	paintTab(rectTabs_[3], u8"Sp.\ue013", d.vario_data.getSpeed().isValid() ?
			 format(tmpBuf, "%.0f", d.vario_data.getSpeed().kmph()) : "--", medium_);

	// QNH
	paintTab(rectTabs_[4], "QNH", format(tmpBuf, "%.0f", d.vario_data.getQnh()), medium_);

	// Fly Time
	long flyTime = (millis() - d.vario_data.getStartTime())/1000;
	paintTab(rectTabs_[5], "Fly. Time",
			 format(tmpBuf, "%d:%02d:%02d", flyTime / 3600, flyTime / 60 % 60, flyTime % 60 ), medium_ );

	// Altitude GPS
	paintTab(rectTabs_[6], "AltGPS m", format(tmpBuf, "%.0f", d.vario_data.getAltGps().meters()), medium_);

	// FL
	paintTab(rectTabs_[7], "FL", format(tmpBuf, "%d", d.vario_data.getAltFL()), medium_);

	vario_.update(d.vario_data);
	speed_.update(d.vario_data);

}
