#include "BlueFlyVario.h"
#include <unistd.h>


void BlueFlyVario::poolCallBack(void *parameter) {
    BlueFlyVario *mythis = static_cast<BlueFlyVario*>(parameter);
    mythis->stream_.begin(57600, SERIAL_8N1, mythis->rxPin_, mythis->txPin_);
    for(;;) {
        while (mythis->stream_.available()) {
			String line = mythis->stream_.readStringUntil('\n');
			int startPrs = line.indexOf(String("PRS "));
			if(startPrs >= 0) {
				mythis->parsePRS(line.c_str()+startPrs, line.length() - startPrs);
				line.remove(startPrs, 10);
			}
        	if(line.length() > 0 ) {
        		mythis->parseGPS(line.c_str(), line.length());
        	}
        }
        if(mythis->varioData_.isVarioConfigEnabled()) {
        	char c;
        	while(read(STDIN_FILENO, &c, sizeof(c)) > 0) {
        		mythis->stream_.write(c);
        	}
        }

        taskDelay(50);
    }
}


void BlueFlyVario::parsePRS(const char *line, int len) {
	if(len > 3  && strncmp(line, "PRS", 3)==0){
        char *endptr;
        long pressure100 = strtol(line+4, &endptr, 16);
        updateBaro(pressure100);
	}
}

void BlueFlyVario::parseGPS(const char *line, int len) {
	for(int i = 0; i < len; i++) {
		TinyGPSPlus::encode(*(line+i));
	}
	TinyGPSPlus::encode('\r');
	TinyGPSPlus::encode('\n');
	updateGPS(*this);
	if(TinyGPSPlus::location.isUpdated()) {
		TinyGPSPlus::location.lat();
		Tracking::addBRecord();
	}
}


void BlueFlyVario::updateGPS( TinyGPSPlus &gps) {
	updated_ = true;
	if(gps.location.isValid()) varioData_.setLocation(gps.location);
	if(gps.altitude.isValid()) varioData_.setAltGps(gps.altitude);
	if(gps.speed.isValid()) varioData_.setSpeed(gps.speed);
	if(gps.date.isValid()) varioData_.setDate(gps.date);
	if(gps.time.isValid()) varioData_.setTime(gps.time);
	if(gps.course.isValid()) varioData_.setCourse(gps.course);
	if(gps.speed.isValid())  varioData_.setSpeed(gps.speed);
	if(gps.hdop.isValid())  varioData_.setHdop(gps.hdop);
	if(gps.satellites.isValid())  varioData_.setSatellites(gps.satellites);
}


void BlueFlyVario::updateBaro(long pressure100) {
	updated_ = true;
    double pressureRaw = static_cast<double>(pressure100)/ 100.;
    if(pressureIsOk(pressureRaw)) {
        presureFilter_.update(pressureRaw);
        varioData_.setPressure( presureFilter_.get());
        calcVario();
    }
}

Stream &BlueFlyVario::getStream() {
  return stream_;
}


void BlueFlyVario::setSpeaker(uint8_t level) {
	  varioData_.setSpeaker(level);
	  char buffer[16];
	  level /=25;
	  switch(level) {
	  default:
	  case 0:
		  sprintf(buffer, "$BVL %d*\r\n", 1);
		  break;
	  case 1:
		  sprintf(buffer, "$BVL %d*\r\n", 25);
		  break;
	  case 2:
		  sprintf(buffer, "$BVL %d*\r\n", 50);
		  break;
	  case 3:
		  sprintf(buffer, "$BVL %d*\r\n", 150);
		  break;
	  case 4:
		  sprintf(buffer, "$BVL %d*\r\n", 1000);
		  break;
	  }
	  stream_.write(buffer);
}

BlueFlyVario::BlueFlyVario(int rxPin, int txPin) : TinyGPSPlus(), Tracking(),
		rxPin_(rxPin), txPin_(txPin), presureFilter_(), varioCalculator_(), stream_(2), varioData_(), updated_(false)
{}


bool BlueFlyVario::pressureIsOk(double pressure) 
{ return pressure > 300. && pressure < 1250.; }


void BlueFlyVario::calcVario() 
{ varioData_.setVario(varioCalculator_.calc(varioData_.getAltitude())); }

void BlueFlyVario::setStartAltitude()
{
	updated_ = true;
	varioData_.setStartAlt(varioData_.getAltitude());
} 


void BlueFlyVario::calcQNH(float altitude)
{
	updated_ = true;
	varioData_.calcQNH(altitude);
}

