/*
 * FileDTO.cpp
 *
 *  Created on: Mar 5, 2022
 *      Author: martin
 */

#include "FileDTO.h"
#include <FFat.h>

FileDTO::FileDTO(): _good(false), _directory(false), _size(0), _name(), _path() {

}

FileDTO::FileDTO(const String &path): _good(false), _directory(false), _size(0), _name(), _path() {
	File file = FFat.open(path, FILE_READ);
	_good = file ? true : false;
	_directory = file.isDirectory();
	_size = file.size();
	_name = file.name();
	_path = file.path();
}


FileDTO::~FileDTO() {
}

std::vector<FileDTO> FileDTO::files(const FileDTO &folderDTO) {
	std::vector<FileDTO> rv;
	File folder = FFat.open(folderDTO.getPath(), FILE_READ);
	if (!folder || !folder.isDirectory()) {
		return rv;
	}
	File f;
	while ((f = folder.openNextFile())) {
		rv.push_back(FileDTO(f.path()));
	}
	return rv;
}






