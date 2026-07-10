/**
 * @file LilyGo-EPD47-CYD.h
 * @brief FREENOVE CYD Port - Main initialization header
 * 
 * This is the main entry point for the FREENOVE CYD (3.2" LCD) port.
 * It replaces the EPD-specific LilyGo-EPD47.h with LCD-specific initialization.
 */

#pragma once

#include <Arduino.h>
#include "TinyGPS++.h"
#include "DisplayAdapter.h"
#include "DisplayAdapter_LCD.h"
#include "TouchSensor_FT6236.h"
#include "ButtonController.h"
#include "MainScreen_CYD.h"
#include "BlueFlyVario.h"
#include "LilyGo-EPD47.h"
#include "WifiWebServer.h"

// Global instances
extern DisplayAdapter_LCD* g_display;
extern TouchSensor_FT6236* g_touch;
extern ButtonController* g_buttons;
extern MainScreen_CYD* g_screen;
extern BlueFlyVario* g_vario;
extern WifiWebServer* g_wifiServer;

// Initialization functions
void setupDisplay();
void setupTouch();
void setupButtons();
void setupVario();
void setupWifi();

void idf_setup_cyd();
void idf_loop_cyd();
