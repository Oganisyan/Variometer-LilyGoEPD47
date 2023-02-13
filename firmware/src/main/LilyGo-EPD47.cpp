/* Simple firmware for a ESP32 displaying a static image on an EPaper Screen.

   Write an image into a header file using a 3...2...1...0 format per pixel,
   for 4 bits color (16 colors - well, greys.) MSB first.  At 80 MHz, screen
   clears execute in 1.075 seconds and images are drawn in 1.531 seconds.
*/

#include "LilyGo-EPD47.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <FFat.h>
#include "WifiWebServer.h"


void taskDelay(int ms) {
	vTaskDelay(ms / portTICK_PERIOD_MS);
}



#include "epd_driver.h"
#include "epd_highlevel.h"

int epd_driver_v6_vcom = 1560;

#include "Power.h"
#define BAT_ADC 36
Power power(BAT_ADC); // @suppress("Invalid arguments")

// select the font based on display width
#include "Melone_40.h"
#include "Melone_20.h"
#include "Melone_16.h"

#define FONT_L Melone_40  // Large
#define FONT_M Melone_20  // Middle
#define FONT_S Melone_16  // Small

#include "MainScrean.h"

MainScrean screan(FONT_L, FONT_M, FONT_S);

#define TOUCH_INT   13
#define TOUCH_SDA	15
#define TOUCH_SCL	14

#include "touch.h"
TouchClass touch(TOUCH_INT, TOUCH_SDA, TOUCH_SCL);

#include "BlueFlyVario.h"

#define BF_RX_PIN 12
#define BF_TX_PIN TOUCH_INT

BlueFlyVario vario(BF_RX_PIN, BF_TX_PIN);
WifiWebServer wifiServer(vario.getStream());


#include "ButtonCtrl.h"

ButtonCtrl buttonCtrl(34, 35, 39);

#define WAVEFORM EPD_BUILTIN_WAVEFORM



uint32_t t1, t2;

int temperature;
EpdiyHighlevelState hl;


void shutdown() {
	touch.sleep();
	screan.setMessage(u8"System shutdown \ue00b", true);
	esp_sleep_enable_ext0_wakeup(GPIO_NUM_35,0);
	delay(5000);
	esp_deep_sleep_start();
}

void setSpeaker(uint8_t level) {
	bool isTouchEnabeled = touch.isEnabeled();
	if(isTouchEnabeled) touch.sleep();
	vario.setSpeaker(level);
	if(isTouchEnabeled) touch.wakeup();
}

void speakerPP() {
	uint8_t level = vario.getVarioData().getSpeaker();
	level += 0x19;
	if(level > 0x64) level=100;
	setSpeaker(level);
}
void speakerMM() {
	uint8_t level = vario.getVarioData().getSpeaker();
	level -= 0x19;
	if(level > 0x64) level=0;
	setSpeaker(level);
}

void switchWiFi() {
	char buffer[64];
	touch.sleep();
	if(vario.switchWiFi()) {
		wifiServer.begin();
		if(wifiServer.getAaddress() == WifiWebServer::BAD_IP_ADDRESS) {
			return switchWiFi();
		}
		screan.setMessage(format(buffer, u8"\ue011 %s%s %s",
				wifiServer.getSettings().getSsid(),
				wifiServer.getSettings().isSoftIP() ? "*" : "",
				wifiServer.getAaddress().toString().c_str()), true);
		screan.repaint();
	} else {
		screan.setMessage(u8"\ue011 WiFi Off", true);
		wifiServer.end();
		screan.repaint();
	}
}

void switchTracking() {
	touch.sleep();
	if(vario.switchTracking()) {
		screan.setMessage(u8"\ue017 Tracking On", true);

	} else {
		screan.setMessage(u8"\ue017 Tracking Off", true);
	}
	screan.repaint();
}

void  rotate () {
  epd_set_rotation(static_cast<EpdRotation>((epd_get_rotation() + 1) % 4));
  char buf[64];
  screan.setMessage(format( buf, "EPD Rotation: %d", epd_get_rotation()), true);
  screan.repaint();
}

void caclQNHFromGPS () {
	float altGPS =static_cast<float>(vario.getVarioData().getAltGps().meters());
	vario.calcQNH(altGPS);
	screan.setMessage("Calc QNH from GPS!", true);
	screan.repaint();
}

void setStartAltZero () {
	vario.setStartAlt(vario.getVarioData().getAltitude());
	screan.setMessage("Reset Alt2!", true);
	screan.repaint();
}


void enableVarioConfig(bool enable) {
	vario.enableVarioConfig(enable);
	if(enable)
		screan.setMessage("Enable Vario Config!", true);
}


void  onePointCallBack (touchPoint_t& p1) {
  if(screan.isInRect(p1.nX, p1.nY, BUTTON_SHUTDOWN)) {
    shutdown();
  } else if(screan.isInRect(p1.nX, p1.nY, BUTTON_MENU)) {
	  switchWiFi();
	  //listDir((fs::FS &)SPIFFS, "/");
  } else if(screan.isInRect(p1.nX, p1.nY, BUTTON_USE_GPS_ALT)) {
	  caclQNHFromGPS();
  } else if(screan.isInRect(p1.nX, p1.nY, BUTTON_RESET_A2)) {
	  setStartAltZero();
  } else if(screan.isInRect(p1.nX, p1.nY, BUTTON_CONFIG_VARIO)) {
	  enableVarioConfig(true);
  } else if(screan.isInRect(p1.nX, p1.nY, BUTTON_TRACKING)) {
	  switchTracking();//rotate();
  }
}






void simulate(void *) {
	power.setup();
    for(;;) {
    	if(vario.isUpdated()) {
			LiLyGoEPD47 d = vario.getVarioData();
			d.setBatVolt(power.get());
			screan.setVarioData(d);
    	}
        taskDelay(100);
    }
}

void buttonAction(int pin) {
  switch(pin) {
    case 34:
    	//rotate();
    	speakerPP();
    	break;
    case 35:
		if(screan.switchMenu()) {
		    enableVarioConfig(false);
			touch.wakeup();
		} else {
			touch.sleep();
		}
		break;
    case 39:
        //shutdown();
    	speakerMM();
    	break;
    default: 
      break; 
  }
}

void idf_loop() {
	taskDelay(100);
}

void checkOrCreateDir(fs::FS &fs, const char *path) {
	if(!fs.open(path)) {
		printf("Crate folder %s : %s\n", path,  fs.mkdir(path) ? "OK" : "Fail");
	} else {
		printf("Find folder %s\n", path);
	}
}


void idf_setup() {

	//FFat.format();
    if(!FFat.begin(true, "/ffat")){
		printf("FFat Mount Failed\n");
	} else {
		checkOrCreateDir(FFat,"/tracking");
		checkOrCreateDir(FFat, "/config");
	}

    pinMode(TOUCH_INT, INPUT_PULLUP);
  
    epd_init(EPD_LUT_1K);
    hl = epd_hl_init(WAVEFORM); // @suppress("Invalid arguments")
  
    // Default orientation is EPD_ROT_LANDSCAPE
    epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
    epd_fullclear(&hl, epd_ambient_temperature());
  
    TaskHandle_t taskHandle;
    xTaskCreatePinnedToCore(
        simulate,           				// Function that should be called
        "Simulate",         				// Name of the task (for debugging)
        4*1024,             				// Stack size (bytes)
        NULL,               				// Parameter to pass
        4,                  				// Task priority
        &taskHandle,         				// Task handle
        0                   				// Core you want to run the task on (0 or 1)
    );
    if(!taskHandle)  {
        printf("\n Error: Cant Create Update Screan Task\n");  
    } else {
        printf("Start simulation...\n");
    }
    
    touch.setOnePointCallBack( onePointCallBack);
    //touch.setTwoPointCallBack( twoPointCallBack);
    //touch.setTrePointCallBack( trePointCallBack);
    //touch.setForPointCallBack( forPointCallBack);
  
    if (!touch.begin()) {
    	printf("Start Touch failed\n");
    } else {
    	printf("Start Touch ok\n");
    }
    touch.sleep();
    xTaskCreatePinnedToCore(
        TouchClass::poolCallBack,   // Function that should be called
        "TouchClass Task",          // Name of the task (for debugging)
        4*1024,                     // Stack size (bytes)
        &touch,                     // Parameter to pass
        3,      					// Task priority
		&taskHandle,    // Task handle
        0                           // Core you want to run the task on (0 or 1)
    );
    if(!taskHandle)  {
        printf("\n Error: Cant Create touch Task\n");  
    } else {
        printf("Start Touch Task...\n");
    } 

    buttonCtrl.begin();
    buttonCtrl.setButtonCallBack(buttonAction);
    xTaskCreatePinnedToCore(
    	ButtonCtrl::poolCallBack,   // Function that should be called
        "ButtonCtrl",               // Name of the task (for debugging)
        8*1024,                     // Stack size (bytes)
        &buttonCtrl,                // Parameter to pass
        2,                          // Task priority
		&taskHandle,                // Task handle
        0                           // Core you want to run the task on (0 or 1)
    );
    printf("Start Button Ctrl...\n");

    screan.begin();


    xTaskCreatePinnedToCore(
        BlueFlyVario::poolCallBack, // Function that should be called
        "Baromerer",                // Name of the task (for debugging)
        8*1024,                     // Stack size (bytes)
        &vario,                     // Parameter to pass
        8,                          // Task priority
		&taskHandle,                // Task handle
        0                           // Core you want to run the task on (0 or 1)
    );
    if(!taskHandle)  {
        printf("\n Start Barometer Faild!\n");
    } else {
        printf("Start Barometer...\n");
    }
    printf("Setup Done...\n");
    
}
