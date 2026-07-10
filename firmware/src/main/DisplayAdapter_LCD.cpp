#include "DisplayAdapter_LCD.h"
#include <math.h>
#include <esp_log.h>

static const char *TAG = "DisplayAdapter_LCD";

DisplayAdapter_LCD::DisplayAdapter_LCD() 
    : width_(DISPLAY_WIDTH), height_(DISPLAY_HEIGHT), rotation_(DISPLAY_ROTATION) {
}

DisplayAdapter_LCD::~DisplayAdapter_LCD() {
}

void DisplayAdapter_LCD::begin() {
    tft.init();
    tft.setRotation(rotation_);
    tft.fillScreen(TFT_BLACK);
    setBacklight(255);
    ESP_LOGI(TAG, "LCD Display initialized: %dx%d", width_, height_);
}

void DisplayAdapter_LCD::end() {
    sleep();
}

void DisplayAdapter_LCD::fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) {
    tft.fillRect(x, y, w, h, color);
}

void DisplayAdapter_LCD::drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) {
    tft.drawRect(x, y, w, h, color);
}

void DisplayAdapter_LCD::fillScreen(uint16_t color) {
    tft.fillScreen(color);
}

void DisplayAdapter_LCD::drawPixel(int16_t x, int16_t y, uint16_t color) {
    tft.drawPixel(x, y, color);
}

void DisplayAdapter_LCD::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    tft.drawLine(x0, y0, x1, y1, color);
}

void DisplayAdapter_LCD::drawCircle(int16_t x, int16_t y, uint16_t r, uint16_t color) {
    tft.drawCircle(x, y, r, color);
}

void DisplayAdapter_LCD::fillCircle(int16_t x, int16_t y, uint16_t r, uint16_t color) {
    tft.fillCircle(x, y, r, color);
}

void DisplayAdapter_LCD::drawTriangle(const DisplayTriangle_t &tri, uint16_t color) {
    tft.drawTriangle(tri.x0, tri.y0, tri.x1, tri.y1, tri.x2, tri.y2, color);
}

void DisplayAdapter_LCD::fillTriangle(const DisplayTriangle_t &tri, uint16_t color) {
    tft.fillTriangle(tri.x0, tri.y0, tri.x1, tri.y1, tri.x2, tri.y2, color);
}

void DisplayAdapter_LCD::drawArc(int16_t x, int16_t y, uint16_t radius, uint16_t thickness,
                                 uint16_t start_angle, uint16_t sweep_angle, uint16_t color) {
    // Simplified arc drawing using lines
    float angle_step = 1.0f;  // degrees
    float end_angle = start_angle + sweep_angle;
    
    int16_t prev_x = x + radius * cos((start_angle - 90) * M_PI / 180.0f);
    int16_t prev_y = y + radius * sin((start_angle - 90) * M_PI / 180.0f);
    
    for (float angle = start_angle; angle <= end_angle; angle += angle_step) {
        float rad = (angle - 90) * M_PI / 180.0f;
        int16_t curr_x = x + radius * cos(rad);
        int16_t curr_y = y + radius * sin(rad);
        tft.drawLine(prev_x, prev_y, curr_x, curr_y, color);
        prev_x = curr_x;
        prev_y = curr_y;
    }
}

void DisplayAdapter_LCD::fillArc(int16_t x, int16_t y, uint16_t radius, uint16_t thickness,
                                 uint16_t start_angle, uint16_t sweep_angle, uint16_t color) {
    // Draw filled arc by drawing multiple arcs with decreasing radius
    float angle_step = 1.0f;
    float end_angle = start_angle + sweep_angle;
    
    for (uint16_t r = radius - thickness; r <= radius; r++) {
        int16_t prev_x = x + r * cos((start_angle - 90) * M_PI / 180.0f);
        int16_t prev_y = y + r * sin((start_angle - 90) * M_PI / 180.0f);
        
        for (float angle = start_angle; angle <= end_angle; angle += angle_step) {
            float rad = (angle - 90) * M_PI / 180.0f;
            int16_t curr_x = x + r * cos(rad);
            int16_t curr_y = y + r * sin(rad);
            tft.drawLine(prev_x, prev_y, curr_x, curr_y, color);
            prev_x = curr_x;
            prev_y = curr_y;
        }
    }
}

void DisplayAdapter_LCD::drawString(const char* text, int16_t x, int16_t y, uint16_t color) {
    tft.setTextColor(color);
    tft.setCursor(x, y);
    tft.print(text);
}

void DisplayAdapter_LCD::drawStringCentered(const char* text, int16_t x, int16_t y, uint16_t color) {
    tft.setTextColor(color);
    tft.setCursor(x - (strlen(text) * 6) / 2, y);  // Approximate centering
    tft.print(text);
}

void DisplayAdapter_LCD::setRotation(uint8_t rotation) {
    rotation_ = rotation % 4;
    tft.setRotation(rotation_);
}

void DisplayAdapter_LCD::setBacklight(uint8_t brightness) {
    // PWM control of backlight (0-255)
    uint16_t pwm_value = (brightness * 1023) / 255;
    ledcWrite(0, pwm_value);
}

void DisplayAdapter_LCD::sleep() {
    tft.writecommand(0x10);  // ILI9341 sleep mode
    setBacklight(0);
}

void DisplayAdapter_LCD::wakeup() {
    tft.writecommand(0x11);  // ILI9341 wake
    setBacklight(255);
}

void DisplayAdapter_LCD::updateScreen() {
    // LCD updates immediately, no need to explicitly update
}

void DisplayAdapter_LCD::fullClear() {
    tft.fillScreen(TFT_BLACK);
}

uint16_t DisplayAdapter_LCD::rgbTo565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

// Factory function
DisplayAdapter* createDisplayAdapter() {
    return new DisplayAdapter_LCD();
}
