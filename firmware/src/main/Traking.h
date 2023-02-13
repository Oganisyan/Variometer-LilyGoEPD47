/*
 * Traking.h
 *
 *  Created on: 24.01.2022
 *      Author: martin
 */

#ifndef TRAKING_H_
#define TRAKING_H_
#include "LilyGo-EPD47.h"
#include "MySettings.h"
#include "MyEpdUtil.h"

#include <FFat.h>
#include <esp_rom_md5.h>


class Tracking {
	fs::File file_;
	long calcGWS84(double coordinate);
	TrackingSettings config_;
	long lastBRecordTime_;
	md5_context_t signCtx_;
	void printRecord(const char *p);
public:
	Tracking();
	virtual ~Tracking();
	void addHRecords();
	void addBRecord();
	void addFRecord();
	void addGRecord();

	virtual const LiLyGoEPD47  &getVarioData() = 0;
	void start();
	void stop();
};




class IGCData {
	typedef struct  {
		String  time;
		float	lat;
		float 	lng;
		int		h1;
		int 	h2;
	}	pos_t;


	String pilot ="n/a";
	String pilot2 ="n/a";
	String gliderType ="n/a";
	String gliderId ="n/a";
	String date;
	boolean baroH_;

	pos_t startPos_;
	pos_t endPos_;

	File igc_;
	File kml_;

	char buffer[128];
	char *getPosString(pos_t pos) {
		return format(buffer, "%f, %f, %d", pos.lat, pos.lng, baroH_ ? pos.h2 : pos.h1);
	}


	void parserHRecord(String line){
		int index;
		if((index=line.indexOf("HFDTE"))>=0)
		{
			// ddMMyy
			date = "20";
			date += line.substring(9,11);
			date += ".";
			date += line.substring(7,9);
			date += ".";
			date += line.substring(5,7);
		}else if((index=line.indexOf("HFPLTPILOTINCHARGE:"))>=0){
			pilot = line.substring(index+19);
		}else if((index=line.indexOf("HFCM2CREW2:"))>=0){
			pilot2 = line.substring(index+11);
		}else if((index=line.indexOf("HFGTYGLIDERTYPE:"))>=0){
			gliderType = line.substring(index+16);
		}else if((index=line.indexOf("HFGIDGLIDERID:"))>=0){
			gliderId = line.substring(index+14);
		}
	}

	void parserBecord(String line){
		pos_t pos;
		pos.time = line.substring(1,3);
		pos.time += ":";
		pos.time += line.substring(3,5);
		pos.time += ":";
		pos.time += line.substring(5,7);
		pos.time += " ";
		pos.time += date;

		pos.lat = static_cast<float>(atoi(line.substring(10,15).c_str()));
		pos.lat /= 60000.F;
		pos.lat += static_cast<float>(atoi(line.substring(8,10).c_str()));
		switch(line.charAt(7)){
		default:
		case 'N':
		case 'n':
			break;
		case 'S':
		case 's':
			pos.lat = - pos.lat;
			break;
		}
		pos.lng = static_cast<float>(atoi(line.substring(19,24).c_str()));
		pos.lng /= 60000.F;
		pos.lng += static_cast<float>(atoi(line.substring(16,19).c_str()));
		switch(line.charAt(15)){
		default:
		case 'E':
		case 'e':
			break;
		case 'W':
		case 'w':
			pos.lng = - pos.lng;
			break;
		}
		pos.h1 = atoi(line.substring(25,30).c_str());
		pos.h2 = atoi(line.substring(30,35).c_str());
		if(startPos_.time.isEmpty()) startPos_ = pos;
		endPos_ = pos;
		kml_.print("          "); kml_.println(getPosString(endPos_));
		kml_.flush();
	}


    String line_;
	void printHeader() {
		kml_.println();
		kml_.println("<kml xmlns=\"http://earth.google.com/kml/2.0\">");
		kml_.println("<Folder>");
		kml_.println("  <description><![CDATA[Pilots: "+pilot+", "+pilot2+"<br>Glider: "+gliderType+", "+gliderId+"]]>");
		kml_.println("  </description>");
		kml_.println("  <open>2</open>");
		kml_.println("    <Placemark>");
		kml_.print("      <name>"); kml_.print(igc_.name()); kml_.println("</name>");
		kml_.println("      <Style><LineStyle><color>FF0000FF</color><width>3</width></LineStyle></Style>");
		kml_.println("      <LineString>");
		kml_.println("      <altitudeMode>absolute</altitudeMode>");
		kml_.println("      <tessellate>1</tessellate>");
		kml_.println("        <coordinates>");
	}
	void printFoot() {
		kml_.println("        </coordinates>");
		kml_.println("      </LineString>");
		kml_.println("    </Placemark>");
		kml_.println("    <Placemark>");
		kml_.println("      <name>Start: "+ startPos_.time + "</name>");
		kml_.println("      <Point>");
		kml_.print("        <coordinates>");kml_.print(getPosString(startPos_)); kml_.println(" </coordinates>");
		kml_.println("      </Point>");
		kml_.println("    </Placemark>");
		kml_.println("    <Placemark>");
		kml_.println("      <name>Finish: " + endPos_.time + "</name>");
		kml_.println("      <Point>");
		kml_.print("        <coordinates>");kml_.print(getPosString(endPos_));kml_.println("</coordinates>");
		kml_.println("      </Point>");
		kml_.println("    </Placemark>");
		kml_.println("  </Folder>");
		kml_.println("</kml>");
		kml_.flush();
	}

public:
	IGCData(File igc, File kml) : igc_(igc), kml_(kml) {
		int emptyCount = 0;
		boolean firstBRecord = true;
		for(line_ = igc_.readStringUntil('\n'); igc_.available(); line_ = igc_.readStringUntil('\n')) {
        	if(line_.length()==0) {
        		if((emptyCount++) > 5 ) {
        			break;
        		} else {
        			continue;
        		}
        	}
        	emptyCount = 0;

        	char record_type = line_.charAt(0);
        	switch(record_type)
        	{
        	case 'G':
        	case 'g':
        		break;;
        	case 'B':
        	case 'b':
        		if(firstBRecord) {
        			firstBRecord =! firstBRecord;
        			printHeader();
        		}
        		parserBecord(line_);
        		continue;
        	case 'H':
        	case 'h':
        		parserHRecord(line_);
        		continue;
       		default:
       			continue;
        	}
        }
		if(!firstBRecord) {
			printFoot();
		}
		igc_.close();
		kml_.close();
	}

};

#endif /* TRAKING_H_ */
