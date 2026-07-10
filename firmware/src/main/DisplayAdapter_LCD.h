#pragma once

#include "DisplayAdapter.h"
#include "FreenoveConfig.h"
#include <TFT_eSPI.h>

/**
 * @class DisplayAdapter_LCD
 * @brief Implementation of DisplayAdapter for LCD displays (ILI9341, ST7789)
 *        Wraps TFT_eSPI library for FREENOVE CYD compatibility
 */
class DisplayAdapter_LCD : public DisplayAdapter {
private:
    TFT_eSPI tft;
    uint16_t width_;
    uint16_t height_;
    uint8_t rotation_;

public:
    DisplayAdapter_LCD();
    virtual ~DisplayAdapter_LCD();

    void begin() override;
    void end() override;

    void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) override;
    void fillScreen(uint16_t color) override;
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void drawCircle(int16_t x, int16_t y, uint16_t r, uint16_t color) override;
    void fillCircle(int16_t x, int16_t y, uint16_t r, uint16_t color) override;
    void drawTriangle(const DisplayTriangle_t &tri, uint16_t color) override;
    void fillTriangle(const DisplayTriangle_t &tri, uint16_t color) override;

    void drawArc(int16_t x, int16_t y, uint16_t radius, uint16_t thickness,
                uint16_t start_angle, uint16_t sweep_angle, uint16_t color) override;
    void fillArc(int16_t x, int16_t y, uint16_t radius, uint16_t thickness,
                uint16_t start_angle, uint16_t sweep_angle, uint16_t color) override;

    void drawString(const char* text, int16_t x, int16_t y, uint16_t color) override;
    void drawStringCentered(const char* text, int16_t x, int16_t y, uint16_t color) override;

    void setRotation(uint8_t rotation) override;
    uint8_t getRotation() override { return rotation_; }
    void setBacklight(uint8_t brightness) override;
    void sleep() override;
    void wakeup() override;
    void updateScreen() override;
    void fullClear() override;

    uint16_t getWidth() override { return width_; }
    uint16_t getHeight() override { return height_; }

    uint16_t rgbTo565(uint8_t r, uint8_t g, uint8_t b) override;

    // Helper methods
    TFT_eSPI& getTFT() { return tft; }
};
