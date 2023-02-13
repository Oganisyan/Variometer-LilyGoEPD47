#ifndef _BLUE_FLY_VARIO_
#define _BLUE_FLY_VARIO_
#include "LilyGo-EPD47.h"
#include "MyEpdUtil.h"
#include "KalmanFilter.h"
#include "TinyGPS++.h"
#include "Traking.h"
#include <Stream.h>

#define MAX_CALIB_COUNT 100
class VarioCalculator {

	float 	lastAltitude_;  // letzte gemessen höhe
	float 	vario_;			// Steigung in m/s
	long 	delatT_ = 50;   // in ms, wird berechnet

	long 	calibStartTime_;
	long    calibCount_;

public:
	VarioCalculator() : lastAltitude_(0.F), vario_(0.F), delatT_(19), calibStartTime_(0), calibCount_(MAX_CALIB_COUNT)
	{}

	float calc(float altitude){
		if(calibCount_ >= 0) {
			if(calibCount_ == MAX_CALIB_COUNT) {
				calibStartTime_ = millis();
			} else if(calibCount_ == 0) {
				delatT_ = ((millis() - calibStartTime_)*10 +5 )/10 / MAX_CALIB_COUNT;
			}
			calibCount_-=1;
			return NaN;
		} else {
			vario_ *= 0.9F;
			vario_ += static_cast<double>(altitude - lastAltitude_)*100/delatT_; // 0.1 * 1000 / deltaT
		}
		lastAltitude_ = altitude;
		return vario_;
	}


};



class BlueFlyVario : TinyGPSPlus, public Tracking {
  int rxPin_;
  int txPin_;
  KalmanFilter presureFilter_;
  VarioCalculator varioCalculator_;
  HardwareSerial  stream_;
  
  LiLyGoEPD47 varioData_;
  bool updated_;


  void updateBaro(long pressure100);
  void updateGPS(TinyGPSPlus &gps);

  bool pressureIsOk(double pressure);
  void calcVario();

  void setStartAltitude();
  
  void parsePRS(const char *line, int len);
  void parseGPS(const char *line, int len);
public: 
  BlueFlyVario(int rxPin, int txPin);
  static void poolCallBack(void *parameter);

  void calcQNH(float altitude);
  void setStartAlt(float startAlt) {
	  varioData_.setStartAlt(startAlt);
  }
  void enableVarioConfig(bool enable){
	  varioData_.setVarioConfigEnabled(enable);
  }

  bool switchWiFi() {
	  varioData_.setWifiEnabled(!varioData_.isWifiEnabled());
	  return varioData_.isWifiEnabled();
  }

  bool switchTracking() {
	  varioData_.setTrackingEnabled(!varioData_.isTrackingEnabled());
	  if(varioData_.isTrackingEnabled()) {
		  Tracking::start();
	  } else {
		  Tracking::stop();
	  }
	  return varioData_.isTrackingEnabled();
  }

  Stream &getStream();
  void setSpeaker(uint8_t level);
  bool isUpdated()
  { return updated_; }
  const LiLyGoEPD47  &getVarioData()
  {
	  updated_ = false;
	  return varioData_;
  }
  
};

#endif //_BLUE_FLY_VARIO_
