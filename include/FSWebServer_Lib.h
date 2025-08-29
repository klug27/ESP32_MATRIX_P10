#pragma once

#ifndef __FSWEBSERVER_LIB_H
#define __FSWEBSERVER_LIB_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <StreamString.h>
#include "SPIFFS.h"
#include "FS.h"
//#include <LEAmDNS.h>



#define CONFIG_FILE             "/jsonFiles/config.json"
#define SCENARIO_FILE           "/jsonFiles/scenario.json"
#define USER_CONFIG_FILE        "/jsonFiles/userconfig.json"
#define GENERIC_CONFIG_FILE     "/jsonFiles/genericconfig.json"
#define SECRET_FILE             "/jsonFiles/secret.json"
#define EFFET_FOLDER            "/Effets"
#define EFFET_FILE              "/jsonFiles/effets.json"
#define GIF_FOLDER              "/Effets/"
#define VIDEO_FOLDER            "/Videos"
#define VIDEO_FILE              "/jsonFiles/videos.json"
#define IMAGE_FOLDER            "/images"
#define IMAGE_FILE              "/jsonFiles/images.json"
#define ANIMATION_FILE          "/jsonFiles/animation.json"
#define FONT_FILE               "/jsonFiles/font.json" 
#define Waiting_time 20000


typedef struct {
    String APssid = "Pico_AP"; // ChipID is appended to this name
    String APpassword = "12345678";
    bool APenable = false; // AP disabled by default
} strApConfig;



void replyOK() ;
void replyOKWithMsg(String msg);
void replyNotFound(String msg);
void replyBadRequest(String msg);
void replyServerError(String msg);
void configureWifiAP();
void configureWifi();
bool Save_File(String Data, String file);
void printFile(File dir, int numTabs, String data);
void deleteRecursive(String path);
void Print_JsonFile();
String formatBytes(size_t bytes) ;
String getContentType(String filename) ;
bool exists(String path);
void send_general_configuration_values_html() ;
void lightingstatus();
void lighting();
void handleRoot();
void handleRoot2();
void handleGetEdit();
void printSize(File dir, int numTabs);
void handleStatus();
bool handleFileRead(String path) ;
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList() ;
bool load_Scenario();
bool defaultScenario();
void handleNotFound() ;
void Init_Server();
void Server_handler();
void deleteFile();
void defaultConfig();
bool save_config();
bool load_config();
void send_general_configuration_html();
String urldecode(String input) ;
unsigned char h2int(char c);
#endif // __FSWEBSERVER_LIB_H
