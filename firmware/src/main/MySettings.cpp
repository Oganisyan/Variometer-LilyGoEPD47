/*
 * DeviceSettings.cpp
 *
 *  Created on: Mar 14, 2022
 *      Author: martin
 */

#include "MySettings.h"
#include <cJSON.h>
#include <FFat.h>

WiFiSettings::WiFiSettings() : _ssid("MeloneFly"), _pass("12345678"), _softIP(true), _prio(0)
{}

const char* WiFiSettings::getPass() const {
	return _pass.c_str();
}

uint8_t WiFiSettings::getPrio() const {
	return _prio;
}

bool WiFiSettings::isSoftIP() const {
	return _softIP;
}

const char* WiFiSettings::getSsid() const {
	return _ssid.c_str();
}


std::vector<WiFiSettings> WiFiSettings::list() {
	std::vector<WiFiSettings> rv;
	WiFiSettings item;
	rv.push_back(item);
	if(FFat.exists(WIFI_FILE))
		read(rv);
	else
		write(rv);
	return rv;
}

void WiFiSettings::write(const std::vector<WiFiSettings> &list){
	printf("Open %s for write\n", WIFI_FILE);
	File file = ::FFat.open(WIFI_FILE, FILE_WRITE);
	if(file) {
		cJSON *root = cJSON_CreateArray();
		for(int i = 0; i < list.size(); i++) {
			cJSON *item = cJSON_CreateObject();
			cJSON_AddItemToObject(item, "ssid",   cJSON_CreateString(list[i].getSsid()));
			cJSON_AddItemToObject(item, "pass",   cJSON_CreateString(list[i].getPass()));
			cJSON_AddItemToObject(item, "softIP", cJSON_CreateNumber(list[i].isSoftIP()? 1 : 0));
			cJSON_AddItemToObject(item, "prio",   cJSON_CreateNumber(list[i].getPrio()));
			cJSON_AddItemToArray(root, item);

		}
		char *rendered=cJSON_Print(root);
		printf("%s\n",rendered);
		file.write((uint8_t *)rendered, strlen(rendered));
		file.close();
		cJSON_Delete(root);
	} else {
		printf("Can`t Open\n");
	}

}

void WiFiSettings::read(std::vector<WiFiSettings> &list){
	File file = FFat.open(WIFI_FILE, FILE_READ);
	if(file) {
		String content = "";
		char buffer[129];
		size_t len;
		while((len = file.read((uint8_t *) buffer, 128)) > 0 && (len  <= 128)) {
			buffer[len] = 0;
			content += buffer;
		}

		cJSON *root = cJSON_Parse(content.c_str());
		cJSON *elem;
		int n = cJSON_GetArraySize(root);
		WiFiSettings item;
		for (int i = 0; i < n; i++) {
		    elem = cJSON_GetArrayItem(root, i);
		    item._ssid = cJSON_GetObjectItem(elem, "ssid")->valuestring;
		    item._pass = cJSON_GetObjectItem(elem, "pass")->valuestring;
		    item._softIP = (cJSON_GetObjectItem(elem, "softIP")->valueint != 0);
		    item._prio = cJSON_GetObjectItem(elem, "prio")->valueint;
			list.push_back(item);
		}
		file.close();
		cJSON_Delete(root);
		printf("Read return True");
	}

}



TrackingSettings::TrackingSettings() :
	_manufactureId(APP_NAME),
	_pilot("n/a"),
	_pilot2("n/a"),
	_gliderType("n/a"),
	_gliderId("n/a"),
	_dtmGpsDatum("WGS-1984"),
	_firmwareVersion(APP_VERSION),
	_hardwareVersion(APP_VERSION),
	_ftyFrtype("n/a"),
	_gpsDescriptor(APP_NAME),
	_prsSensor("MS5611"),
	_frsSecurity("n/a"),
	_appNameVersion(APP_VERSION),
	_inttervalBRecord(3000) // 3 sek
{
}




void TrackingSettings::init() {
	if(FFat.exists(TRACKING_FILE))
		read();
	else
		write();
}

void TrackingSettings::read() {
	File file = FFat.open(TRACKING_FILE, FILE_READ);
	if(file) {
		String content = "";
		char buffer[129];
		size_t len;
		while((len = file.read((uint8_t *) buffer, 128)) > 0 && (len  <= 128)) {
			buffer[len] = 0;
			content += buffer;
		}
		cJSON *root = cJSON_Parse(content.c_str());
		_pilot = cJSON_GetObjectItem(root,"pilot")->valuestring;
		_pilot2 = cJSON_GetObjectItem(root,"pilot2")->valuestring;
		_gliderType = cJSON_GetObjectItem(root,"gliderType")->valuestring;
		_gliderId = cJSON_GetObjectItem(root,"gliderId")->valuestring;
		_inttervalBRecord = cJSON_GetObjectItem(root,"inttervalBRecord")->valueint;

		file.close();
		cJSON_Delete(root);
		printf("Read return True");
	}

}

void TrackingSettings::write(){
	printf("Open %s for write\n", TRACKING_FILE);
	File file = ::FFat.open(TRACKING_FILE, FILE_WRITE);
	if(file) {
		cJSON *root;
		root=cJSON_CreateObject();
		cJSON_AddItemToObject(root, "pilot", cJSON_CreateString(getPilot()));
		cJSON_AddItemToObject(root, "pilot2", cJSON_CreateString(getPilot2()));
		cJSON_AddItemToObject(root, "gliderType", cJSON_CreateString(getGliderType()));
		cJSON_AddItemToObject(root, "gliderId", cJSON_CreateString(getGliderId()));
		cJSON_AddItemToObject(root, "inttervalBRecord", cJSON_CreateNumber(getInttervalBRecord()));
		char *rendered=cJSON_Print(root);
		printf("%s\n",rendered);
		file.write((uint8_t *)rendered, strlen(rendered));
		file.close();
		cJSON_Delete(root);
	} else {
		printf("Can`t Open\n");
	}

}


TrackingSettings::~TrackingSettings() {
}

const char* TrackingSettings::getAppNameVersion() const {
	return _appNameVersion.c_str();
}

const char* TrackingSettings::getDtmGpsDatum() const {
	return _dtmGpsDatum.c_str();
}

const char* TrackingSettings::getFirmwareVersion() const {
	return _firmwareVersion.c_str();
}

const char* TrackingSettings::getFrsSecurity() const {
	return _frsSecurity.c_str();
}

const char* TrackingSettings::getFtyFrtype() const {
	return _ftyFrtype.c_str();
}

const char* TrackingSettings::getGliderId() const {
	return _gliderId.c_str();
}

const char* TrackingSettings::getGliderType() const {
	return _gliderType.c_str();
}

const char* TrackingSettings::getGpsDescriptor() const {
	return _gpsDescriptor.c_str();
}

const char* TrackingSettings::getHardwareVersion() const {
	return _hardwareVersion.c_str();
}

long TrackingSettings::getInttervalBRecord() const {
	return _inttervalBRecord;
}

const char* TrackingSettings::getManufactureId() const {
	return _manufactureId.c_str();
}

const char* TrackingSettings::getPilot() const {
	return _pilot.c_str();
}

const char* TrackingSettings::getPilot2() const {
	return _pilot2.c_str();
}

const char* TrackingSettings::getPrsSensor() const {
	return _prsSensor.c_str();
}
