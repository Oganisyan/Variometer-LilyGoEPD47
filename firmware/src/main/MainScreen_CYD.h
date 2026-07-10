#pragma once

#include "LilyGo-EPD47.h"
#include "DisplayAdapter.h"
#include "AnalogVarioWidget.h"
#include "SpeedometerWidget.h"
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @class MainScreen_CYD
 * @brief Main UI screen for FREENOVE CYD (320x240 IPS-LCD)
 *        Adapted layout from EPD version with color support and smaller resolution
 */

// Button area identifiers
#define BUTTON_VARIO        0
#define BUTTON_MENU         1
#define BUTTON_USE_GPS_ALT  2
#define BUTTON_RESET_A2     3
#define BUTTON_TRACKING     4
#define BUTTON_SHUTDOWN     5

// Layout configuration for 320x240
#define HEADER_HEIGHT   28   // Status bar
#define CONTROL_HEIGHT  32   // Button bar
#define MAIN_AREA_HEIGHT (DISPLAY_HEIGHT - HEADER_HEIGHT - CONTROL_HEIGHT)  // 180

typedef struct {
    bool paintScreen = false;
    bool updateScreen = false;
    bool isMenuActive = false;
    LiLyGoEPD47 varioData;
    std::string message = u8"System started";
} ScreenData_t;

class MainScreen_CYD {
private:
    DisplayAdapter* display_;
    portMUX_TYPE mutex_;
    
    // Screen state
    ScreenData_t screenData_;
    ScreenData_t screenDataBuffer_;
    
    // Layout rectangles
    DisplayRect_t rectHeader_;
    DisplayRect_t rectControl_;
    DisplayRect_t rectVario_;
    DisplayRect_t rectData_;
    DisplayRect_t buttonRects_[6];
    
    // Widgets
    AnalogVarioWidget varioWidget_;
    SpeedometerWidget speedWidget_;
    
    // Paint methods
    static void paintTask(void *parameter);
    void paint();
    void paintHeader(const ScreenData_t &data);
    void paintControlBar(const ScreenData_t &data);
    void paintDataPanel(const ScreenData_t &data);
    void paintVarioArea(const ScreenData_t &data);
    void updateLayout();
    
    // Helper methods
    void drawButton(int x, int y, int w, int h, const char* icon, const char* label, uint16_t color);
    void drawDataCell(int x, int y, int w, int h, const char* label, const char* value, uint16_t color);
    
public:
    MainScreen_CYD(DisplayAdapter* display);
    virtual ~MainScreen_CYD();
    
    TaskHandle_t begin();
    
    void setVarioData(const LiLyGoEPD47 &varioData);
    void setMessage(const std::string &message, bool messageToFront = false);
    bool switchMenu();
    void repaint();
    
    bool isInRect(int x, int y, int rectId);
    const DisplayRect_t& getButtonRect(int rectId) const { return buttonRects_[rectId]; }
    
private:
    ScreenData_t getData(bool forPaint = false);
    void setData(const ScreenData_t &data);
};
