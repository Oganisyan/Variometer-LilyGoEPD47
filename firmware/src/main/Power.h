#ifndef __POWER_H__
#define __POWER_H__
#include "LilyGo-EPD47.h"

class Power {
	uint8_t pin_;
	int vref_;
public:
	Power(uint8_t pin);
	void setup();
	float get();
};


#endif //__POWER_H__
