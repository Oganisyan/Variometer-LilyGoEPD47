#include "LilyGo-EPD47.h"

#include "ButtonCtrl.h"


void defaultButtonCallBack(int pin) {
	
}

void ButtonCtrl::poolCallBack(void *parameter) {
	ButtonCtrl *pThis= static_cast<ButtonCtrl*>(parameter);
    for(;;) {
		for(int i = 0; i < 5; i++) {
			if(pThis->pins_[i] >= 0) {
				bool st = !digitalRead(pThis->pins_[i]);
				if(st ^ pThis->status_[i]) {
					pThis->status_[i] = st;
					if(st)
						pThis->buttonCallBack_(pThis->pins_[i]);
				}

			}
		}
		taskDelay(50);
    }

}

ButtonCtrl::ButtonCtrl(int pin1, int pin2, int pin3, int pin4, int pin5) : buttonCallBack_(defaultButtonCallBack)
{
	pins_[0] = pin1;
	pins_[1] = pin2;
	pins_[2] = pin3;
	pins_[3] = pin4;
	pins_[4] = pin5;
	status_[0] = false;
	status_[1] = false;
	status_[2] = false;
	status_[3] = false;
	status_[4] = false;
}

void ButtonCtrl::begin() 
{
	for(int i = 0; i < 5; i++) {
		if(pins_[i] >= 0)
			pinMode(pins_[i], INPUT_PULLUP);
	}
}

