#include "ButtonController.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "ButtonController";
static const uint32_t DEBOUNCE_TIME = 20;  // ms

ButtonController::ButtonController(int16_t btn_up, int16_t btn_center, int16_t btn_down)
    : btn_up_(btn_up), btn_center_(btn_center), btn_down_(btn_down), callback_(nullptr) {
    for (int i = 0; i < 3; i++) {
        last_press_time_[i] = 0;
        btn_pressed_[i] = false;
    }
}

ButtonController::~ButtonController() {
}

void ButtonController::begin() {
    pinMode(btn_up_, INPUT_PULLUP);
    pinMode(btn_center_, INPUT_PULLUP);
    pinMode(btn_down_, INPUT_PULLUP);
    
    ESP_LOGI(TAG, "Button controller initialized");
}

void ButtonController::end() {
}

void ButtonController::checkButton(uint8_t btn_id) {
    if (btn_id >= 3) return;
    
    int16_t pin = (btn_id == BTN_UP) ? btn_up_ : 
                  (btn_id == BTN_CENTER) ? btn_center_ : btn_down_;
    
    bool is_pressed = (digitalRead(pin) == LOW);
    uint32_t now = millis();
    
    if (is_pressed && !btn_pressed_[btn_id]) {
        // Button pressed
        if ((now - last_press_time_[btn_id]) > DEBOUNCE_TIME) {
            btn_pressed_[btn_id] = true;
            last_press_time_[btn_id] = now;
            
            if (callback_) {
                callback_(btn_id);
            }
            
            ESP_LOGD(TAG, "Button %d pressed", btn_id);
        }
    } else if (!is_pressed && btn_pressed_[btn_id]) {
        // Button released
        if ((now - last_press_time_[btn_id]) > DEBOUNCE_TIME) {
            btn_pressed_[btn_id] = false;
            ESP_LOGD(TAG, "Button %d released", btn_id);
        }
    }
}

void ButtonController::pollTask(void *parameter) {
    ButtonController *controller = static_cast<ButtonController*>(parameter);
    
    for (;;) {
        controller->checkButton(BTN_UP);
        controller->checkButton(BTN_CENTER);
        controller->checkButton(BTN_DOWN);
        
        vTaskDelay(20 / portTICK_PERIOD_MS);  // 20ms poll interval
    }
}
