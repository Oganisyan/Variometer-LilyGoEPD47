#include "MainScreen_CYD.h"
#include "FreenoveConfig.h"
#include <stdio.h>
#include <cmath>
#include <iomanip>
#include <sstream>

static const char *TAG = "MainScreen_CYD";

MainScreen_CYD::MainScreen_CYD(DisplayAdapter* display)
    : display_(display), mutex_(portMUX_INITIALIZER_UNLOCKED),
      varioWidget_(nullptr, nullptr, nullptr), speedWidget_(nullptr, nullptr, nullptr) {
}

MainScreen_CYD::~MainScreen_CYD() {
}

ScreenData_t MainScreen_CYD::getData(bool forPaint) {
    taskENTER_CRITICAL(&mutex_);
    ScreenData_t d = screenData_;
    if (forPaint) {
        screenData_.paintScreen = false;
        screenData_.updateScreen = false;
    }
    taskEXIT_CRITICAL(&mutex_);
    return d;
}

void MainScreen_CYD::setData(const ScreenData_t &data) {
    taskENTER_CRITICAL(&mutex_);
    screenData_ = data;
    taskEXIT_CRITICAL(&mutex_);
}

TaskHandle_t MainScreen_CYD::begin() {
    // Initialize layout
    updateLayout();
    display_->begin();
    display_->fillScreen(COLOR_BLACK);
    
    // Set initial screen state
    ScreenData_t d = getData();
    d.paintScreen = true;
    d.updateScreen = true;
    setData(d);
    
    TaskHandle_t xHandle = NULL;
    xTaskCreatePinnedToCore(
        MainScreen_CYD::paintTask,
        "MainScreen_CYD",
        8 * 1024,
        this,
        5,
        &xHandle,
        1  // Core 1
    );
    
    if (!xHandle) {
        printf("Error: Failed to create MainScreen task\n");
    } else {
        printf("MainScreen_CYD started\n");
    }
    
    return xHandle;
}

void MainScreen_CYD::paintTask(void *parameter) {
    MainScreen_CYD *screen = static_cast<MainScreen_CYD*>(parameter);
    
    for (;;) {
        screen->paint();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void MainScreen_CYD::updateLayout() {
    // Header area (status bar)
    rectHeader_.x = 0;
    rectHeader_.y = 0;
    rectHeader_.width = DISPLAY_WIDTH;
    rectHeader_.height = HEADER_HEIGHT;
    
    // Main vario area (left side, 2/3 width)
    rectVario_.x = 0;
    rectVario_.y = HEADER_HEIGHT;
    rectVario_.width = (DISPLAY_WIDTH * 2) / 3;
    rectVario_.height = MAIN_AREA_HEIGHT;
    
    // Data panel (right side, 1/3 width)
    rectData_.x = (DISPLAY_WIDTH * 2) / 3;
    rectData_.y = HEADER_HEIGHT;
    rectData_.width = DISPLAY_WIDTH / 3;
    rectData_.height = MAIN_AREA_HEIGHT;
    
    // Control bar (button area)
    rectControl_.x = 0;
    rectControl_.y = DISPLAY_HEIGHT - CONTROL_HEIGHT;
    rectControl_.width = DISPLAY_WIDTH;
    rectControl_.height = CONTROL_HEIGHT;
    
    // Button layout (6 buttons across)
    int btn_width = DISPLAY_WIDTH / 3;
    int btn_height = CONTROL_HEIGHT;
    
    for (int i = 0; i < 3; i++) {
        // Top row
        buttonRects_[i].x = i * btn_width;
        buttonRects_[i].y = DISPLAY_HEIGHT - CONTROL_HEIGHT;
        buttonRects_[i].width = btn_width;
        buttonRects_[i].height = btn_height;
        
        // Bottom row
        buttonRects_[i + 3].x = i * btn_width;
        buttonRects_[i + 3].y = DISPLAY_HEIGHT - CONTROL_HEIGHT / 2;
        buttonRects_[i + 3].width = btn_width;
        buttonRects_[i + 3].height = CONTROL_HEIGHT / 2;
    }
}

void MainScreen_CYD::paint() {
    ScreenData_t d = getData(true);
    
    if (d.paintScreen) {
        // Full repaint
        display_->fillScreen(COLOR_BLACK);
        updateLayout();
        paintVarioArea(d);
        paintDataPanel(d);
    }
    
    if (d.updateScreen || d.paintScreen) {
        paintHeader(d);
        paintControlBar(d);
    }
}

void MainScreen_CYD::paintHeader(const ScreenData_t &data) {
    // Draw header background
    display_->fillRect(rectHeader_.x, rectHeader_.y, rectHeader_.width, rectHeader_.height, COLOR_DARK_GRAY);
    
    // Left side: Status icons
    char statusStr[64];
    snprintf(statusStr, sizeof(statusStr), "%s %s %s",
             data.varioData.getLocation().isValid() ? "📍" : "🌐",
             data.varioData.isTrackingEnabled() ? "📍" : " ",
             data.varioData.isWifiEnabled() ? "📡" : " ");
    
    display_->drawString(statusStr, 5, 8, COLOR_WHITE);
    
    // Right side: Time and battery
    char timeStr[32];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
             data.varioData.getTime().hour(),
             data.varioData.getTime().minute(),
             data.varioData.getTime().second());
    
    display_->drawString(timeStr, DISPLAY_WIDTH - 50, 8, COLOR_WHITE);
}

void MainScreen_CYD::paintControlBar(const ScreenData_t &data) {
    // Draw control bar background
    display_->fillRect(rectControl_.x, rectControl_.y, rectControl_.width, rectControl_.height, COLOR_DARK_GRAY);
    
    if (data.isMenuActive) {
        // Draw menu buttons
        drawButton(buttonRects_[BUTTON_MENU].x, buttonRects_[BUTTON_MENU].y,
                  buttonRects_[BUTTON_MENU].width, buttonRects_[BUTTON_MENU].height,
                  "⚙", "Menu", COLOR_CYAN);
        drawButton(buttonRects_[BUTTON_USE_GPS_ALT].x, buttonRects_[BUTTON_USE_GPS_ALT].y,
                  buttonRects_[BUTTON_USE_GPS_ALT].width, buttonRects_[BUTTON_USE_GPS_ALT].height,
                  "📡", "GPS Alt", COLOR_GREEN);
        drawButton(buttonRects_[BUTTON_RESET_A2].x, buttonRects_[BUTTON_RESET_A2].y,
                  buttonRects_[BUTTON_RESET_A2].width, buttonRects_[BUTTON_RESET_A2].height,
                  "🔄", "Reset", COLOR_YELLOW);
        drawButton(buttonRects_[BUTTON_TRACKING].x, buttonRects_[BUTTON_TRACKING].y,
                  buttonRects_[BUTTON_TRACKING].width, buttonRects_[BUTTON_TRACKING].height,
                  "📍", "Track", COLOR_MAGENTA);
        drawButton(buttonRects_[BUTTON_SHUTDOWN].x, buttonRects_[BUTTON_SHUTDOWN].y,
                  buttonRects_[BUTTON_SHUTDOWN].width, buttonRects_[BUTTON_SHUTDOWN].height,
                  "⏻", "Shut", COLOR_RED);
    } else {
        // Draw message
        display_->drawStringCentered(data.message.c_str(),
                                    DISPLAY_WIDTH / 2,
                                    DISPLAY_HEIGHT - CONTROL_HEIGHT + 8,
                                    COLOR_YELLOW);
    }
}

void MainScreen_CYD::paintVarioArea(const ScreenData_t &data) {
    // Draw vario gauge background
    display_->fillRect(rectVario_.x, rectVario_.y, rectVario_.width, rectVario_.height, COLOR_BLACK);
    display_->drawRect(rectVario_.x, rectVario_.y, rectVario_.width, rectVario_.height, COLOR_WHITE);
    
    // Draw large vario value
    char varioStr[16];
    if (std::isnan(data.varioData.getVario())) {
        snprintf(varioStr, sizeof(varioStr), "--");
    } else {
        snprintf(varioStr, sizeof(varioStr), "%.1f", data.varioData.getVario());
    }
    
    uint16_t varioColor = (data.varioData.getVario() > 0) ? COLOR_GREEN :
                         (data.varioData.getVario() < 0) ? COLOR_RED : COLOR_WHITE;
    
    display_->drawStringCentered(varioStr,
                                rectVario_.x + rectVario_.width / 2,
                                rectVario_.y + rectVario_.height / 2,
                                varioColor);
}

void MainScreen_CYD::paintDataPanel(const ScreenData_t &data) {
    // Draw data panel background
    display_->fillRect(rectData_.x, rectData_.y, rectData_.width, rectData_.height, COLOR_BLACK);
    display_->drawRect(rectData_.x, rectData_.y, rectData_.width, rectData_.height, COLOR_WHITE);
    
    int x = rectData_.x + 5;
    int y = rectData_.y + 5;
    int cell_height = 25;
    
    // Altitude
    char altStr[32];
    snprintf(altStr, sizeof(altStr), "%.0f m", data.varioData.getAltitude());
    drawDataCell(x, y, rectData_.width - 10, cell_height, "Alt1", altStr, COLOR_CYAN);
    y += cell_height;
    
    // Altitude 2
    snprintf(altStr, sizeof(altStr), "%.0f m", data.varioData.getAltitude2());
    drawDataCell(x, y, rectData_.width - 10, cell_height, "Alt2", altStr, COLOR_CYAN);
    y += cell_height;
    
    // Speed
    char speedStr[32];
    if (data.varioData.getSpeed().isValid()) {
        snprintf(speedStr, sizeof(speedStr), "%.0f", data.varioData.getSpeed().kmph());
    } else {
        snprintf(speedStr, sizeof(speedStr), "--");
    }
    drawDataCell(x, y, rectData_.width - 10, cell_height, "Spd", speedStr, COLOR_GREEN);
    y += cell_height;
    
    // QNH
    char qnhStr[32];
    snprintf(qnhStr, sizeof(qnhStr), "%.0f", data.varioData.getQnh());
    drawDataCell(x, y, rectData_.width - 10, cell_height, "QNH", qnhStr, COLOR_WHITE);
}

void MainScreen_CYD::drawButton(int x, int y, int w, int h, const char* icon, const char* label, uint16_t color) {
    display_->drawRect(x, y, w, h, color);
    display_->drawString(icon, x + 5, y + 5, color);
    display_->drawString(label, x + 5, y + 15, color);
}

void MainScreen_CYD::drawDataCell(int x, int y, int w, int h, const char* label, const char* value, uint16_t color) {
    display_->drawRect(x, y, w, h, color);
    display_->drawString(label, x + 3, y + 2, COLOR_LIGHT_GRAY);
    display_->drawString(value, x + 3, y + 12, color);
}

void MainScreen_CYD::setVarioData(const LiLyGoEPD47 &varioData) {
    ScreenData_t d = getData();
    d.varioData = varioData;
    d.updateScreen = true;
    setData(d);
}

void MainScreen_CYD::setMessage(const std::string &message, bool messageToFront) {
    ScreenData_t d = getData();
    d.message = message;
    if (messageToFront) {
        d.isMenuActive = false;
    }
    d.updateScreen = true;
    setData(d);
}

bool MainScreen_CYD::switchMenu() {
    ScreenData_t d = getData();
    d.isMenuActive = !d.isMenuActive;
    d.updateScreen = true;
    setData(d);
    return d.isMenuActive;
}

void MainScreen_CYD::repaint() {
    ScreenData_t d = getData();
    d.paintScreen = true;
    d.updateScreen = true;
    setData(d);
}

bool MainScreen_CYD::isInRect(int x, int y, int rectId) {
    if (rectId >= 6) return false;
    
    const DisplayRect_t &r = buttonRects_[rectId];
    return x >= r.x && y >= r.y && x <= (r.x + r.width) && y <= (r.y + r.height);
}
