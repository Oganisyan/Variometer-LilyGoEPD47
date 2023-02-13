#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#define CONFIG_ASYNC_TCP_USE_WDT 1

#include "MySettings.h"

#include <Arduino.h>
#include <WebServer.h>
#include <IPAddress.h>
#include <FS.h>

void taskDelay(int ms);

class BufferPrinter  : virtual public Print {
	String _content;
public:
	BufferPrinter();
	virtual size_t write(uint8_t c);
	size_t write(const uint8_t *buffer, size_t size);
	void reset();
	bool isEmpty();
	const String &content();
};


class WifiWebServer : WebServer {
	IPAddress address_;
	TaskHandle_t xHandle;
	BufferPrinter printer_;
	Stream& _varioStream;
	WiFiSettings _settings;
	static void callBack(void *);
	void redirect(const String &url);
	File _file;
	WiFiSettings scane();

public:
	static const IPAddress BAD_IP_ADDRESS;

	WifiWebServer(Stream& varioStream);
	IPAddress getAaddress() {
		return address_;
	}
	void begin();
    void end();

	const WiFiSettings& getSettings() const {
		return _settings;
	}
};


#endif //_WEB_SERVER_H_
