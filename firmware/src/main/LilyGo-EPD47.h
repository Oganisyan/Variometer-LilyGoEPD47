#ifndef _VARIO_DATA_H_
#define _VARIO_DATA_H_
#include <Arduino.h>
#include "TinyGPS++.h"


#define GFXfont EpdFont
#define GFXglyph EpdGlyph
#define UnicodeInterval EpdUnicodeInterval
void taskDelay(int ms);

const float NaN = std::numeric_limits<float>::quiet_NaN();

void idf_setup();
void idf_loop();

class LiLyGoEPD47 {
	// Runtime Data
	float batVolt_;
	bool varioConfigEnabled_;
	bool wifiEnabled_;
	bool trackingEnabled_;

	float vario_;
	float startAlt_;
	float pressure_;
	float qnh_;
	long  startTime_;
	TinyGPSLocation location_;
	TinyGPSDate date_;
	TinyGPSTime time_;
	TinyGPSSpeed speed_;
	TinyGPSCourse course_;
	TinyGPSAltitude altGPS_;
	TinyGPSDecimal hdop_;
	TinyGPSInteger satellites_;

	// Config Data
	uint8_t speaker_ = 0; // 0, 25, 50, 75, 100

	float getAltitudeImpl(float pQNH) const {
	    double A = pressure_/pQNH;
	    double B = 0.1902630958088845; //1./5.25588;
	    double C = pow(A,B);
	    C = 1 - C;
	    C = C /0.0000225577;
	    return static_cast<float>(C);
	}

	void calcQNHImpl(float altitude)
	{
	    double A = static_cast<double>(altitude) / 44330. +1.;
	    double C = pow(A,5.25588);
	    C *= pressure_;
	    qnh_ = C;
	}


public:
	LiLyGoEPD47() : batVolt_(0.F), varioConfigEnabled_(false), wifiEnabled_(false),trackingEnabled_(false),
					vario_(0.F), startAlt_(0.F), pressure_(1013.25F), qnh_(1013.25F),startTime_(millis()),
					location_(), date_(), time_(), speed_(), course_(), altGPS_(), hdop_(), satellites_(),
					speaker_(0x00)
	{}

	const TinyGPSAltitude& getAltGps() const {
		return altGPS_;
	}

	void setAltGps(const TinyGPSAltitude &altGps) {
		altGPS_ = altGps;
	}

	float getBatVolt() const {
		return batVolt_;
	}

	void setBatVolt(float batVolt) {
		batVolt_ = batVolt;
	}

	const TinyGPSCourse& getCourse() const {
		return course_;
	}

	void setCourse(const TinyGPSCourse &course) {
		course_ = course;
	}

	const TinyGPSDate& getDate() const {
		return date_;
	}

	void setDate(const TinyGPSDate &date) {
		date_ = date;
	}

	const TinyGPSDecimal& getHdop() const {
		return hdop_;
	}

	void setHdop(const TinyGPSDecimal &hdop) {
		hdop_ = hdop;
	}

	const TinyGPSLocation& getLocation() const {
		return location_;
	}

	void setLocation(const TinyGPSLocation &location) {
		location_ = location;
	}

	float getPressure() const {
		return pressure_;
	}

	void setPressure(float pressure = 1013.25F) {
		pressure_ = pressure;
	}

	float getQnh() const {
		return qnh_;
	}

	void setQnh(float qnh = 1013.25F) {
		qnh_ = qnh;
	}

	const TinyGPSInteger& getSatellites() const {
		return satellites_;
	}

	void setSatellites(const TinyGPSInteger &satellites) {
		satellites_ = satellites;
	}

	uint8_t getSpeaker() const {
		return speaker_;
	}

	void setSpeaker(uint8_t speaker = 0) {
		speaker_ = speaker;
	}

	const TinyGPSSpeed& getSpeed() const {
		return speed_;
	}

	void setSpeed(const TinyGPSSpeed &speed) {
		speed_ = speed;
	}

	float getStartAlt() const {
		return startAlt_;
	}

	void setStartAlt(float startAlt = 0.F) {
		startAlt_ = startAlt;
	}

	long getStartTime() const {
		return startTime_;
	}

	void setStartTime(long startTime = millis()) {
		startTime_ = startTime;
	}

	const TinyGPSTime& getTime() const {
		return time_;
	}

	void setTime(const TinyGPSTime &time) {
		time_ = time;
	}

	bool isTrackingEnabled() const {
		return trackingEnabled_;
	}

	void setTrackingEnabled(bool trackingEnabled) {
		trackingEnabled_ = trackingEnabled;
	}

	float getVario() const {
		return vario_;
	}

	void setVario(float vario = 0.F) {
		vario_ = vario;
	}

	bool isVarioConfigEnabled() const {
		return varioConfigEnabled_;
	}

	void setVarioConfigEnabled(bool varioConfigEnabled) {
		varioConfigEnabled_ = varioConfigEnabled;
	}

	bool isWifiEnabled() const {
		return wifiEnabled_;
	}

	void setWifiEnabled(bool wifiEnabled) {
		wifiEnabled_ = wifiEnabled;
	}

	float getAltitude() const {
			return getAltitudeImpl(qnh_);
	}

	float getAltitude(float pQNH) const {
		return getAltitudeImpl(pQNH);
	}


	float getAltitude2() const {
		return getAltitude() - startAlt_;
	}

	int16_t getAltFL() const {
		return static_cast<int16_t>(getAltitudeImpl(1013.25F) * 3.28F / 100);
	}

	void calcQNH(float altitude) {
		calcQNHImpl(altitude);
	}

};



#endif //_VARIO_DATA_H_
