# ESP-IDF 5.5 Component CMakeLists.txt
# Modernes idf_component_register API

idf_component_register(
    SRCS
        main.cpp
        AnalogVarioWidget.cpp
        BlueFlyVario.cpp
        ButtonController.cpp
        DisplayAdapter.cpp
        DisplayAdapter_LCD.cpp
        KalmanFilter.cpp
        LilyGo-EPD47.cpp
        MainScreen_CYD.cpp
        MyEpdUtil.cpp
        Power.cpp
        SpeedometerWidget.cpp
        TinyGPS++.cpp
        TouchSensor_FT6236.cpp
        Tracking.cpp
        WifiWebServer.cpp
        FileDTO.cpp
        MySettings.cpp
    
    INCLUDE_DIRS
        .
    
    PRIV_REQUIRES
        driver
        esp_timer
        freertos
        hal
        esp_wifi
        esp_event
        esp_http_client
        esp_http_server
        fatfs
        nvs_flash
        esp_rom
        esp_common
        esp_adc
        esp_gpio
        esp_psram
)
