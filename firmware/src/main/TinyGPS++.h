/*
 TinyGPS++ - a small GPS library for Arduino providing universal NMEA parsing
 Based on work by and "distanceBetween" and "courseTo" courtesy of Maarten Lamers.
 Suggestion to add satellites, courseTo(), and cardinal() by Matt Monson.
 Location precision improvements suggested by Wayne Holder.
 Copyright (C) 2008-2013 Mikal Hart
 All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __TinyGPSPlus_h
#define __TinyGPSPlus_h

#include "Arduino.h"
#include <limits.h>
#include <algorithm>
#include <string.h>


#define _GPS_VERSION "0.92" // software version of this library
#define _GPS_MPH_PER_KNOT 1.15077945
#define _GPS_MPS_PER_KNOT 0.51444444
#define _GPS_KMPH_PER_KNOT 1.852
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001
#define _GPS_FEET_PER_METER 3.2808399
#define _GPS_MAX_FIELD_SIZE 15

#define GxGGA_TERMS "GPGGA,GLGGA,GAGGA,GNGGA"
#define GxRMC_TERMS "GPRMC,GLRMC,GARMC,GNRMC"
#define GxGSV_TERMS "GPGSV,GLGSV,GAGSV,GNGSV"

enum class GpsSourceType {
	GPS, Galileo, GLONASS, GNSS, Unknown
};

struct RawDegrees {
	uint16_t deg;
	uint32_t billionths;
	bool negative;
public:
	RawDegrees() :
			deg(0), billionths(0), negative(false) {
	}
};

struct TinyGPSLocation {
	friend class TinyGPSPlus;
public:
	bool isValid() const {
		return valid;
	}
	bool isUpdated() const {
		return updated;
	}
	uint32_t age() const {
		return valid ? millis() - lastCommitTime : (uint32_t) ULONG_MAX;
	}
	const RawDegrees& rawLat() {
		updated = false;
		return rawLatData;
	}
	const RawDegrees& rawLng() {
		updated = false;
		return rawLngData;
	}
	double lat();
	double lng();

	TinyGPSLocation() :
			valid(false), updated(false), lastCommitTime(millis()) {
	}

private:
	bool valid, updated;
	RawDegrees rawLatData, rawLngData, rawNewLatData, rawNewLngData;
	uint32_t lastCommitTime;
	void commit();
	void setLatitude(const char *term);
	void setLongitude(const char *term);
};

struct TinyGPSDate {
	friend class TinyGPSPlus;
public:
	bool isValid() const {
		return valid;
	}
	uint32_t age() const {
		return valid ? millis() - lastCommitTime : (uint32_t) ULONG_MAX;
	}
	uint32_t value() const {
		return date;
	}
	uint16_t year() const;
	uint8_t month() const;
	uint8_t day() const;

	TinyGPSDate() :
			valid(false), date(0), newDate(0), lastCommitTime(millis()) {
	}

private:
	bool valid;
	uint32_t date, newDate;
	uint32_t lastCommitTime;
	void commit();
	void setDate(const char *term);
};

struct TinyGPSTime {
	friend class TinyGPSPlus;
public:
	bool isValid() const {
		return valid;
	}

	uint32_t age() const {
		return valid ? millis() - lastCommitTime : (uint32_t) ULONG_MAX;
	}

	uint32_t value() const {
		return time;
	}
	uint8_t hour() const;
	uint8_t minute() const;
	uint8_t second() const;
	uint8_t centisecond() const;

	TinyGPSTime() :
			valid(false), time(0), newTime(0), lastCommitTime(millis()) {
	}

private:
	bool valid;
	uint32_t time, newTime;
	uint32_t lastCommitTime;
	void commit();
	void setTime(const char *term);
};


struct TinyGPSSV {
	friend class TinyGPSPlus;
public:
	bool isValid() const {
		return valid;
	}

	uint32_t age() const {
		return valid ? millis() - lastCommitTime : (uint32_t) ULONG_MAX;
	}

	uint32_t value() const {
		return time;
	}
	uint8_t hour() const;
	uint8_t minute() const;
	uint8_t second() const;
	uint8_t centisecond() const;

	TinyGPSSV() :
			valid(false), lastCommitTime(millis()) , time(0), newTime(0) {
	}

private:
	bool valid;
	uint32_t lastCommitTime;
	uint32_t time, newTime;
	void commit();
	void setTime(const char *term);
};


struct TinyGPSDecimal {
	friend class TinyGPSPlus;
public:
	bool isValid() const {
		return valid;
	}
	uint32_t age() const {
		return valid ? millis() - lastCommitTime : (uint32_t) ULONG_MAX;
	}
	int32_t value() const {
		return val;
	}

	TinyGPSDecimal() :
			valid(false), lastCommitTime(millis()), val(0), newval(0) {
	}

private:
	bool valid;
	uint32_t lastCommitTime;
	int32_t val, newval;
	void commit();
	void set(const char *term);
};

struct TinyGPSInteger {
	friend class TinyGPSPlus;
public:
	bool isValid() const {
		return valid;
	}
	uint32_t age() const {
		return valid ? millis() - lastCommitTime : (uint32_t) ULONG_MAX;
	}
	uint32_t value() const {
		return val;
	}

	TinyGPSInteger() :
			valid(false), lastCommitTime(millis()), val(0), newval(0) {
	}

private:
	bool valid;
	uint32_t lastCommitTime;
	uint32_t val, newval;
	void commit();
	void set(const char *term);
};

struct TinyGPSSpeed: TinyGPSDecimal {
	double knots() const {
		return value() / 100.0;
	}
	double mph() const {
		return _GPS_MPH_PER_KNOT * value() / 100.0;
	}
	double mps() const {
		return _GPS_MPS_PER_KNOT * value() / 100.0;
	}
	double kmph() const {
		return _GPS_KMPH_PER_KNOT * value() / 100.0;
	}
};

struct TinyGPSCourse: public TinyGPSDecimal {
	double deg() const {
		return value() / 100.0;
	}
};

struct TinyGPSAltitude: TinyGPSDecimal {
	double meters() const {
		return static_cast<double>(value()) / 100.0;
	}
	double miles() const {
		return _GPS_MILES_PER_METER * value() / 100.0;
	}
	double kilometers() const {
		return _GPS_KM_PER_METER * value() / 100.0;
	}
	double feet() const {
		return _GPS_FEET_PER_METER * value() / 100.0;
	}
};

class TinyGPSPlus;
class TinyGPSCustom {
public:
	TinyGPSCustom(TinyGPSPlus &gps, const char *sentenceName, int termNumber);
	void begin(TinyGPSPlus &gps, const char *_sentenceName, int _termNumber);

	bool isValid() const {
		return valid;
	}
	uint32_t age() const {
		return valid ? millis() - lastCommitTime : (uint32_t) ULONG_MAX;
	}
	const char* value() const {
		return buffer;
	}

private:
	void commit();
	void set(const char *term);

	char stagingBuffer[_GPS_MAX_FIELD_SIZE + 1];
	char buffer[_GPS_MAX_FIELD_SIZE + 1];
	unsigned long lastCommitTime;
	bool valid;
	const char *sentenceName;
	int termNumber;
	friend class TinyGPSPlus;
	TinyGPSCustom *next;
};

class TinyGPSPlus {

	inline GpsSourceType getSourceType(const char *term, const char *types) {
		char tmp[6] = {0};
		strncpy(tmp, term, std::min<size_t>(5, strlen(term)));
		const char *rv =strstr(types, tmp);
		int pos = (rv == NULL) ? -1 : static_cast<int>(rv - types);
		switch(pos)
		{
		case 0:  return GpsSourceType::GPS;
		case 6:  return GpsSourceType::Galileo;
		case 12: return GpsSourceType::GLONASS;
		case 18: return GpsSourceType::GNSS;
		default: break;
		}
		return GpsSourceType::Unknown;

	}
	inline GpsSourceType getSourceTypeGxGGA(const char *term) {
		return getSourceType(term, GxGGA_TERMS);
	}
	inline GpsSourceType getSourceTypeGxRMC(const char *term) {
		return getSourceType(term, GxRMC_TERMS);
	}
	inline GpsSourceType getSourceTypeGxGSV(const char *term) {
		return getSourceType(term, GxGSV_TERMS);
	}

public:
	TinyGPSPlus();
	bool encode(char c); // process one character received from GPS
	TinyGPSPlus& operator <<(char c) {
		encode(c);
		return *this;
	}

	TinyGPSLocation location;
	TinyGPSDate date;
	TinyGPSTime time;
	TinyGPSSpeed speed;
	TinyGPSCourse course;
	TinyGPSAltitude altitude;
	TinyGPSInteger satellites;
	TinyGPSDecimal hdop;

	static const char* libraryVersion() {
		return _GPS_VERSION;
	}

	static double distanceBetween(double lat1, double long1, double lat2,
			double long2);
	static double courseTo(double lat1, double long1, double lat2,
			double long2);
	static const char* cardinal(double course);

	static int32_t parseDecimal(const char *term);
	static void parseDegrees(const char *term, RawDegrees &deg);

	uint32_t charsProcessed() const {
		return encodedCharCount;
	}
	uint32_t sentencesWithFix() const {
		return sentencesWithFixCount;
	}
	uint32_t failedChecksum() const {
		return failedChecksumCount;
	}
	uint32_t passedChecksum() const {
		return passedChecksumCount;
	}

private:
	enum {
		GPS_SENTENCE_GxGGA, GPS_SENTENCE_GxRMC, GPS_SENTENCE_OTHER
	};

	// parsing state variables
	uint8_t parity;
	bool isChecksumTerm;
	char term[_GPS_MAX_FIELD_SIZE];
	uint8_t curSentenceType;
	uint8_t curTermNumber;
	uint8_t curTermOffset;
	bool sentenceHasFix;

	// custom element support
	friend class TinyGPSCustom;
	TinyGPSCustom *customElts;
	TinyGPSCustom *customCandidates;
	void insertCustom(TinyGPSCustom *pElt, const char *sentenceName, int index);

	// statistics
	uint32_t encodedCharCount;
	uint32_t sentencesWithFixCount;
	uint32_t failedChecksumCount;
	uint32_t passedChecksumCount;

	// internal utilities
	int fromHex(char a);
	bool endOfTermHandler();
};

#endif // def(__TinyGPSPlus_h)
