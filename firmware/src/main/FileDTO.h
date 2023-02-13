/*
 * FileDTO.h
 *
 *  Created on: Mar 5, 2022
 *      Author: martin
 */

#ifndef MAIN_FILEDTO_H_
#define MAIN_FILEDTO_H_
#include <Arduino.h>
#include <FS.h>
#include <vector>



class FileDTO {
	boolean _good;
	boolean _directory;
	uint32_t _size;
	String _name;
	String _path;

public:
	FileDTO();
	FileDTO(const String &path);
	virtual ~FileDTO();

	boolean isDirectory() const {
		return _directory;
	}

	const String& getName() const {
		return _name;
	}

	const String& getPath() const {
		return _path;
	}

	uint32_t getSize() const {
		return _size;
	}

    operator bool() const{
		return _good;
    }

    String json() {
    	String rc = "{ \"good\" :";
    	rc += (_good) ? "true" : "false";
    	rc += ", \"directory\" :";
    	rc += (_directory) ? "true" : "false";
    	rc += ", \"size\" :";
    	rc += String(_size);
    	rc += ", \"name\" : \"" + _name + "\"";
    	rc += ", \"path\" : \"" + _path + "\"}";
    	return rc;
    }

	static std::vector<FileDTO> files(const FileDTO &folder);


};

#endif /* MAIN_FILEDTO_H_ */
