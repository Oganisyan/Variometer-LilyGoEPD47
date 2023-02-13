#ifndef _BUTTON_CTRL_H_
#define _BUTTON_CTRL_H_
#pragma once
#include <Arduino.h>

typedef void  (*buttonCallBack_t) (int pin);

class ButtonCtrl {
	buttonCallBack_t buttonCallBack_;
	int   pins_[5];
	bool  status_[5];
public:
    static void poolCallBack(void *parameter) ;
	ButtonCtrl(int pin1, int pin2 = -1, int pin3 = -1, int pin4 = -1, int pin5 = -1);
    void begin();
	void setButtonCallBack(buttonCallBack_t buttonCallBack)
	{ buttonCallBack_ = buttonCallBack; }
};


#endif
