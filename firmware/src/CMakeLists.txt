# ESP-IDF 5.5 Root CMakeLists.txt
# Kompatibel mit ESP32, ESP32-S3, ESP32-C3, ESP32-P4

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
get_filename_component(ProjectId ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})
