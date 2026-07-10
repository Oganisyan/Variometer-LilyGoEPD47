# FREENOVE ESP32 CYD Portierung - Dokumentation

## Übersicht

Diese Portierung adaptiert das Original-Variometer-Projekt (entwickelt für **LilyGo EPD47 E-Ink 4.7"**) auf das **FREENOVE ESP32 CYD 3.2" IPS-LCD Display**.

### Hardware-Unterschiede

| Aspekt | LilyGo EPD47 | FREENOVE CYD |
|--------|-----------|----------|
| **Display** | E-Ink 4.7", 960×540px, schwarz/weiß | IPS-LCD 3.2", 320×240px, RGB565 Farbe |
| **Controller** | Epson EPD Driver | ILI9341 |
| **Touch** | I2C Capacitive (0x5A) | FT6236 I2C Capacitive (0x38) |
| **Update-Rate** | Langsam (~1Hz) | Schnell (bis 60Hz) |
| **Farben** | 4-bit Graustufen | 16-bit RGB |
| **Energieverbrauch** | ~0.5mA (Sleep) | ~20mA (aktiv) |

## Neue Dateien für CYD-Port

### Display-Abstraktions-Layer
- **FreenoveConfig.h** - Hardware-Konfiguration (Pins, Farben, Größen)
- **DisplayAdapter.h** - Abstraktes Interface für Display-Operationen
- **DisplayAdapter_LCD.h/cpp** - Implementierung für ILI9341 LCD (nutzt TFT_eSPI)

### Eingabe-Controller
- **TouchSensor_FT6236.h/cpp** - FT6236 Touchscreen-Treiber
- **ButtonController.h/cpp** - Hardware-Button Manager

### UI/Screen
- **MainScreen_CYD.h/cpp** - Angepasste Benutzeroberfläche für 320×240
- **LilyGo-EPD47-CYD.h/cpp** - CYD-spezifische Initialisierung

## Architektur

### Abstraktions-Layer
```
Application (BlueFlyVario, MainScreen_CYD)
        ↓
   DisplayAdapter (unified API)
        ↓
DisplayAdapter_LCD (ILI9341/ST7789)
        ↓
  TFT_eSPI Library
        ↓
   Hardware (SPI, GPIO)
```

### Task-Struktur
```
Core 0:                          Core 1:
├─ Vario Read (BlueFlyVario)     ├─ Display Paint (MainScreen_CYD)
├─ WiFi Server                   ├─ (reserved for rendering)
└─ GPS Parser (TinyGPS++)        

Shared:
├─ Touch Poll (IRQ-based, Core 0)
├─ Button Poll (Core 0)
└─ Mutex-protected: LiLyGoEPD47 data struct
```

## Installation & Setup

### 1. Abhängigkeiten (platformio.ini)

```ini
[env:freenove-cyd]
platform = espressif32
board = esp32-s3-devkitc-1  ; oder lolin32
framework = espidf, arduino

lib_deps =
    TFT_eSPI@2.5.0              ; LCD-Rendering
    TinyGPS++@1.0.3             ; GPS-Parsing
    ArduinoJson@6.21.0          ; WiFi-Config
    arduino-libraries/NTPClient@3.1.0

build_flags =
    -D BOARD_HAS_PSRAM
    -D CORE_DEBUG_LEVEL=3
    -D TFT_eSPI_DRIVER=0         ; ILI9341
    -D TFT_WIDTH=320
    -D TFT_HEIGHT=240
```

### 2. TFT_eSPI Konfiguration

In `User_Setup_Select.h`:
```cpp
// Setup 25: ILI9341 auf FREENOVE CYD
#include <../Generic/ILI9341_Parallel.h>
```

Oder manuell `User_Setup.h`:
```cpp
#define ILI9341_DRIVER
#define TFT_WIDTH 320
#define TFT_HEIGHT 240

// SPI Pins für FREENOVE CYD
#define TFT_CS   GPIO_NUM_5
#define TFT_RST  GPIO_NUM_27
#define TFT_DC   GPIO_NUM_17
#define TFT_MOSI GPIO_NUM_23
#define TFT_SCL  GPIO_NUM_18
#define TFT_MISO GPIO_NUM_19
#define TFT_BL   GPIO_NUM_16
```

### 3. Hauptprogramm (main.cpp)

```cpp
#include "LilyGo-EPD47-CYD.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    idf_setup_cyd();
}

void loop() {
    idf_loop_cyd();
}
```

## GPIO Pin-Zuordnung

### Display (ILI9341 SPI)
| Pin | GPIO | Funktion |
|-----|------|----------|
| CS  | 5    | Chip Select |
| DC  | 17   | Data/Command |
| RST | 27   | Reset |
| MOSI| 23   | SPI Data Out |
| CLK | 18   | SPI Clock |
| MISO| 19   | SPI Data In |
| BL  | 16   | Backlight PWM |

### Touch (I2C FT6236)
| Pin | GPIO | Funktion |
|-----|------|----------|
| SDA | 21   | I2C Data |
| SCL | 22   | I2C Clock |
| INT | 39   | Interrupt |

### Buttons
| Pin | GPIO | Funktion |
|-----|------|----------|
| UP  | 34   | Speaker + |
| MID | 0    | Menu Toggle |
| DWN | 35   | Speaker - |

### ADC (Battery)
| Pin | GPIO | Funktion |
|-----|------|----------|
| BAT | 35   | Batterie-Spannung |

## Layout-Anpassungen

### Original EPD (960×540)
```
┌─────────────────────┐
│      Header (90px)  │
├─────────┬───────────┤
│         │           │
│ Vario   │  Daten    │
│ (gauge) │  (tabs)   │
│         │           │
├─────────┴───────────┤
│   Control (90px)    │
└─────────────────────┘
```

### CYD LCD (320×240)
```
┌────────────────┐
│   Header (28)  │
├────────┬───────┤
│        │ Data  │
│ Vario  │(compact)
│        │       │
├────────┴───────┤
│  Controls (32) │
└────────────────┘
```

## Farbschema (RGB565)

```cpp
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800  // Descent
#define COLOR_GREEN     0x07E0  // Climb
#define COLOR_CYAN      0x07FF  // Status
#define COLOR_YELLOW    0xFFE0  // Messages
#define COLOR_DARK_GRAY 0x4208  // Background
```

## Performance-Tipps

### Display-Rendering
- LCD aktualisiert sofort (kein EPD-Flicker)
- Refresh-Rate: ~100ms für stabiles Bild
- Backlight PWM: GPIO16 für Energiesparen

### Touch-Sampling
- FT6236 Poll-Interval: 50ms
- Max 5 simultane Touch-Punkte
- I2C Speed: 400kHz (FT6236-Standard)

### Memory
- RAM: ~100KB für DisplayAdapter + MainScreen_CYD
- PSRAM: Optional für erweiterte Features

## Migration vom EPD-Port

### Bestehender Code bleibt unverändert
- `BlueFlyVario` (Serial-Kommunikation)
- `WifiWebServer` (WiFi-Interface)
- `TinyGPS++` (GPS-Parsing)
- `LiLyGoEPD47` (Datenmodell)

### Was sich ändert
- **Display-Rendering** → DisplayAdapter abstrahiert Details
- **UI-Layout** → MainScreen_CYD optimiert für kleine Auflösung
- **Touchscreen** → FT6236 statt E-Ink-Controller
- **Farbe** → RGB565 statt Graustufen

## Debugging

### Serial-Monitor
```bash
picocom -b 115200 /dev/ttyUSB0
```

### Logs
```cpp
esp_log_level_set(TAG, ESP_LOG_DEBUG);
```

### TouchScreen Kalibrierung
```cpp
g_touch->setCalibration(0, 320, 0, 240);
// oder für gekippte Achsen:
g_touch->setCalibration(0, 240, 0, 320);
```

## Bekannte Limitierungen

1. **Kleineres Display** - Weniger Platz für Information
   - Lösung: Tabbierte Navigation oder Scrollen

2. **Schneller Batterie-Drain** (LCD vs E-Ink)
   - Lösung: Sleep-Modus nach 5min Inaktivität

3. **Keine Backlight beim Starten**
   - Lösung: Backlight-Init in DisplayAdapter_LCD::begin()

## Weitere Entwicklung

- [ ] BMP388/BMP180 Barometer-Treiber
- [ ] GPS-Module Integration
- [ ] SD-Karten Flight-Logging
- [ ] Bluetooth Audio (Variometer-Töne)
- [ ] Kalibrierungs-UI

## Support & Links

- **TFT_eSPI**: https://github.com/Bodmer/TFT_eSPI
- **FT6236**: https://www.focaltech-systems.com/
- **FREENOVE ESP32**: https://github.com/Freenove/Freenove_ESP32
- **Original LilyGo**: https://github.com/Xinyuan-LilyGO/LilyGo-EPD47
