/*
 * DeviceSettings.h
 *
 *  Created on: Mar 14, 2022
 *      Author: martin
 */

#ifndef MAIN_SETTINGS_H_
#define MAIN_SETTINGS_H_
#include <Arduino.h>
#include <vector>

#define WIFI_FILE "/config/wifi.json"
#define TRACKING_FILE "/config/tracking.json"
#define APP_VERSION	"0.0.0"
#define APP_NAME	"MeloneFly"

class WiFiSettings {
	String  _ssid;
	String  _pass;
	bool    _softIP;
	uint8_t _prio;
	static void read(std::vector<WiFiSettings> &list);
	static void write(const std::vector<WiFiSettings> &list);
public:
	WiFiSettings();
	static std::vector<WiFiSettings> list();
	const char* getPass() const;
	uint8_t getPrio() const;
	bool isSoftIP() const;
	const char* getSsid() const;
};


class TrackingSettings {
	String _manufactureId;
	String _pilot;
	String _pilot2;
	String _gliderType;
	String _gliderId;
	String _dtmGpsDatum;
	String _firmwareVersion;
	String _hardwareVersion;
	String _ftyFrtype;
	String _gpsDescriptor;
	String _prsSensor;
	String _frsSecurity;
	String _appNameVersion;
	long _inttervalBRecord;

public:
	TrackingSettings();
	virtual ~TrackingSettings();
	const char* getAppNameVersion() const;
	const char* getDtmGpsDatum() const;
	const char* getFirmwareVersion() const;
	const char* getFrsSecurity() const;
	const char* getFtyFrtype() const;
	const char* getGliderId() const;
	const char* getGliderType() const;
	const char* getGpsDescriptor() const;
	const char* getHardwareVersion() const;
	long getInttervalBRecord() const;
	const char* getManufactureId() const;
	const char* getPilot() const;
	const char* getPilot2() const;
	const char* getPrsSensor() const;
	void read();
	void write();
	void init();
};

#endif /* MAIN_SETTINGS_H_ */
