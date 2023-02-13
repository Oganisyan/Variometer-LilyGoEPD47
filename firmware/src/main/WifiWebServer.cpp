//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include "WifiWebServer.h"
#include "WebUiContetnts.h"
#include "FileDTO.h"


#include <Update.h>
#include <Arduino.h>
#include <WiFi.h>
#include <FFat.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <uri/UriRegex.h>
#include <map>

const IPAddress WifiWebServer::BAD_IP_ADDRESS(0xFF, 0xFF, 0xFF, 0xFF);

#define EP_GET_FILE	"/get-file"
#define EP_DEL_FILE	"/del-file"
#define EP_KML_FILE	"/kml-file"

BufferPrinter::BufferPrinter() : Print(), _content("") {

}

size_t BufferPrinter::write(uint8_t c) {
	::printf("%c", c);
	_content += c;
	return 1;
}

size_t BufferPrinter::write(const uint8_t *buffer, size_t size)
{
	::printf("%s", buffer);
	_content += String(buffer, size);
	return size;
}
void BufferPrinter::reset() {
	_content = "";
}
bool BufferPrinter::isEmpty () {
	return _content.isEmpty();
}

const String& BufferPrinter::content(){
	return _content;
}



WifiWebServer::WifiWebServer(Stream& varioStream) :
		WebServer(80), address_(), xHandle(NULL), printer_(), _varioStream(varioStream), _settings(), _file() {
	address_ = BAD_IP_ADDRESS;
	enableCORS(true);

	// Web Files
	for (int i = 0; i < WEB_UI_FILE_COUNT; i++) {
		if(strcmp(fileNames[i], "/index.html")==0) {
			on(UriRegex("^\\/home(.*)$"), HTTP_GET, [&, i]() {
				sendHeader("Content-Encoding", "gzip");
				send_P(200, mimiTypes[i], (const char*) files[i], fileSizes[i]);
			});
		}

		on(fileNames[i], HTTP_GET, [&, i]() {
			sendHeader("Content-Encoding", "gzip");
			send_P(200, mimiTypes[i], (const char*) files[i], fileSizes[i]);
		});

	}

	// Firmware Update
	on("/api/firmware", HTTP_POST, [&]() {
		sendHeader("Connection", "close");
		if(Update.hasError()) {
			const static String unknown_error = "unknown server error";
			send(500, "text/plain", printer_.isEmpty() ? unknown_error : printer_.content());
		} else {
			send_P(200, "text/plain","OK", 2);
			::taskDelay(100);
			yield();
			::taskDelay(100);
			ESP.restart();
		}
	}
	, [&]() {
		HTTPUpload &up = WifiWebServer::upload();
		if (up.status == UPLOAD_FILE_START) {
			printer_.reset();
			printf("Update Received: %s\n", up.filename.c_str());
			if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) { //start with max available size
				Update.printError(printer_);
			}
		} else if (up.status == UPLOAD_FILE_WRITE) {
			if (Update.write(up.buf, up.currentSize)
					!= up.currentSize) {
				Update.printError(printer_);
			}
		} else if (up.status == UPLOAD_FILE_END) {
			if (Update.end(true)) { //true to set the size to the current progress
				printf("Update Success: %u\nRebooting...\n",
						up.totalSize);
			} else {
				Update.printError(printer_);
			}
		} else {
			printf(
					"Update Failed Unexpectedly (likely broken  connection): status=%d\n",
					up.status);
		}
	});

	// Vario Settings GET
	on("/api/vario", HTTP_GET, [&]() {
		_varioStream.write("$RST*");
		taskDelay(500);
		_varioStream.write("$BST*");
		String BST = "";
		String SET = "";

		for(int i = 0; i < 100; i++) {
			String tmp = _varioStream.readStringUntil('\n');
			if(tmp.startsWith("BST")) {
				BST = tmp;
			} else if(tmp.startsWith("SET")) {
				SET = tmp;
			}
			if(!SET.isEmpty() && !BST.isEmpty()) {
				break;
			}
		}
		if(!SET.isEmpty() && !BST.isEmpty()) {
			String buffer = "";
			SET.trim();
			BST.trim();
			// BUGFIX for SET
			SET = SET.indexOf(' ') > 0 ? SET.substring(SET.indexOf(' ')+1) : SET;
			while(!BST.isEmpty() && !SET.isEmpty()) {
				int bstIndex = BST.indexOf(' ');
				int setIndex = SET.indexOf(' ');
				String b = (bstIndex >= 0) ? BST.substring(0, bstIndex) : BST;
				BST = (bstIndex >= 0) ? BST.substring(bstIndex+1) : "";
				String s = (setIndex >= 0) ? SET.substring(0, setIndex) : SET;
				SET = (setIndex >= 0) ? SET.substring(setIndex+1) : "";

				buffer += buffer.isEmpty() ?  "[{\"type\" : \"" : ", {\"type\" : \"";
				buffer += b;
				buffer += "\", \"value\": ";
				buffer += s;
				buffer += "}";
			}
			buffer += "]";

			send(200, "application/json", buffer);
		} else {
			send(500, "text/plain", "Can't recive Vario Status!");
		}
	});

	// TODO Vario Setting POST
	on(UriRegex("^\\/api\\/vario\\/([A-Z][A-Z][A-Z])\\=([0-9]*)$") , HTTP_GET, [&]() {
		String type = pathArg(0);
		String value = pathArg(1);
		String cmd = "$" + type;
		if(!value.isEmpty())
			cmd += " "+value;
		cmd+= "*\r\n";
		_varioStream.write(cmd.c_str());
		taskDelay(500);
		cmd.trim();
		send(200, "application/json", "{\"status\": \""+cmd +" OK\"}");
	});

	// File Broswer GET
	on(UriRegex("^\\/api\\/flash(.*)$"), HTTP_GET, [&]() {
		FileDTO file = pathArg(0).isEmpty() ? FileDTO("/") : FileDTO(pathArg(0));
		if(file.isDirectory()) {
			String rv = "";
			std::vector<FileDTO> files = FileDTO::files(file);
			for(std::vector<FileDTO>::iterator  f = files.begin(); f != files.end(); f++ ) {
				rv += (rv.isEmpty()) ? "[ " : ", ";
				rv += (*f).json();
			}
			rv += (rv.isEmpty()) ? "[ ]" : " ]";
			send(200, "application/json", rv);
		} else {
			File f = FFat.open(file.getPath(), FILE_READ);
			sendHeader("Content-Type", "text/text");
			sendHeader("Content-Disposition", String("attachment; filename=")+f.name());
	        sendHeader("Connection", "close");
			streamFile(f, "application/octet-stream");
		}
	});

	// File Broswer Delete
	on(UriRegex("^\\/api\\/flash(.*)$"), HTTP_DELETE, [&]() {
		FileDTO file = pathArg(0).isEmpty() ? FileDTO("/") : FileDTO(pathArg(0));
		if(file && !file.isDirectory() && FFat.remove(file.getPath())) {
			send(204, "text/plain","");
		} else {
			send(500, "text/plain", "Delete failed(unknown error)!");
		}
	});


	// File Broswer Upload
	on(UriRegex("^\\/api\\/flash(.*)$"), HTTP_POST, [&]() {
		sendHeader("Connection", "close");
		if (_file) {
			_file.close();
			send_P(200, "text/plain","OK", 2);
		} else {
			const static String unknown_error = "unknown server error";
			send(500, "text/plain", printer_.isEmpty() ? unknown_error : printer_.content());
		}
	}
	, [&]() {
		HTTPUpload &up = WifiWebServer::upload();
		if (up.status == UPLOAD_FILE_START) {
			printf("path %s  %s\n",pathArg(0).c_str(), up.filename.c_str());
		    String filename = pathArg(0) + "/" +up.filename;
			printf("open file %s\n",filename.c_str());
		    _file = FFat.open(filename, FILE_WRITE);
			if (!_file) { //start with max available size
				printf("can't open file %s\n",filename.c_str());
			}
		} else if (up.status == UPLOAD_FILE_WRITE) {
			if (_file.write(up.buf, up.currentSize) != up.currentSize) {
				printf("can't write size %d\n", up.currentSize);
			}
		} else if (up.status == UPLOAD_FILE_END) {
			if (_file) {
				printf("Upload OK \n");
			}
		} else {
			printf(
					"Update Failed Unexpectedly (likely broken  connection): status=%d\n",
					up.status);
		}
	});

	onNotFound([&]() {
		return redirect("/index.html");
	});
}

void WifiWebServer::redirect(const String &url) {
	sendHeader("Location", url);
	send(302);
}

void WifiWebServer::end() {
	address_ = BAD_IP_ADDRESS;
	WiFi.mode((wifi_mode_t)WIFI_MODE_NULL);
	WebServer::stop();
}

void WifiWebServer::callBack(void *p) {
	WifiWebServer *myThis = static_cast<WifiWebServer*>(p);
	for (;;) {
		myThis->handleClient();
		::taskDelay(100);
	}
}

WiFiSettings WifiWebServer::scane() {
	WiFiSettings rv;

    int n = WiFi.scanNetworks();
    printf("scan done\n");
    if (n == 0) {
        printf("no networks found\n");
    } else {
    	std::map<String, int8_t> wifi;
        for (int i = 0; i < n; ++i) {
        	if(WiFi.RSSI(i) > -85)
        		wifi[WiFi.SSID(i)] = WiFi.RSSI(i);
            delay(10);
        }
        std::vector<WiFiSettings> list = WiFiSettings::list();
        for(int i = 0; i < list.size(); i++) {
        	if(list[i].getPrio() > rv.getPrio()) {
        		if(list[i].isSoftIP() || wifi.find(list[i].getSsid()) != wifi.end()){
        			rv = list[i];
        		}
        	} else if(list[i].getPrio() == rv.getPrio()) {
        		if(!list[i].isSoftIP() && !rv.isSoftIP() && wifi.find(list[i].getSsid()) != wifi.end() &&
        			(wifi.find(rv.getSsid()) == wifi.end() || wifi.find(rv.getSsid())->second < wifi.find(list[i].getSsid())->second)) {
        			rv = list[i];
        		}
        	}
        }
    }
    printf("WiFi: %s pwd: %s isSoftIP: %d Prio: %d\n", rv.getSsid(), rv.getPass(), rv.isSoftIP(), rv.getPrio());
    return rv;
}

void WifiWebServer::begin() {

	WiFi.mode((wifi_mode_t)WIFI_MODE_STA);
	_settings = scane();

	if(_settings.isSoftIP()) {
	    if(!WiFi.softAP(_settings.getSsid(), _settings.getPass())) {
	    	address_ = BAD_IP_ADDRESS;
	        printf("WiFi Failed!\n");
	    	return;
	    }
	    address_ = WiFi.softAPIP();
	} else {
		WiFi.begin(_settings.getSsid(), _settings.getPass());
		if (WiFi.waitForConnectResult() != WL_CONNECTED) {
			address_ = BAD_IP_ADDRESS;
			printf("WiFi Failed!\n");
			return;
		}
		address_ = WiFi.localIP();
	}
	printf("WiFi(%s%s) IP Address: %s\n",
			_settings.getSsid(),
			_settings.isSoftIP() ? "*": "",
			address_.toString().c_str());

	WebServer::begin();
	xTaskCreatePinnedToCore(WifiWebServer::callBack, // Function that should be called
			"WifiWebServer",       		// Name of the task (for debugging)
			8 * 1024,             		// Stack size (bytes)
			this,               		// Parameter to pass
			5,                  		// Task priority
			&xHandle,           		// Task handle
			0                   	// Core you want to run the task on (0 or 1)
			);
	if (!xHandle) {
		printf("\n Error: Cant WebServer Task\n");
	} else {
		printf("Start WebServer...\n");
	}

}

