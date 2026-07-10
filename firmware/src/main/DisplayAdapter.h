#pragma once

#include <Arduino.h>
#include <vector>
#include <utility>

// Display type detection
#define DISPLAY_TYPE_LCD    1
#define DISPLAY_TYPE_EPD    2
#define CURRENT_DISPLAY     DISPLAY_TYPE_LCD

// Unified rect structure
typedef struct {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} DisplayRect_t;

// Triangle structure for arc rendering
typedef struct {
    int16_t x0, y0;
    int16_t x1, y1;
    int16_t x2, y2;
} DisplayTriangle_t;

/**
 * @class DisplayAdapter
 * @brief Abstraction layer for different display types (EPD vs LCD)
 *        Provides unified rendering API independent of display hardware
 */
class DisplayAdapter {
public:
    virtual ~DisplayAdapter() {}

    // Initialization
    virtual void begin() = 0;
    virtual void end() = 0;

    // Basic drawing
    virtual void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) = 0;
    virtual void fillScreen(uint16_t color) = 0;
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

    // Shapes
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) = 0;
    virtual void drawCircle(int16_t x, int16_t y, uint16_t r, uint16_t color) = 0;
    virtual void fillCircle(int16_t x, int16_t y, uint16_t r, uint16_t color) = 0;
    virtual void drawTriangle(const DisplayTriangle_t &tri, uint16_t color) = 0;
    virtual void fillTriangle(const DisplayTriangle_t &tri, uint16_t color) = 0;

    // Arc/circle operations
    virtual void drawArc(int16_t x, int16_t y, uint16_t radius, uint16_t thickness,
                        uint16_t start_angle, uint16_t sweep_angle, uint16_t color) = 0;
    virtual void fillArc(int16_t x, int16_t y, uint16_t radius, uint16_t thickness,
                        uint16_t start_angle, uint16_t sweep_angle, uint16_t color) = 0;

    // Text rendering
    virtual void drawString(const char* text, int16_t x, int16_t y, uint16_t color) = 0;
    virtual void drawStringCentered(const char* text, int16_t x, int16_t y, uint16_t color) = 0;

    // Screen control
    virtual void setRotation(uint8_t rotation) = 0;
    virtual uint8_t getRotation() = 0;
    virtual void setBacklight(uint8_t brightness) = 0;
    virtual void sleep() = 0;
    virtual void wakeup() = 0;
    virtual void updateScreen() = 0;
    virtual void fullClear() = 0;

    // Query
    virtual uint16_t getWidth() = 0;
    virtual uint16_t getHeight() = 0;

    // Color conversion
    virtual uint16_t rgbTo565(uint8_t r, uint8_t g, uint8_t b) = 0;
};

// Factory function
DisplayAdapter* createDisplayAdapter();
