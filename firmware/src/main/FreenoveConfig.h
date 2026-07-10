#pragma once

// FREENOVE ESP32 CYD 3.2" Display Configuration
// Hardware: ILI9341 LCD (320x240, RGB565) + FT6236 Touchscreen

// Display Settings
#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240
#define DISPLAY_ROTATION 1  // 0=Portrait, 1=Landscape, 2=Inverted Portrait, 3=Inverted Landscape

// SPI Pins for ILI9341
#define ILI9341_CS      GPIO_NUM_5   // Chip Select
#define ILI9341_DC      GPIO_NUM_17  // Data/Command
#define ILI9341_RST     GPIO_NUM_27  // Reset
#define ILI9341_SDA     GPIO_NUM_23  // MOSI (SPI Data In)
#define ILI9341_SCL     GPIO_NUM_18  // Clock
#define ILI9341_MISO    GPIO_NUM_19  // MISO (SPI Data Out)
#define ILI9341_BL      GPIO_NUM_16  // Backlight
#define ILI9341_SPI_FREQ 40000000   // 40 MHz SPI frequency

// I2C Pins for Touch (FT6236)
#define TOUCH_SDA       GPIO_NUM_21
#define TOUCH_SCL       GPIO_NUM_22
#define TOUCH_INT       GPIO_NUM_39  // Interrupt pin
#define TOUCH_ADDR      0x38         // FT6236 I2C address
#define TOUCH_WIDTH     320          // Raw touch width
#define TOUCH_HEIGHT    240          // Raw touch height

// ADC Pin for Battery
#define BAT_ADC         GPIO_NUM_35  // GPIO35 (ADC1_CH7) for battery voltage

// Button Pins
#define BTN_UP          GPIO_NUM_34  // Left button (Speaker +)
#define BTN_CENTER      GPIO_NUM_0   // Center button (Menu toggle)
#define BTN_DOWN        GPIO_NUM_35  // Right button (Speaker -)

// Color Scheme (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_YELLOW    0xFFE0
#define COLOR_DARK_GRAY 0x4208
#define COLOR_LIGHT_GRAY 0xC618

// Color Scheme for Vario
#define VARIO_COLOR_UP    COLOR_GREEN   // Climbing
#define VARIO_COLOR_DOWN  COLOR_RED     // Descending
#define VARIO_COLOR_NEUTRAL COLOR_WHITE // Neutral

// UI Layout Constants
#define HEADER_HEIGHT   30
#define CONTROL_HEIGHT  35
#define TAB_HEIGHT      (DISPLAY_HEIGHT - HEADER_HEIGHT - CONTROL_HEIGHT) / 3
#define TAB_WIDTH       DISPLAY_WIDTH / 3

// Performance Settings
#define TOUCH_POLL_INTERVAL_MS  50   // Touch polling interval
#define DISPLAY_REFRESH_RATE_MS 100  // Display update rate
#define BUTTON_DEBOUNCE_MS      20   // Button debounce time

// Font sizes (TFT_eSPI uses built-in fonts 1-8)
#define FONT_LARGE      4   // Large font for vario display
#define FONT_MEDIUM     3   // Medium font for data
#define FONT_SMALL      2   // Small font for labels
