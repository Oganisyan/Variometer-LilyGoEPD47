#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <vector>

/**
 * @brief Touch point data structure
 */
typedef struct {
    uint16_t x;      // X coordinate
    uint16_t y;      // Y coordinate
    uint16_t prev_x; // Previous X coordinate
    uint16_t prev_y; // Previous Y coordinate
    uint8_t  id;     // Touch point ID
    uint8_t  state;  // Touch state (0=pressed, 1=moving, 2=released)
} TouchPoint_t;

/**
 * @brief Callback function types
 */
typedef void (*TouchCallback_t)(const std::vector<TouchPoint_t>&);

/**
 * @class TouchSensor_FT6236
 * @brief FT6236 Capacitive Touch Sensor Driver
 *        Compatible with FREENOVE ESP32 CYD
 */
class TouchSensor_FT6236 {
private:
    uint8_t i2c_address_;
    int16_t pin_int_;
    int16_t pin_sda_;
    int16_t pin_scl_;
    TwoWire* i2c_port_;
    bool enabled_;
    
    TouchCallback_t callback_;
    std::vector<TouchPoint_t> touch_points_;
    
    // Calibration data
    int16_t cal_min_x_, cal_max_x_;
    int16_t cal_min_y_, cal_max_y_;
    
    // Private methods
    uint8_t readRegister(uint8_t reg);
    void readRegisters(uint8_t reg, uint8_t* data, uint8_t len);
    void writeRegister(uint8_t reg, uint8_t value);
    
    void calibrateTouch();
    uint16_t mapTouchX(uint16_t raw_x);
    uint16_t mapTouchY(uint16_t raw_y);
    
public:
    TouchSensor_FT6236(int16_t pin_int, int16_t pin_sda, int16_t pin_scl, 
                       uint8_t i2c_addr = 0x38);
    virtual ~TouchSensor_FT6236();
    
    // Initialization
    bool begin(TwoWire &wire = Wire);
    void end();
    
    // Control
    void sleep();
    void wakeup();
    bool isEnabled() const { return enabled_; }
    
    // Touch reading
    uint8_t scanTouchPoints();
    const std::vector<TouchPoint_t>& getTouchPoints() const { return touch_points_; }
    
    // Callbacks
    void setCallback(TouchCallback_t callback) { callback_ = callback; }
    
    // Calibration
    void setCalibration(int16_t min_x, int16_t max_x, int16_t min_y, int16_t max_y);
    void resetCalibration();
    
    // Static poll task for FreeRTOS
    static void pollTask(void *parameter);
};
