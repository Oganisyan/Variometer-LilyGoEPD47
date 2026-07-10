#include "TouchSensor_FT6236.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "TouchSensor_FT6236";

// FT6236 Register Addresses
#define FT6236_REG_DEV_MODE     0x00
#define FT6236_REG_GEST_ID      0x01
#define FT6236_REG_TD_STATUS    0x02
#define FT6236_REG_TOUCH1_XH    0x03
#define FT6236_REG_TOUCH1_XL    0x04
#define FT6236_REG_TOUCH1_YH    0x05
#define FT6236_REG_TOUCH1_YL    0x06
#define FT6236_REG_TOUCH1_WEIGHT 0x07
#define FT6236_REG_TOUCH1_MISC  0x08

// Touch point data register offset
#define FT6236_TOUCH_STEP       6

TouchSensor_FT6236::TouchSensor_FT6236(int16_t pin_int, int16_t pin_sda, int16_t pin_scl, uint8_t i2c_addr)
    : i2c_address_(i2c_addr), pin_int_(pin_int), pin_sda_(pin_sda), pin_scl_(pin_scl),
      i2c_port_(nullptr), enabled_(false), callback_(nullptr),
      cal_min_x_(0), cal_max_x_(320), cal_min_y_(0), cal_max_y_(240) {
}

TouchSensor_FT6236::~TouchSensor_FT6236() {
}

bool TouchSensor_FT6236::begin(TwoWire &wire) {
    i2c_port_ = &wire;
    
    // Initialize I2C
    i2c_port_->begin(pin_sda_, pin_scl_, 400000);
    
    // Check if device is present
    i2c_port_->beginTransmission(i2c_address_);
    if (i2c_port_->endTransmission() != 0) {
        ESP_LOGE(TAG, "FT6236 not found at address 0x%02x", i2c_address_);
        return false;
    }
    
    // Reset device
    pinMode(pin_int_, INPUT);
    
    // Set to normal working mode (0x00)
    writeRegister(FT6236_REG_DEV_MODE, 0x00);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    
    enabled_ = true;
    wakeup();
    
    ESP_LOGI(TAG, "FT6236 Touch sensor initialized");
    return true;
}

void TouchSensor_FT6236::end() {
    sleep();
}

uint8_t TouchSensor_FT6236::readRegister(uint8_t reg) {
    if (!i2c_port_) return 0;
    
    i2c_port_->beginTransmission(i2c_address_);
    i2c_port_->write(reg);
    i2c_port_->endTransmission();
    
    i2c_port_->requestFrom(i2c_address_, (uint8_t)1);
    if (i2c_port_->available()) {
        return i2c_port_->read();
    }
    return 0;
}

void TouchSensor_FT6236::readRegisters(uint8_t reg, uint8_t* data, uint8_t len) {
    if (!i2c_port_) return;
    
    i2c_port_->beginTransmission(i2c_address_);
    i2c_port_->write(reg);
    i2c_port_->endTransmission();
    
    i2c_port_->requestFrom(i2c_address_, len);
    uint8_t i = 0;
    while (i2c_port_->available() && i < len) {
        data[i++] = i2c_port_->read();
    }
}

void TouchSensor_FT6236::writeRegister(uint8_t reg, uint8_t value) {
    if (!i2c_port_) return;
    
    i2c_port_->beginTransmission(i2c_address_);
    i2c_port_->write(reg);
    i2c_port_->write(value);
    i2c_port_->endTransmission();
}

uint8_t TouchSensor_FT6236::scanTouchPoints() {
    if (!enabled_) {
        touch_points_.clear();
        return 0;
    }
    
    // Read touch status
    uint8_t td_status = readRegister(FT6236_REG_TD_STATUS);
    uint8_t num_touches = td_status & 0x0F;
    
    if (num_touches == 0) {
        touch_points_.clear();
        return 0;
    }
    
    if (num_touches > 5) {
        num_touches = 5;  // Max 5 touch points
    }
    
    touch_points_.clear();
    
    // Read each touch point
    for (uint8_t i = 0; i < num_touches; i++) {
        uint8_t base_reg = FT6236_REG_TOUCH1_XH + (i * FT6236_TOUCH_STEP);
        uint8_t data[6];
        readRegisters(base_reg, data, 6);
        
        TouchPoint_t tp;
        tp.id = (data[2] >> 4) & 0x0F;
        tp.state = data[0] >> 6;  // Event flag
        
        // Parse coordinates (12-bit)
        tp.x = ((data[0] & 0x0F) << 8) | data[1];
        tp.y = ((data[2] & 0x0F) << 8) | data[3];
        
        // Apply calibration
        tp.x = mapTouchX(tp.x);
        tp.y = mapTouchY(tp.y);
        
        tp.prev_x = tp.x;
        tp.prev_y = tp.y;
        
        touch_points_.push_back(tp);
    }
    
    return num_touches;
}

void TouchSensor_FT6236::sleep() {
    if (!i2c_port_) return;
    
    writeRegister(FT6236_REG_DEV_MODE, 0x01);  // Sleep mode
    enabled_ = false;
    
    ESP_LOGI(TAG, "Touch sensor sleep");
}

void TouchSensor_FT6236::wakeup() {
    if (!i2c_port_) return;
    
    writeRegister(FT6236_REG_DEV_MODE, 0x00);  // Normal mode
    enabled_ = true;
    
    ESP_LOGI(TAG, "Touch sensor wakeup");
}

void TouchSensor_FT6236::setCalibration(int16_t min_x, int16_t max_x, int16_t min_y, int16_t max_y) {
    cal_min_x_ = min_x;
    cal_max_x_ = max_x;
    cal_min_y_ = min_y;
    cal_max_y_ = max_y;
    
    ESP_LOGI(TAG, "Touch calibration set: X[%d,%d] Y[%d,%d]", min_x, max_x, min_y, max_y);
}

void TouchSensor_FT6236::resetCalibration() {
    cal_min_x_ = 0;
    cal_max_x_ = 320;
    cal_min_y_ = 0;
    cal_max_y_ = 240;
}

uint16_t TouchSensor_FT6236::mapTouchX(uint16_t raw_x) {
    // Map raw touch coordinates to screen coordinates
    if (cal_max_x_ <= cal_min_x_) return raw_x;
    return (raw_x - cal_min_x_) * 320 / (cal_max_x_ - cal_min_x_);
}

uint16_t TouchSensor_FT6236::mapTouchY(uint16_t raw_y) {
    // Map raw touch coordinates to screen coordinates
    if (cal_max_y_ <= cal_min_y_) return raw_y;
    return (raw_y - cal_min_y_) * 240 / (cal_max_y_ - cal_min_y_);
}

void TouchSensor_FT6236::pollTask(void *parameter) {
    TouchSensor_FT6236 *sensor = static_cast<TouchSensor_FT6236*>(parameter);
    
    for (;;) {
        if (sensor->isEnabled() && digitalRead(sensor->pin_int_) == LOW) {
            uint8_t num_touches = sensor->scanTouchPoints();
            
            if (num_touches > 0 && sensor->callback_) {
                sensor->callback_(sensor->getTouchPoints());
            }
        }
        
        vTaskDelay(50 / portTICK_PERIOD_MS);  // 50ms poll interval
    }
}
