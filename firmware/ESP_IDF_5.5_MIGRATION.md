# ESP-IDF 5.5 Migration Guide

## Überblick

Dieses Dokument beschreibt die Migration des Variometer-Projekts von **ESP-IDF 4.4 zu 5.5** mit vollständiger Chip-Unterstützung:

- ✅ ESP32 (klassisch)
- ✅ ESP32-S3 (FREENOVE CYD)
- ✅ ESP32-C3 (geplant)
- ✅ ESP32-P4 (neu!)

---

## 1. Build-System Migration

### Vorher (ESP-IDF 4.4)

```cmake
# firmware/src/main/CMakeLists.txt
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS main.cpp ...)
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
```

### Nachher (ESP-IDF 5.5)

```cmake
# firmware/src/main/CMakeLists.txt
idf_component_register(
    SRCS main.cpp ...
    INCLUDE_DIRS .
    PRIV_REQUIRES driver esp_timer freertos hal ...
)
```

**Vorteile:**
- ✅ Moderne, lesbare API
- ✅ Bessere Dependency Resolution
- ✅ CMake 3.16+ optimiert
- ✅ Bessere Fehlerbehandlung

---

## 2. sdkconfig - Multi-Target Support

### Dateistruktur

```
firmware/src/
├── sdkconfig           (Default → sdkconfig.esp32)
├── sdkconfig.esp32     (Klassischer ESP32)
├── sdkconfig.esp32s3   (FREENOVE CYD - empfohlen)
├── sdkconfig.esp32c3   (Low-Power)
└── sdkconfig.esp32p4   (High-Performance)
```

### Target auswählen

```bash
# ESP32 (klassisch)
idf.py set-target esp32

# ESP32-S3 (FREENOVE CYD)
idf.py set-target esp32s3

# ESP32-P4 (Next-Gen)
idf.py set-target esp32p4

# Automatisch sdkconfig.esp32p4 nutzen
```

### Wichtige Konfigurationen

| Setting | 4.4 | 5.5 | Bedeutung |
|---------|-----|-----|----------|
| `CONFIG_IDF_TARGET` | `esp32` | `esp32p4` | Ziel-Chip |
| `CONFIG_COMPILER_CXX_STANDARD` | `c++14` | `c++17` | Neuer Standard |
| `CONFIG_SPIRAM_SPEED_80M` | Manual | Auto-detect | PSRAM-Optimierung |
| `CONFIG_PERFORMANCE_LEVEL_*` | N/A | `MEDIUM/HIGH` | CPU-Taktung |
| `CONFIG_FREERTOS_TICK_RATE_HZ` | 1000 | 1000 | Kein Unterschied |

---

## 3. Code-Änderungen

### A. Logging (esp_log.h)

**Vorher (4.4):**
```cpp
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

static const char *TAG = "MyModule";
ESP_LOGD(TAG, "Debug: %d", value);
```

**Nachher (5.5):**
```cpp
#include "esp_log.h"

static const char *TAG = "MyModule";

// Für Debugging im Setup:
esp_log_level_set(TAG, ESP_LOG_DEBUG);

ESP_LOGD(TAG, "Debug: %d", value);  // Funktioniert wie vorher
```

**Alle Log-Level funktionieren:**
```cpp
ESP_LOGE(TAG, "Error");      // Error
ESP_LOGW(TAG, "Warning");    // Warning
ESP_LOGI(TAG, "Info");       // Info
ESP_LOGD(TAG, "Debug");      // Debug
ESP_LOGV(TAG, "Verbose");    // Verbose
```

### B. FreeRTOS Tasks

**Vorher (4.4):**
```cpp
xTaskCreatePinnedToCore(
    taskFunc,          // Function
    "TaskName",        // Name
    4096,              // Stack size
    nullptr,           // Parameter
    5,                 // Priority
    &taskHandle,       // Task handle
    1                  // Core (0 or 1)
);
```

**Nachher (5.5) - Identisch, aber erweitert:**
```cpp
xTaskCreatePinnedToCore(
    taskFunc,
    "TaskName",
    4096,
    nullptr,
    5,
    &taskHandle,
    1  // Core: 0, 1, oder tskNO_AFFINITY (5.5 neu!)
);

// Optional: Extended version mit noch mehr Features
xTaskCreatePinnedToCore(
    taskFunc,
    "TaskName",
    4096,
    nullptr,
    5,
    &taskHandle,
    tskNO_AFFINITY    // Kein Core-Pinning
);
```

### C. GPIO

**Vorher (4.4):**
```cpp
#include "driver/gpio.h"

gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
gpio_set_level(GPIO_NUM_5, 1);
```

**Nachher (5.5) - Kompatibel + modernisiert:**
```cpp
#include "driver/gpio.h"

// Alt (funktioniert): gpio_set_*
gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
gpio_set_level(GPIO_NUM_5, 1);

// Neu (empfohlen):
gpio_config_t cfg = {
    .pin_bit_mask = (1ULL << GPIO_NUM_5),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};
gpio_config(&cfg);
gpio_set_level(GPIO_NUM_5, 1);
```

### D. I2C

**Vorher (4.4):**
```cpp
#include "driver/i2c.h"

i2c_config_t conf = {};
conf.mode = I2C_MODE_MASTER;
conf.sda_io_num = SDA_PIN;
conf.scl_io_num = SCL_PIN;
conf.master.clk_speed = 400000;

i2c_param_config(I2C_NUM_0, &conf);
i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
```

**Nachher (5.5) - Identisch:**
```cpp
// Kein Code-Unterschied - i2c.h API unverändert
```

### E. ADC

**Vorher (4.4):**
```cpp
#include "driver/adc.h"

adc1_config_width(ADC_WIDTH_BIT_12);
adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

uint16_t value = adc1_get_raw(ADC1_CHANNEL_0);
```

**Nachher (5.5) - Modern (aber beide funktionieren):**
```cpp
#include "esp_adc/adc_oneshot.h"

adc_oneshot_unit_handle_t adc_handle;
adc_oneshot_unit_init_cfg_t init_cfg = {
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};
adc_oneshot_new_unit(&init_cfg, &adc_handle);

adc_oneshot_chan_cfg_t chan_cfg = {
    .bitwidth = ADC_BITWIDTH_12,
    .atten = ADC_ATTEN_DB_11,
};
adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_0, &chan_cfg);

int value = 0;
adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &value);
```

---

## 4. Chip-spezifische Unterschiede

### ESP32 (Klassik)
```
✅ Dual-Core (240 MHz)
✅ 512KB SRAM + 4MB FLASH
✅ WiFi 802.11 b/g/n
✅ Bluetooth 4.2
⚠️  Langsamer als moderne Varianten
```

### ESP32-S3 (FREENOVE CYD - empfohlen)
```
✅ Dual-Core (240 MHz)
✅ 512KB SRAM + bis 16MB PSRAM
✅ WiFi 802.11 b/g/n
✅ BLE 5.0
✅ USB OTG / JTAG
✅ SHA, HMAC, ECC beschleunigt
⭐ Beste Balance für dieses Projekt
```

### ESP32-P4 (Next-Gen - Experimental)
```
✅ Quad-Core (240+ MHz)
✅ 768KB SRAM + bis 8MB PSRAM
✅ WiFi 6 (802.11ax)
✅ BLE 5.3
✅ Native USB 2.0
✅ Höhere Taktung
⚠️  Noch nicht alle Features stabil
```

---

## 5. Build & Flash

### Setup
```bash
# ESP-IDF 5.5 installieren
curl -L https://github.com/espressif/esp-idf/releases/download/v5.5.0/esp-idf-v5.5.tar.gz | tar xz
cd esp-idf
./install.sh
source ./export.sh

cd ~/Variometer-LCD/firmware/src
```

### Bauen
```bash
# Target auswählen (z.B. ESP32-S3)
idf.py set-target esp32s3

# Konfigurieren
idf.py menuconfig

# Bauen
idf.py build

# Monitor + Flash
idf.py -p /dev/ttyUSB0 flash monitor
```

### Output
```
Building idf_component 'main' because it has no build log or cmake.toolchain has changed.
...
ELF file SHA256: abc123...
SHA256 digest of BOOTLOADER: def456...
SHA256 digest of secure boot key: ...

Flashed firmware successfully!

--- idf_monitor ---
I (0) cpu_start: Starting scheduler on APP CPU.
I (MainScrean_CYD): LCD Display initialized: 320x240
I (TouchSensor_FT6236): FT6236 Touch sensor initialized
System ready
```

---

## 6. Performance-Optimierung

### ESP32-S3 (FREENOVE CYD)

```cmake
# CMakeLists.txt
idf_component_register(
    ...
    PRIV_REQUIRES driver esp_timer freertos
)
```

```ini
# sdkconfig.esp32s3
CONFIG_SPIRAM_SPEED_80M=y          # PSRAM auf 80 MHz
CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=8192
CONFIG_PERFORMANCE_LEVEL_MEDIUM=y  # Normale Taktung
CONFIG_FREERTOS_TICK_RATE_HZ=1000  # 1 kHz Systemtakt
```

### ESP32-P4 (High-Performance)

```ini
# sdkconfig.esp32p4
CONFIG_SPIRAM_SPEED_120M=y         # PSRAM auf 120 MHz
CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL=32768
CONFIG_PERFORMANCE_LEVEL_HIGH=y    # Höchste Taktung
CONFIG_ESP_TASK_WDT_TIMEOUT_S=5    # Watchdog
```

---

## 7. Troubleshooting

### Problem: "Target mismatch"
```
Target 'esp32' in sdkconfig does not match IDF_TARGET 'esp32p4'
```

**Lösung:**
```bash
rm -rf build sdkconfig
idf.py set-target esp32p4
idf.py menuconfig  # Speichern
idf.py build
```

### Problem: "Component not found"
```
ERROR: Unknown component 'driver'
```

**Lösung:** `idf_component_register()` mit vollständigen `PRIV_REQUIRES` verwenden:
```cmake
PRIV_REQUIRES driver esp_timer freertos hal esp_wifi esp_event
```

### Problem: "C++17 features not available"
```
error: 'auto' is not supported by '-std=c++14'
```

**Lösung:** In `sdkconfig`:
```ini
CONFIG_COMPILER_CXX_STANDARD="c++17"
```

---

## 8. Backward Compatibility

Dieser Port ist **kompatibel** mit:
- ✅ ESP-IDF 5.1, 5.2, 5.3, 5.4, 5.5
- ✅ Alle CMake-Versionen ab 3.16
- ✅ Arduino-Framework (optional)
- ✅ Bestehende Komponenten (BlueFlyVario, WifiWebServer, etc.)

**Nicht kompatibel mit:**
- ❌ ESP-IDF 4.4 (verwende `feature/esp-idf-4.4` Branch)
- ❌ Arduino IDE (verwende PlatformIO stattdessen)

---

## 9. Weitere Ressourcen

- [ESP-IDF 5.5 Dokumentation](https://docs.espressif.com/projects/esp-idf/en/v5.5/)
- [Migration Guide](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/migration-guides/index.html)
- [CMake Build System](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-guides/build-system.html)
- [FreeRTOS Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel)

---

**Stand:** July 10, 2026  
**Version:** 5.5.0  
**Mantainer:** Martin Oganisyan
