#include "LilyGo-EPD47-CYD.h"
#include "FreenoveConfig.h"
#include <FFat.h>
#include <esp_log.h>

static const char *TAG = "VariometerCYD";

// Global instances
DisplayAdapter_LCD* g_display = nullptr;
TouchSensor_FT6236* g_touch = nullptr;
ButtonController* g_buttons = nullptr;
MainScreen_CYD* g_screen = nullptr;
BlueFlyVario* g_vario = nullptr;
WifiWebServer* g_wifiServer = nullptr;

// Task handles
TaskHandle_t taskVario = nullptr;
TaskHandle_t taskWifi = nullptr;

/**
 * @brief Touch callback handler
 */
void touchCallback(const std::vector<TouchPoint_t>& points) {
    if (points.empty()) return;
    
    const TouchPoint_t& p = points[0];  // Use first touch point
    
    // Check button presses
    if (g_screen->isInRect(p.x, p.y, BUTTON_MENU)) {
        g_screen->switchMenu();
        g_vario->switchWiFi();
    } else if (g_screen->isInRect(p.x, p.y, BUTTON_USE_GPS_ALT)) {
        float altGPS = static_cast<float>(g_vario->getVarioData().getAltGps().meters());
        g_vario->calcQNH(altGPS);
        g_screen->setMessage("QNH from GPS!", true);
    } else if (g_screen->isInRect(p.x, p.y, BUTTON_RESET_A2)) {
        g_vario->setStartAlt(g_vario->getVarioData().getAltitude());
        g_screen->setMessage("Alt2 reset!", true);
    } else if (g_screen->isInRect(p.x, p.y, BUTTON_TRACKING)) {
        g_vario->switchTracking();
        g_screen->setMessage("Track toggle", true);
    } else if (g_screen->isInRect(p.x, p.y, BUTTON_SHUTDOWN)) {
        g_screen->setMessage("Shutdown...", true);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_deep_sleep_start();
    }
}

/**
 * @brief Button callback handler
 */
void buttonCallback(uint8_t button_id) {
    switch (button_id) {
        case ButtonController::BTN_UP:
            // Speaker volume up
            uint8_t level = g_vario->getVarioData().getSpeaker();
            level += 25;
            if (level > 100) level = 100;
            g_vario->setSpeaker(level);
            ESP_LOGI(TAG, "Speaker: %d%%", level);
            break;
            
        case ButtonController::BTN_CENTER:
            // Toggle menu
            if (g_screen->switchMenu()) {
                g_touch->wakeup();
            } else {
                g_touch->sleep();
            }
            break;
            
        case ButtonController::BTN_DOWN:
            // Speaker volume down
            level = g_vario->getVarioData().getSpeaker();
            if (level >= 25) {
                level -= 25;
            } else {
                level = 0;
            }
            g_vario->setSpeaker(level);
            ESP_LOGI(TAG, "Speaker: %d%%", level);
            break;
    }
}

/**
 * @brief Vario update task
 */
void varioUpdateTask(void *pvParameters) {
    for (;;) {
        if (g_vario && g_vario->isUpdated()) {
            LiLyGoEPD47 data = g_vario->getVarioData();
            
            // Update screen with new data
            if (g_screen) {
                g_screen->setVarioData(data);
            }
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief WiFi server task
 */
void wifiServerTask(void *pvParameters) {
    for (;;) {
        if (g_wifiServer && g_vario->isWifiEnabled()) {
            // WiFi server loop handled internally
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Initialize display
 */
void setupDisplay() {
    ESP_LOGI(TAG, "Initializing display...");
    
    g_display = new DisplayAdapter_LCD();
    if (!g_display) {
        ESP_LOGE(TAG, "Failed to allocate display adapter");
        return;
    }
    
    g_display->begin();
    g_display->fillScreen(COLOR_BLACK);
    g_display->drawString("Variometer CYD", 10, 10, COLOR_WHITE);
    g_display->drawString("Initializing...", 10, 30, COLOR_CYAN);
    
    ESP_LOGI(TAG, "Display initialized: %dx%d", g_display->getWidth(), g_display->getHeight());
}

/**
 * @brief Initialize touch sensor
 */
void setupTouch() {
    ESP_LOGI(TAG, "Initializing touch sensor...");
    
    g_touch = new TouchSensor_FT6236(TOUCH_INT, TOUCH_SDA, TOUCH_SCL, TOUCH_ADDR);
    if (!g_touch) {
        ESP_LOGE(TAG, "Failed to allocate touch sensor");
        return;
    }
    
    if (!g_touch->begin()) {
        ESP_LOGE(TAG, "Failed to initialize FT6236 touch sensor");
        return;
    }
    
    g_touch->setCallback(touchCallback);
    
    // Start touch polling task
    xTaskCreatePinnedToCore(
        TouchSensor_FT6236::pollTask,
        "TouchPoll",
        4 * 1024,
        g_touch,
        3,
        nullptr,
        0
    );
    
    ESP_LOGI(TAG, "Touch sensor initialized");
}

/**
 * @brief Initialize buttons
 */
void setupButtons() {
    ESP_LOGI(TAG, "Initializing buttons...");
    
    g_buttons = new ButtonController(BTN_UP, BTN_CENTER, BTN_DOWN);
    if (!g_buttons) {
        ESP_LOGE(TAG, "Failed to allocate button controller");
        return;
    }
    
    g_buttons->begin();
    g_buttons->setCallback(buttonCallback);
    
    // Start button polling task
    xTaskCreatePinnedToCore(
        ButtonController::pollTask,
        "ButtonPoll",
        4 * 1024,
        g_buttons,
        2,
        nullptr,
        0
    );
    
    ESP_LOGI(TAG, "Buttons initialized");
}

/**
 * @brief Initialize vario sensor
 */
void setupVario() {
    ESP_LOGI(TAG, "Initializing BlueFly vario...");
    
    // TODO: Implement BlueFlyVario initialization
    // g_vario = new BlueFlyVario(BF_RX_PIN, BF_TX_PIN);
    
    ESP_LOGI(TAG, "BlueFly vario initialized");
}

/**
 * @brief Initialize WiFi
 */
void setupWifi() {
    ESP_LOGI(TAG, "WiFi setup ready");
    
    // TODO: Implement WiFi server initialization
    // g_wifiServer = new WifiWebServer(g_vario->getStream());
}

/**
 * @brief Main CYD initialization
 */
void idf_setup_cyd() {
    ESP_LOGI(TAG, "Starting FREENOVE CYD Variometer v0.1");
    
    // Initialize filesystem
    if (!FFat.begin(true, "/ffat")) {
        ESP_LOGE(TAG, "FFat mount failed");
    } else {
        ESP_LOGI(TAG, "FFat mounted");
    }
    
    // Initialize hardware in order
    setupDisplay();
    setupTouch();
    setupButtons();
    setupVario();
    setupWifi();
    
    // Initialize main screen
    ESP_LOGI(TAG, "Initializing main screen...");
    g_screen = new MainScreen_CYD(g_display);
    if (g_screen) {
        g_screen->begin();
    }
    
    // Create vario update task
    xTaskCreatePinnedToCore(
        varioUpdateTask,
        "VarioUpdate",
        4 * 1024,
        nullptr,
        4,
        &taskVario,
        0
    );
    
    // Set startup message
    if (g_screen) {
        g_screen->setMessage(u8"System ready", true);
    }
    
    ESP_LOGI(TAG, "Setup complete");
}

/**
 * @brief Main CYD loop
 */
void idf_loop_cyd() {
    vTaskDelay(100 / portTICK_PERIOD_MS);
}
