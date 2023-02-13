#include "Power.h"
#include <esp_adc_cal.h>

Power::Power(uint8_t pin) :  pin_(pin), vref_(1100)
{}

void Power::setup() {
	// Correct the ADC reference voltage
	esp_adc_cal_characteristics_t adc_chars;
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
	if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
		vref_ = adc_chars.vref;
	}
}

float Power::get() {
	double reading = analogRead(pin_);
	for (int i = 0; i < 9; i++) {
	    reading = analogRead(pin_) * 0.1 + reading * 0.9;
	}
	if (reading < 1 || reading > 4095) return NaN;
	return static_cast<float>(reading * vref_ * 1.6117216e-6); // reading / 4095 * 2 * 3.3 * vref_ / 1000
}

