/*
 * Traking.cpp
 *
 *  Created on: 24.01.2022
 *      Author: martin
 */

#include "Traking.h"


#include <stddef.h>
#include <stdio.h>
#include <cmath>
#include <cstdint>

#include "MyEpdUtil.h"
#include "TinyGPS++.h"






Tracking::Tracking() : file_(), config_(), lastBRecordTime_(0), signCtx_() {

}

Tracking::~Tracking(){

}

void Tracking::start() {
	config_.init();
	char buffer[64];
	LiLyGoEPD47  data = getVarioData();
	for(int i=0; i < 100; i++ ) {
		char *fname = format(buffer, "/tracking/log%04d%02d%02d%02d.igc", data.getDate().year(), data.getDate().month(), data.getDate().day(),i);
		if(!FFat.exists(fname)) {
			printf("Create %s\n", fname);
			file_ = FFat.open(fname, FILE_WRITE);
			esp_rom_md5_init(&signCtx_);
			addHRecords();
			return;
		}

	}
}

void Tracking::stop() {
	addGRecord();
	file_.close();
}



long Tracking::calcGWS84(double coordinate) {
	coordinate = abs(coordinate);
	double degrees = static_cast<int>(coordinate);
	double mins = (coordinate - degrees)*(60.0/100.0);
	return static_cast<long>((degrees+mins) * 1.e5 + 0.5);
}

void Tracking::printRecord(const char *record) {
	esp_rom_md5_update(&signCtx_, reinterpret_cast<unsigned char const *>(record), strlen(record));
	file_.print(record);
	file_.print("\n");
}


void Tracking::addGRecord() {
	char tmp[34]={0};
	if(file_.available()) {
		unsigned char buffer[16] = {0};
		esp_rom_md5_final(buffer, &signCtx_);
		file_.print(
			format(tmp, "G%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				buffer[ 0],buffer[ 1],buffer[ 2],buffer[ 3],buffer[ 4],buffer[ 5],buffer[ 6],buffer[ 7],
				buffer[ 8],buffer[ 9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15]));
	}
}


void Tracking::addHRecords() {
	char buffer[256];
	LiLyGoEPD47  data = getVarioData();
	if(file_.available() &&  data.getDate().isValid()) {
		printRecord(format(buffer, "A%s %s", config_.getManufactureId(), config_.getAppNameVersion()));
		printRecord(format(buffer, "HFDTE%02d%02d%02d", data.getDate().day(), data.getDate().month(), data.getDate().year()%100));
		if(data.getHdop().isValid())
			printRecord(format(buffer, "HFFXA%03d", 5*data.getHdop().value()/100));
		printRecord(format(buffer, "HFPLTPILOTINCHARGE: %s", config_.getPilot()));
		printRecord(format(buffer, "HFCM2CREW2: %s", config_.getPilot2()));
		printRecord(format(buffer, "HFGTYGLIDERTYPE: %s", config_.getGliderType()));
		printRecord(format(buffer, "HFGIDGLIDERID: %s", config_.getGliderId()));
		printRecord(format(buffer, "HFDTM100GPSDATUM: %s", config_.getDtmGpsDatum()));
		printRecord(format(buffer, "HFRFWFIRMWAREVERSION:%s", config_.getFirmwareVersion()));
		printRecord(format(buffer, "HFRHWHARDWAREVERSION:%s", config_.getHardwareVersion()));
		printRecord(format(buffer, "HFFTYFRTYPE:%s", config_.getFtyFrtype()));
		printRecord(format(buffer, "HFGPS:%s", config_.getGpsDescriptor()));
		printRecord(format(buffer, "HFPRSPRESSALTSENSOR:%s", config_.getPrsSensor()));
	} else {
		printf("File aviable %d, Date isValid %d\n", file_.available(), data.getDate().isValid());
	}
}


void Tracking::addBRecord()
{
	LiLyGoEPD47  data = getVarioData();

	if(file_.available() && getVarioData().getLocation().isValid() && getVarioData().getTime().isValid()
			&& millis() >= (lastBRecordTime_ + config_.getInttervalBRecord())) {
		lastBRecordTime_ = millis() ;
		char line[256];
		TinyGPSLocation location = data.getLocation();
		double lat = location.lat();
		double lng = location.lng();
		int  height = static_cast<int>(data.getAltitude(1013.25F));
		int  gpsHeight = data.getAltGps().isValid() ?  static_cast<int>(data.getAltGps().meters()) : 0;
		sprintf( line,
				"B%02d%02d%02d"   															// B + Time
				"%07ld%c"																	// Latitude
				"%08ld%c"																	// Longitude
				"A%05d%05d"                                                 				// Altitude Baro Gps
				"%03d"                                                      				// Accuracy
				"%02d",                                                     				// Sat-Count
				data.getTime().hour(), data.getTime().minute(), data.getTime().second(), 	// Time
				calcGWS84(lat), lat >= 0. ? 'N':'S',  										// Latitude
				calcGWS84(lng), lng >= 0. ? 'E':'W',										// Longitude
				height, gpsHeight,															// Altitude Baro Gps
				5*data.getHdop().value()/100,												// Accuracy
				data.getSatellites().value()                                     			// Sat-Count
		);
		printRecord(line);
	}
}




void Tracking::addFRecord()
{
	// TODO
}
