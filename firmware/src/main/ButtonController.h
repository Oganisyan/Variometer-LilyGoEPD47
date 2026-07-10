#pragma once

#include <Arduino.h>
#include <vector>
#include "DisplayAdapter.h"
#include "TouchSensor_FT6236.h"

/**
 * @brief Button event callback
 */
typedef void (*ButtonCallback_t)(uint8_t button_id);

/**
 * @class ButtonController
 * @brief Manages hardware buttons for FREENOVE CYD
 */
class ButtonController {
private:
    int16_t btn_up_;
    int16_t btn_center_;
    int16_t btn_down_;
    
    uint32_t last_press_time_[3];
    bool btn_pressed_[3];
    
    ButtonCallback_t callback_;
    
    static void pollTask(void *parameter);
    void checkButton(uint8_t btn_id);
    
public:
    ButtonController(int16_t btn_up, int16_t btn_center, int16_t btn_down);
    virtual ~ButtonController();
    
    void begin();
    void end();
    void setCallback(ButtonCallback_t callback) { callback_ = callback; }
    
    // Button IDs
    static constexpr uint8_t BTN_UP = 0;
    static constexpr uint8_t BTN_CENTER = 1;
    static constexpr uint8_t BTN_DOWN = 2;
};
