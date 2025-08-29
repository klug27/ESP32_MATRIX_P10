#include <dummy.h>

#include "FSWebServer_Lib.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "matrix_pixel_lib.h"

#define FILESYSTEM SPIFFS
#define IMG_WIDTH 64
#define IMG_HEIGHT 48
#define Pannel_Width 32
#define Pannel_Height 16
#define Pannel_para 3

File fsUploadFile,Root;
extern MatrixPanel_I2S_DMA *dma_display;
bool NEW_SCENARIO=true;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";
const char Page_WaitAndReload[] PROGMEM = R"=====(
<meta http-equiv="refresh" content="10; URL=/admin/config.html">
Please Wait....Configuring and Restarting.
)=====";

const char Page_Restart[] PROGMEM = R"=====(
<meta http-equiv="refresh" content="10; URL=/admin/general.html">
Please Wait....Configuring and Restarting.
)=====";


static bool fsOK=true;
String DeviceName = "";
String _Version = "20231209.a";

extern bool scenario_read;
uint32_t blocked=0;
bool Start_Matrix=false;
String output;
uint32_t SizeUsed=0; 
String unsupportedFiles = String();
int counter=0;
int Brightness = 77; //80%
bool BrightnessStatus=false;
wifi_mode_t mode;


static const byte DNS_PORT = 53;
WebServer server(80);
// IPAddress Server_IP(192, 168, 10, 1);
IPAddress Server_IP(10, 1, 1, 1);
IPAddress Subnet_Mask(255, 255, 255, 0);
DNSServer dnsServer;
String textip;
const String fsName="ESP_SPACE";
const String ESPAPname= "camtronics";

const char ssids[100]     = "Orange-92DE";
const char passwords[100] = "2MADFG2qmEd";

void replyOK() 
{
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) {
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) {
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg) {
  Serial.println(msg);
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg) {
  Serial.println(msg);
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}


void configureWifi()
{
  Serial.println("\r\n/********** configureWifi start **********/\r\n");
  if(WiFi.status() == WL_CONNECTED) 
  {
    delay(500);
    WiFi.disconnect();
  }
  WiFi.setHostname("ESP32_GET");
  WiFi.mode(WIFI_STA);
  mode = WIFI_STA;
  if ((ssids == nullptr) && (passwords == nullptr))
  {
    Serial.println("Veuillez entrer les champs ssid et password");
    Serial.println("\r\n/********** configureWifi end **********/\r\n");
    while(1);
  }
  else if ((ssids != nullptr) && (passwords != nullptr))
  {
    WiFi.begin(ssids, passwords);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED)
    { 
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP()); 
    Serial.println("\r\n/********** configureWifi end **********/\r\n");
  }
}


void configureWifiAP() 
{
	Serial.println("\r\n/********** ConfigureWifiAP start **********/\r\n");
//  if(WiFi.status() == WL_CONNECTED) 
//    WiFi.disconnect();

  WiFi.mode(WIFI_AP);
  mode = WIFI_AP;
  // Server_iP = 192.168.10.1
  // Subnet_Mask = 255.255.255.0
  WiFi.softAPConfig(Server_IP, Server_IP, Subnet_Mask);

  if (DeviceName.length() == 0) 
  {
    // String mac = WiFi.macAddress();
    // mac.replace(":", ""); // Enlève les deux-points
    // DeviceName = ESPAPname + "_" + mac;
    DeviceName = ESPAPname + "_"+ String(WiFi.macAddress());
  }

  WiFi.softAP(DeviceName);
  //WiFi.softAP(DeviceName.c_str(), NULL, 1, 0, 1); // Sans mot de passe, canal 1, max 1 client
  Serial.println("Acces point mode enabled\r\n");	
  Serial.print("local IP address : ");
  Serial.println(WiFi.softAPIP());
  //textip = WiFi.softAPIP().toString().c_str();
  if(MDNS.begin(ESPAPname)) 
  {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("\nOpen http://%s/config/index.html to configured your system\r\n",ESPAPname.c_str());
  }

  // Démarrage du serveur DNS : toute requête renvoyée vers l’IP locale
  // dnsServer.start(DNS_PORT, ESPAPname, WiFi.softAPIP());

  //digitalWrite(LED_BUILTIN, HIGH);  
  Serial.println("\r\n/********** ConfigureWifiAP end **********/\r\n");
}


bool Save_File(String Data, String file)
{
  Serial.print("\r\n/**********Save_File start**********/\r\n");
  File SceneFile;
  SceneFile = FILESYSTEM.open(file, "w");
  if (!SceneFile) 
  {
    Serial.print("Failed to open Scenario file for writing\r\n");
    SceneFile.close();
    return false;
  }
  SceneFile.print(Data.c_str());
  SceneFile.close();
  Serial.print("\r\n/**********Save_File end**********/\r\n");
  return true;
}

void  printFile(File dir, int numTabs, String data)
{
  Serial.println("\r\n/**********printFile**********/\r\n");
  uint32_t size=0;
  output.reserve(512);

  while (true)
  {
    File entry =  Root.openNextFile();
    if (!entry)
    {     
      // no more files
      output += "]";
      Serial.println(output);
      break;
    }    
    if (output.length()) 
    {
       if(((strstr(entry.name(),".jpeg"))||(strstr(entry.name(),".jpg"))||(strstr(entry.name(),".raw"))||(strstr(entry.name(),".rgb"))||(strstr(entry.name(),".gif")))&&(strstr(output.c_str(),"{")))
      {
        output += ',';  
      } 
    } 
    else 
    {
      output = '[';
    }
    if (entry.isDirectory())
    {
      output += "{\"type\":\"";
      output += "dir";
      output += F("\",\"name\":\"");
      output += entry.name();   
      output += "\"}";   
      Serial.println(output);  
      printFile(entry, numTabs + 1,data);
    }
    else
    {
      if((strstr(entry.name(),".jpeg"))||(strstr(entry.name(),".jpg"))||(strstr(entry.name(),".raw"))||(strstr(entry.name(),".rgb"))||(strstr(entry.name(),".gif")))
      {
        output += F("{ \r\n\"label\" : \"");
        output += (data + String(numTabs++)).c_str();
        output += F("\",\r\n\"link\" : \"");
        output += entry.name(); 
        output += "\" \r\n}";
      }    
    }   
   entry.close();    
  }
  Serial.println("\r\n/**********printFile**********/\r\n");
}


/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself

   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
void deleteRecursive(String path) 
{
  Serial.println((String("DeleteRecursive string: ") + path));
  File file = FILESYSTEM.open(path);
  bool isDir = file.isDirectory();
  file.close();
  // If it's a plain file, delete it
  if (!isDir) 
  {
    FILESYSTEM.remove(path);
    return;
  }
  
  while (true) 
  {
    File entry = Root.openNextFile();
    if (!entry)
    { 
      FILESYSTEM.rmdir(path);     
      break;
    }
    if (entry.isDirectory())
    { 
       Serial.println((String("Dir name: ") + entry.name()));
       deleteRecursive(path + '/' + entry.name());   
    }
    else
    {
      Serial.println((String("File name: ") + entry.name()));
      FILESYSTEM.remove(path + '/' + entry.name());   
    }
  }  
}

void Print_JsonFile()
{
  String data = "";
  Serial.println("\r\n/**********Print_JsonFile**********/\r\n");
  Root=FILESYSTEM.open(EFFET_FOLDER);
  deleteRecursive(EFFET_FILE);
  Root.close();
  output = "";
  data = "eff00";
  Root=FILESYSTEM.open(EFFET_FOLDER);
  printFile(Root, 1,data);
  if(Save_File(output,EFFET_FILE))
  { 
    Serial.printf("File: %s",EFFET_FILE);
    data = "[\r\n  {\r\n   \"type\": \"Roboto\"\r\n},\r\n{\r\n   \"type\": \"Arial\"\r\n},\r\n{\r\n   \"type\": \"Inter\"\r\n}\r\n]";
    if(Save_File(data,FONT_FILE))
    {
      Serial.printf("File: %s",FONT_FILE);
      data = "[\r\n  {\r\n   \"type\": \"Droite vers Gauche\"\r\n},\r\n{\r\n   \"type\": \"Gauche vers Droite\"\r\n},\r\n{\r\n   \"type\": \"Haut vers Bas\"\r\n}\r\n,{\r\n   \"type\": \"Bas vers Haut\"\r\n}\r\n ]";
      if(Save_File(data,ANIMATION_FILE))
      {
        Serial.printf("File: %s",ANIMATION_FILE);
        Root.close();output = "";
      }else {Root.close();output = ""; } 
    }else {Root.close();output = ""; } 
  }else {Root.close();output = ""; }     
  Serial.println("\r\n/**********Print_JsonFile**********/\r\n");
}

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool exists(String path){
  bool yes = false;
  File file = FILESYSTEM.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
}

void send_general_configuration_values_html() {
  Serial.println("\r\n/**************send_general_configuration_values_html****************/\r\n");
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }  
	String values = "";
	values += "devicename|" + (String)DeviceName + "|input\n";
	values += "userversion|" + _Version + "|div\n";
  server.sendHeader("Access-Control-Allow-Origin","*");
	server.send(200, "text/plain", values);
  Serial.println(values);
	Serial.println("\r\n/**************send_general_configuration_values_html****************/\r\n");
}

void lightingstatus()
{
  Serial.println("/**********lightingstatus**********/\r\n"); 
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }  
	String values = "";
	values += "lighting|" + (String)Brightness + "|input\n";
  values += "BrightnessStatus|" + (String)BrightnessStatus + "|div\n";
  server.sendHeader("Access-Control-Allow-Origin","*");
	server.send(200, "text/plain", values);
  Serial.println(values);
  Serial.println("/**********lightingstatus**********/\r\n"); 
}

void lighting()
{
  Serial.println("/**********Lighting**********/\r\n"); 
  String message;
  message.reserve(100);
  for (uint8_t i = 0; i < server.args(); i++) 
  {
    message += server.arg(i);
    message += '\n';
    Serial.printf("Arg %d:%s",i,message);
  }
  if(server.arg(0)=="true")
  {
    BrightnessStatus=true;
    Serial.printf("Brightness: %d\n BrightnessStatus: %d\n",Brightness,BrightnessStatus);
  }
  else 
  {
    Brightness=server.arg(0).toInt();
    BrightnessStatus=false;
    dma_display->setBrightness8((Brightness*255)/100); //0-255
    Serial.printf("Brightness: %d\n BrightnessStatus: %d\n",Brightness,BrightnessStatus);
  }
  Serial.println("/**********Lighting**********/\r\n"); 
}


void handleRoot2()
{
  Serial.println("\r\n/**********handleRoot start**********/\r\n");
  String uri = WebServer::urlDecode(server.uri());  // required to read paths with blanks
  if (handleFileRead(uri)) 
  {
    return;
  }
  // Dump debug data
  String message;
  message.reserve(100);
  for (uint8_t i = 0; i < server.args(); i++) 
  {
    message += server.arg(i);
    message += '\n';
  }
  Serial.println(message);
  if(uri=="/valid") 
  {
      if(Save_File(message,SCENARIO_FILE))
      {
         NEW_SCENARIO=true;
      }
  }
  else
    return replyNotFound(message);
  Serial.println("\r\n/**********handleRoot end**********/\r\n");
}

void handleRoot()
{
  String path = server.arg("classe"); 
  if (handleFileRead(F("/config/index.html"))) 
  {       
    return;
  }
 #ifdef INCLUDE_FALLBACK_INDEX_HTM
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.sendHeader(F("Content-Encoding"), "gzip");
  server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
 #else
  replyNotFound(FPSTR(FILE_NOT_FOUND));
 #endif
}


void handleGetEdit() 
{
  if (handleFileRead(F("/edit/index.html"))) 
  {
    return;
  }
 #ifdef INCLUDE_FALLBACK_INDEX_HTM
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.sendHeader(F("Content-Encoding"), "gzip");
  server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
 #else
  replyNotFound(FPSTR(FILE_NOT_FOUND));
 #endif
}

void  printSize(File dir, int numTabs)
{ 
  uint32_t size=0;
  output.reserve(64);
  while (true)
  {
    File entry =  dir.openNextFile();
    if (!entry)
    {     
      // no more files
      break;
    }    
    if (entry.isDirectory())
    {
      printSize(entry, numTabs + 1);
    }
    else
    {
      SizeUsed += entry.size();
    }   
      entry.close(); 
  }
}


void handleStatus()
{
  Serial.print("handleStatus:");
  String json;
  int tBytes = SPIFFS.totalBytes(); int uBytes = SPIFFS.usedBytes();
  File Root;
  SizeUsed=0;
  // print the type and size of the first FAT-type volume
  Root=FILESYSTEM.open("/");   
  printSize(Root,0);
  json.reserve(128);
  json = "{\"type\":\"";
  json += fsName;
  json += "\", \"isOk\":";
  if (fsOK) 
  {    
    json += F("\"true\", \"totalBytes\":\"");
    json += tBytes;
    Serial.print("Volume size: ");    
    Serial.print((float)tBytes/1000000);   
    Serial.println("Mb");        
    json += F("\", \"usedBytes\":\"");   
    json += uBytes;
    Serial.print("Volume size Used: ");   
    Serial.print(((float)uBytes)/1000);   
    Serial.println("Kb"); 
    json += "\"";
  } 
  else 
  {
    json += "\"false\"";
  }
  json += F(",\"unsupportedFiles\":\"");
  json += unsupportedFiles;
  json += "\"}";
  server.sendHeader("Access-Control-Allow-Origin","*", true);
  server.send(200, "application/json", json);
  Root.close();
}


bool handleFileRead(String path) 
{
/*  if(scenario_read)
  { 
    scenario_read=false;
    blocked=Waiting_time;
  }else blocked=Waiting_time;
*/
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path)) {
    if (exists(pathWithGz)) {
      path += ".gz";
    }
    File file = FILESYSTEM.open(path, "r");
    server.sendHeader("Access-Control-Allow-Origin","*");
    server.streamFile(file, contentType);
    file.close();
    scenario_read=true;
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) 
  {
    blocked=Waiting_time;
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    if(filename.endsWith(".jpg")||filename.endsWith(".JPG")||filename.endsWith(".jpeg")||filename.endsWith(".JPEG")||filename.endsWith(".BMP")||filename.endsWith(".bmp")||filename.endsWith(".PNG")||filename.endsWith(".png"))
      filename = "/images" + filename;
    else if(filename.endsWith(".mp4")||filename.endsWith(".mkv")||filename.endsWith(".webm")||filename.endsWith(".avi")||filename.endsWith(".raw")||filename.endsWith(".rgb"))
      filename = "/Videos" + filename;
    else if(filename.endsWith(".gif"))
      filename = "/Effets" + filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = FILESYSTEM.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) 
  {
    blocked=Waiting_time;
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if (fsUploadFile)
    {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    blocked=Waiting_time;
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
    Print_JsonFile(); 
    server.sendHeader("Access-Control-Allow-Origin","*"); 
    server.send(200, "text/plain", "");
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    server.sendHeader("Access-Control-Allow-Origin","*");
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  FILESYSTEM.remove(path);
  Print_JsonFile();  
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = FILESYSTEM.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() 
{
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);


  File root = FILESYSTEM.open(path);
  path = String();

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(file.path()).substring(1);
          output += "\"}";
          file = root.openNextFile();
      }
  }
  output += "]";
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, "text/json", output);
}



bool defaultScenario()
{ 
  String defaultS = "[\r\n  {\r\n   \"type\": \"Texte\", \r\n   \"text\": \"CAMTRONICS SARL BETA TECHNOLOGIE\", \r\n   \"font\": \"Roboto\", \r\n   \"size\": 12, \r\n   \"bcolor\": \"ff0000\", \r\n   \"fcolor\": \"0000ff\",\r\n   \"animation\": \"Droite vers Gauche\", \r\n   \"speed\": 100, \r\n   \"row\": \"center\", \r\n   \"col\": \"center\", \r\n   \"timing\": 1 \r\n}]";
  Serial.printf("Default SCENARIO %s",defaultS);
  if(Save_File(defaultS,SCENARIO_FILE))
    return true;
  else 
    return false;   
}


/*void send_network_configuration_values_html() {


  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  } 

  String values = "";
  values += "modeap|" + (String)(_config.Wifi_Mode ? "checked" : "") + "|chk\n"; //(String)_config.Wifi_Mode + "|input\n";
  values += "ssid|" + (String)_config.ssid + "|input\n";
  values += "password|" + (String)_config.password + "|input\n";
  values += "ip_0|" + (String)_config.ip[0] + "|input\n";
  values += "ip_1|" + (String)_config.ip[1] + "|input\n";
  values += "ip_2|" + (String)_config.ip[2] + "|input\n";
  values += "ip_3|" + (String)_config.ip[3] + "|input\n";
  values += "nm_0|" + (String)_config.netmask[0] + "|input\n";
  values += "nm_1|" + (String)_config.netmask[1] + "|input\n";
  values += "nm_2|" + (String)_config.netmask[2] + "|input\n";
  values += "nm_3|" + (String)_config.netmask[3] + "|input\n";
  values += "gw_0|" + (String)_config.gateway[0] + "|input\n";
  values += "gw_1|" + (String)_config.gateway[1] + "|input\n";
  values += "gw_2|" + (String)_config.gateway[2] + "|input\n";
  values += "gw_3|" + (String)_config.gateway[3] + "|input\n";
  values += "dns_0|" + (String)_config.dns[0] + "|input\n";
  values += "dns_1|" + (String)_config.dns[1] + "|input\n";
  values += "dns_2|" + (String)_config.dns[2] + "|input\n";
  values += "dns_3|" + (String)_config.dns[3] + "|input\n";
  values += "dhcp|" + (String)(_config.dhcp ? "checked" : "") + "|chk\n";

  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, "text/plain", values);
  values = "";
}

void send_connection_state_values_html() 
{
	String state = "N/A";
	String Networks = "";
	if (WiFi.status() == 0) state = "Idle";
	else if (WiFi.status() == 1) state = "NO SSID AVAILBLE";
	else if (WiFi.status() == 2) state = "SCAN COMPLETED";
	else if (WiFi.status() == 3) state = "CONNECTED";
	else if (WiFi.status() == 4) state = "CONNECT FAILED";
	else if (WiFi.status() == 5) state = "CONNECTION LOST";
	else if (WiFi.status() == 6) state = "DISCONNECTED";

	WiFi.scanNetworks(true);

	String values = "";
	values += "connectionstate|" + state + "|div\n";
	//values += "networks|Scanning networks ...|div\n";
  server.sendHeader("Access-Control-Allow-Origin","*");
	server.send(200, "text/plain", values);
	state = "";
	values = "";
	Networks = "";
}

void send_information_values_html() 
{
  rtc_get_datetime(&currTime);
  // Display time from RTC
  DateTime_ now = DateTime_(currTime); 
  time_t utc = now.get_time_t();
  time_t local = myTZ->toLocal(utc, &tcr);
  char Time[10];  
  char Timescr[10];
  char Date[20];
  int heure=0;
  int min=0;
  int min_=(DISPLAY_RTC_INTERVAL/60000)+minute(utc); 
  if (min_>= 60)
  {
    heure = hour(utc)+1;
    min = min_- 60;
  } 
  else 
  {
    heure = hour(utc);
    min = (DISPLAY_RTC_INTERVAL/60000)+minute(utc);    
  }
   char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
  strcpy(m, monthShortStr(month(utc)));
  sprintf(Time, "%.2d:%.2d:%.2d", hour(utc), minute(utc), second(utc));
  sprintf(Timescr, "%.2d:%.2d:%.2d", heure, min, second(utc));
  sprintf(Date, "%s %.2d %s %d ", dayShortStr(weekday(utc)), day(utc), m, year(utc));
  String wifi_mode="";
  String values = "";
  if(_config.Wifi_Mode==true)
  {
      wifi_mode="AP";
      values += "x_Wifi_Mode|" + wifi_mode + "|div\n"; 
      values += "x_ssid|" + (String)"PICO_AP" + "|div\n";
      values += "x_ip|" + (String)"10.1.1.1" + "|div\n";
      values += "x_gateway|" + (String)"10.1.1.1"+ "|div\n";
      values += "x_netmask|" + (String)"255.255.255.0" + "|div\n";
      values += "x_mac|" + getMacAddress() + "|div\n";
      values += "x_dns|" + (String)"10.1.1.1" + "|div\n";
  }
  else 
  {
      wifi_mode="STATION"; 
      values += "x_Wifi_Mode|" + wifi_mode + "|div\n"; 
      values += "x_ssid|" + (String)WiFi.SSID() + "|div\n";
      values += "x_ip|" + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] + "|div\n";
      values += "x_gateway|" + (String)WiFi.gatewayIP()[0] + "." + (String)WiFi.gatewayIP()[1] + "." + (String)WiFi.gatewayIP()[2] + "." + (String)WiFi.gatewayIP()[3] + "|div\n";
      values += "x_netmask|" + (String)WiFi.subnetMask()[0] + "." + (String)WiFi.subnetMask()[1] + "." + (String)WiFi.subnetMask()[2] + "." + (String)WiFi.subnetMask()[3] + "|div\n";
      values += "x_mac|" + getMacAddress() + "|div\n";
      values += "x_dns|" + (String)WiFi.gatewayIP()[0] + "." + (String)WiFi.gatewayIP()[1] + "." + (String)WiFi.gatewayIP()[2] + "." + (String)WiFi.gatewayIP()[3] + "|div\n";
  }
  values += "x_ntp_sync|" + String(Timescr) + "|div\n";
  values += "x_ntp_time|" + String(Time) + "|div\n";
  values += "x_ntp_date|" + String(Date) + "|div\n";
  server.sendHeader("Access-Control-Allow-Origin","*");
	server.send(200, "text/plain", values);
  MAIN_LOGDEBUG(values);
	values = "";
}

void send_network_configuration_html() 
{
	if (server.args() > 0)  // Save Settings
	{
		//String temp = "";
		bool oldDHCP = _config.dhcp; // Save status to avoid general.html cleares it
		_config.dhcp = false;
    bool oldWifi_mode = _config.Wifi_Mode; // Save status to avoid general.html cleares it
		_config.Wifi_Mode = false;
		for (uint8_t i = 0; i < server.args(); i++) 
    {
			//MAIN_LOGDEBUG1("Arg %d: %s\r\n", i, server.arg(i).c_str());
      MAIN_LOGDEBUG0("Arg ");
      MAIN_LOGDEBUG0(server.argName(i).c_str());
      MAIN_LOGDEBUG0(":");
      MAIN_LOGDEBUG0(server.arg(i).c_str());
      MAIN_LOGDEBUG();
			if (server.argName(i) == "devicename") 
      {
				_config.deviceName = urldecode(server.arg(i));
				_config.dhcp = oldDHCP;
        _config.Wifi_Mode =oldWifi_mode;
				continue;
			} 
      if (server.argName(i) == "modeap") { _config.Wifi_Mode = true; continue; }         
			if (server.argName(i) == "ssid") { _config.ssid = urldecode(server.arg(i));	continue; }
			if (server.argName(i) == "password") { _config.password = urldecode(server.arg(i)); continue; }
			if (server.argName(i) == "ip_1") { if (checkRange(server.arg(i))) 	_config.ip[0] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "ip_2") { if (checkRange(server.arg(i))) 	_config.ip[1] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "ip_3") { if (checkRange(server.arg(i))) 	_config.ip[2] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "ip_4") { if (checkRange(server.arg(i))) 	_config.ip[3] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "nm_1") { if (checkRange(server.arg(i))) 	_config.netmask[0] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "nm_2") { if (checkRange(server.arg(i))) 	_config.netmask[1] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "nm_3") { if (checkRange(server.arg(i))) 	_config.netmask[2] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "nm_4") { if (checkRange(server.arg(i))) 	_config.netmask[3] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "gw_1") { if (checkRange(server.arg(i))) 	_config.gateway[0] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "gw_2") { if (checkRange(server.arg(i))) 	_config.gateway[1] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "gw_3") { if (checkRange(server.arg(i))) 	_config.gateway[2] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "gw_4") { if (checkRange(server.arg(i))) 	_config.gateway[3] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "dns_1") { if (checkRange(server.arg(i))) 	_config.dns[0] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "dns_2") { if (checkRange(server.arg(i))) 	_config.dns[1] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "dns_3") { if (checkRange(server.arg(i))) 	_config.dns[2] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "dns_4") { if (checkRange(server.arg(i))) 	_config.dns[3] = server.arg(i).toInt(); continue; }
			if (server.argName(i) == "dhcp") { _config.dhcp = true; continue; }
		}
    
    server.sendHeader("Access-Control-Allow-Origin","*");
		server.send_P(200, "text/html", Page_WaitAndReload);
		save_config();
		delay(1000);
		SdFileSystem.end();
    WiFi.disconnect();
    delay(100); 
		restart_Pico();
 	} 
	else {
   String uri = WebServer::urlDecode(server.uri());     
		//MAIN_LOGDEBUG1(uri);	
    handleFileRead(uri);
	}
}

void send_general_configuration_html() 
{
  if (server.args() > 0)  // Save Settings
  {
    for (uint8_t i = 0; i < server.args(); i++) 
    {
      MAIN_LOGDEBUG0("Arg ");
      MAIN_LOGDEBUG0(i);
      MAIN_LOGDEBUG0(": ");
      MAIN_LOGDEBUG0(server.arg(i).c_str());
      MAIN_LOGDEBUG();
      if (server.argName(i) == "devicename") 
      {
        _config.deviceName = urldecode(server.arg(i));
        continue;
      }
    }
    server.sendHeader("Access-Control-Allow-Origin","*");
    server.send(200, "text/html", Page_Restart);
    save_config();
    SdFileSystem.end();
    WiFi.disconnect();
    delay(100);
    restart_Pico();
  }
  else 
  {
    String uri = WebServer::urlDecode(server.uri());
    handleFileRead(uri);
  }
}

void send_wwwauth_configuration_html() {
	MAIN_LOGDEBUG1("send_wwwauth_configuration_html: ", server.args());
	if (server.args() > 0)  // Save Settings
	{
		_httpAuth.auth = false;
		//String temp = "";
		for (uint8_t i = 0; i < server.args(); i++) 
    {
			if (server.argName(i) == "wwwuser") {
				_httpAuth.wwwUsername = urldecode(server.arg(i));
				MAIN_LOGDEBUG1("User: %s\n", _httpAuth.wwwUsername.c_str());
				continue;
			}
			if (server.argName(i) == "wwwpass") {
				_httpAuth.wwwPassword = urldecode(server.arg(i));
				MAIN_LOGDEBUG1("Pass: %s\n", _httpAuth.wwwPassword.c_str());
				continue;
			}
			if (server.argName(i) == "wwwauth") {
				_httpAuth.auth = true;
				MAIN_LOGDEBUG("HTTP Auth enabled\r\n");
				continue;
			}
		}

		saveHTTPAuth();
	}
	handleFileRead("/admin/system.html");

	MAIN_LOGDEBUG("send_wwwauth_configuration_html");
}

void send_wwwauth_configuration_values_html() 
{
	String values = "";
	values += "wwwauth|" + (String)(_httpAuth.auth ? "checked" : "") + "|chk\n";
	values += "wwwuser|" + (String)_httpAuth.wwwUsername + "|input\n";
	values += "wwwpass|" + (String)_httpAuth.wwwPassword + "|input\n";
  server.sendHeader("Access-Control-Allow-Origin","*");
	server.send(200, "text/plain", values);
}

void scan_WIFI()
{
 		String json = "[";
		int n = WiFi.scanComplete();
		if (n == WL_AP_FAILED) {
			WiFi.scanNetworks(true);
		}
		else if (n) {
			for (int i = 0; i < n; ++i) {
        uint8_t bssid[6];
        WiFi.BSSID(i, bssid);
				if (i) json += ",";
				json += "{";
				json += "\"rssi\":" + String(WiFi.RSSI(i));
				json += ",\"ssid\":\"" + String(WiFi.SSID(i)) + "\"";
				json += ",\"bssid\":\"" + String(macToString(bssid)) + "\"";
				json += ",\"channel\":" + String(WiFi.channel(i));
				json += ",\"secure\":" + String(WiFi.encryptionType(i));
				json += ",\"hidden\":" + String("false" ? "true" : "false");
				json += "}";
			}
			WiFi.scanDelete();
			if (WiFi.scanComplete() == WL_AP_FAILED) {
				WiFi.scanNetworks(true);
			}
		}
		json += "]";
    server.sendHeader("Access-Control-Allow-Origin","*");
		server.send(200, "text/json", json);
    MAIN_LOGDEBUG(json);
		json = "";
}
*/
void handleNotFound() 
{
  String uri = WebServer::urlDecode(server.uri());  // required to read paths with blanks
  if (handleFileRead(uri)) 
  {
    return;
  }
  // Dump debug data
  String message;
  message.reserve(100);
  for (uint8_t i = 0; i < server.args(); i++) 
  {
    message += server.arg(i);
    message += '\n';
  }
 if (!fsOK) 
 {
    return replyServerError(FPSTR(FS_INIT_ERROR));
 }
 else
    return replyNotFound(message);
}

void Init_Server()
{   
 ////////////////////////////////
    // WEB SERVER INIT
    server.on("/admin/generalvalues", HTTP_GET, send_general_configuration_values_html);
    //server.on("/admin/values", HTTP_GET, send_network_configuration_values_html);
    //server.on("/admin/connectionstate", HTTP_GET, send_connection_state_values_html);
    //server.on("/admin/infovalues", HTTP_GET, send_information_values_html);
    //server.on("/admin/infovalues2", HTTP_GET, send_information_values_html2);
    //server.on("/admin/config.html", HTTP_GET, send_network_configuration_html);
    //server.on("/admin/config", HTTP_POST, send_network_configuration_html);
    //server.on("/scan", HTTP_GET, scan_WIFI);    
    //server.on("/config.html", HTTP_GET, send_network_configuration_html);
    server.on("/admin/general.html", HTTP_GET, send_general_configuration_html);   
    server.on("/admin/general", HTTP_POST, send_general_configuration_html);  
    //server.on("/admin/system.html", HTTP_GET, send_wwwauth_configuration_html);
    //server.on("/admin/restart", HTTP_GET, restart_Pico);
    //server.on("/admin/wwwauth", HTTP_GET, send_wwwauth_configuration_values_html);
    //server.on("/admin/index.html", HTTP_GET, admin);
    //server.on("/admin", HTTP_GET, admin);
    server.on("/admin/lighting", HTTP_POST, lighting);
    server.on("/admin/lighting", HTTP_GET, lightingstatus);
    //server.on("/update/updatepossible", HTTP_GET, send_update_firmware_values_html);
    //server.on("/setmd5", HTTP_GET, setUpdateMD5);
    //server.on("/rconfig", HTTP_GET, handle_rest_config);
    //server.on("/pconfig", HTTP_POST, post_rest_config);
    //server.on("/login/index.html",HTTP_GET, passwordpage);
    //server.on("/update", HTTP_GET, Update_html);
   // server.on("/update", HTTP_POST,replyOK, updateFirmware);
    // Read matrix status 
    // Matrix get 
    server.on("/",HTTP_GET, handleRoot);
    server.on("/valid",HTTP_POST, handleRoot2);
    //server.on("/valid.html", HTTP_POST, replyOK, handleFileUpload);   
    server.on("/index.htm",HTTP_GET, handleRoot);
    // Filesystem status 
    server.on("/status", HTTP_GET, handleStatus);
    // List directory
    server.on("/list", HTTP_GET, handleFileList);
    // Load editor
    server.on("/edit", HTTP_GET, handleGetEdit);
    // Create file
    server.on("/edit", HTTP_PUT, handleFileCreate);
    server.on("/edit/rename", HTTP_POST, handleFileCreate);
    // Delete file
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    server.on("/edit/delete", HTTP_POST, handleFileDelete);
    // Upload file
    // - first callback is called after the request has ended with all parsed arguments
    // - second callback handles file upload at that location
    server.on("/edit", HTTP_POST, replyOK, handleFileUpload);
    // Default handler for all URIs not defined above
    // Use it to read files from filesystem
    server.onNotFound(handleNotFound);
    // Start server
    server.begin();
    




   
  /*server.on("/admin/general.html", HTTP_GET, send_general_configuration_html);   
  server.on("/admin/general", HTTP_POST, send_general_configuration_html);  
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/",HTTP_GET, handleRoot2);
  server.on("/admin/generalvalues", HTTP_GET, send_general_configuration_values_html);
  // List directory
  server.on("/valid",HTTP_POST, handleRoot);
  // List directory
  server.on("/list", HTTP_GET, handleFileList);
  // Status Of light
  server.on("/admin/lighting", HTTP_POST, lighting);
  server.on("/admin/lighting", HTTP_GET, lightingstatus);
  // Load editor
  server.on("/edit", HTTP_GET, handleGetEdit);
  // Create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  server.on("/edit/rename", HTTP_POST, handleFileCreate);
  // Delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  server.on("/edit/delete", HTTP_POST, handleFileDelete);
  // Upload file
  // - first callback is called after the request has ended with all parsed arguments
  // - second callback handles file upload at that location
  server.on("/edit", HTTP_POST, replyOK, handleFileUpload);
  // Default handler for all URIs not defined above
  // Use it to read files from filesystem
  server.onNotFound(handleNotFound);
    // Start server
  server.begin();
 // */
}

void Server_handler()
{	
  server.handleClient();	
}

// size_t size;
// char buf[2048];

// bool load_Scenario() 
// {
//   Serial.print("/**********load_Scenario**********/\r\n"); 
//   int Scrolldirection=0,Start_Y=0;
//   char _tab[255]; char Scrolldir[25],Col[25],Size=0;
//   memset(_tab, 0, 255); 
//   String Scenario_write="";
//   if(NEW_SCENARIO)
//   {
//     memset(buf, 0, 2048); 
//     File SceneFile = FILESYSTEM.open(SCENARIO_FILE, "r");
//     if (!SceneFile)
//     {
//       Serial.print("Failed to open config file\r");
//       return false;
//     }

//     size = SceneFile.size();

//     // Allocate a buffer to store contents of the file.
//     //std::unique_ptr<char[]> buf(new char[size]);
//     //Serial.printf("191 JSON file size: %lu bytes\r\n",size);
//     //buf[size];
//   // memset(0,0,size);
//     // We don't use String here because ArduinoJson library requires the input
//     // buffer to be mutable. If you don't use ArduinoJson, you may as well
//     // use configFile.readString instead.
//     //SceneFile2.readBytes(buf.get(), size);
//     SceneFile.readBytes(buf, size);
//     SceneFile.close();
//     Serial.printf("191 JSON file size: %lu bytes\r\n",size);
//     NEW_SCENARIO=false;
//   }
// 	DynamicJsonDocument json(2048);
// 	//auto error = deserializeJson(json, buf.get());
//   auto error = deserializeJson(json, buf);
// 	if (error) {
// 		Serial.printf("deserializeJson() failed with code %s",error.c_str());
// 		return false;
// 	}
  
//   String temp;
// 	serializeJsonPretty(json,temp);
// 	Serial.println(temp);
//   String Scene="";
  
//   for(int i=0; i<10; i++)
//   {
//     Scene=json[i]["type"].as<const char *>();
//     Serial.printf("Type: %s\r\n",Scene);
//     if (Scene=="Texte")   
//     {   
//       clearMatrix();
//       char bcolor[20];
//       memset(_tab, 0, 255); 
//       memset(Scrolldir, 0, sizeof(Scrolldir)); 
//       memset(Col, 0, sizeof(Col)); 

//       strcat(_tab,json[i]["text"].as<const char *>());
//       strcat(Scrolldir,json[i]["animation"].as<const char *>());
//       strcat(Col,json[i]["col"].as<const char *>());

//       Serial.printf("Texte write: %s\r\n",_tab);

//       if(!strcmp(Scrolldir,"Droite vers Gauche")) {Scrolldirection=scr_left;}
//       else if(!strcmp(Scrolldir,"Gauche vers Droite")) {Scrolldirection=scr_right;}
//       else if(!strcmp(Scrolldir,"Haut vers Bas")) {Scrolldirection=scr_down;}
//       else if(!strcmp(Scrolldir,"Bas vers Haut")) {Scrolldirection=scr_up;}

//       Size=json[i]["size"].as<long>();
    
//       if(!strcmp(Col,"start")) 
//       {
//            Start_Y=((IMG_HEIGHT/Pannel_para)-Size)/2;
//       }
//       else if(!strcmp(Col,"center")) 
//       {
//           Start_Y=(IMG_HEIGHT/2)-(Size/2);
//       }
//       else if(!strcmp(Col,"end"))
//       {
//           Start_Y=(IMG_HEIGHT-Pannel_Height)+(((IMG_HEIGHT/Pannel_para)-Size)/2);
//       }
//       for(int j=0; j<(json[i]["timing"].as<long>()); j++)
//       {
//        Serial.printf("Start_Y: %d\r\n",Start_Y);
//        if(!scenario_read) break;
//        Matrix_scrolling_Text(Start_Y,IMG_HEIGHT,0,IMG_WIDTH,_tab,color24to565(strtol(json[i]["fcolor"].as<const char *>(), NULL, 16)),color24to565(strtol(json[i]["bcolor"].as<const char *>(), NULL, 16)),json[i]["size"].as<long>(),json[i]["speed"].as<long>(),Scrolldirection);
//       }
//     } 
//     else if (Scene=="Effet")   
//     {
//       clearMatrix();
//       memset(_tab, 0, 255); 
//       strcat(_tab,(String(EFFET_FOLDER) + String("/") + String(json[i]["link"].as<const char *>())).c_str());
//       Serial.printf("File: %s \n",_tab);
//       dump_effet(_tab,json[i]["timing"].as<long>()); 
//     }
//     else if(Scene=="Image")
//     {
//       clearMatrix();
//       memset(_tab, 0, 255); 
//       strcat(_tab, ( String("/") + String(json[i]["link"].as<const char *>())).c_str());
//       Serial.printf("File: %s\n",_tab);
//       dump_image(_tab, json[i]["timing"].as<long>());
//     }
//     else if (Scene=="")   
//     {
//       break;     
//     }
//   }  
// 	Serial.println("\r\n/**********load_Scenario**********/\r\n");
// 	return true;
// }


void deleteFile()
{
    File file = FILESYSTEM.open(CONFIG_FILE, "r");
    if(file) 
    {
      FILESYSTEM.remove(CONFIG_FILE); 
      Serial.println("CONFIG_FILE delected ok\n\n");
    } 
    else Serial.println("CONFIG_FILE not delected\n\n"); 
    file.close();
}


void defaultConfig() 
{
	Serial.println("\r\n/**********DefaultConfig**********/\r\n");
  // String mac = WiFi.macAddress();
  // mac.replace(":", ""); // Enlève les deux-points
  // DeviceName = ESPAPname + "_" + mac;
  DeviceName = ESPAPname + "_"+ String(WiFi.macAddress());
	  File file = FILESYSTEM.open(CONFIG_FILE, "r");
	  if(file)
	  {
		  bool isDir = file.isDirectory();
		  file.close();
		  // If it's a plain file, delete it
		  if (!isDir) {
			FILESYSTEM.remove(CONFIG_FILE);
		  }
	  } else file.close();
	save_config();
  Serial.println("\r\n/**********DefaultConfig**********/\r\n");
}

bool save_config()
 {
  Serial.println("\r\n/**********save_config**********/\r\n");
	//flag_config = false;
	DynamicJsonDocument json(1024);
	
	json["deviceName"] = DeviceName;

	//TODO add AP data to html
	File configFile = FILESYSTEM.open(CONFIG_FILE, "w");
	if (!configFile) {
		Serial.println("Failed to open config file for writing\r\n");
		configFile.close();
		return false;
	}
	String temp;
	serializeJsonPretty(json,temp);
	Serial.println(temp);

	serializeJson(json,configFile);
	configFile.flush();
	configFile.close();
  Serial.println("\r\n/**********save_config**********/\r\n");
  return true;
}

bool load_config() 
{
	Serial.println("/**********load_config**********/\r\n"); 

	File configFile = FILESYSTEM.open(CONFIG_FILE, "r");
	if (!configFile) {
		Serial.println("Failed to open config file\r");
		return false;
	}

	size_t size = configFile.size();

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);
	configFile.close();
	Serial.print("191 JSON file size: ");
  Serial.print(size);
  Serial.println("bytes\r\n");
	DynamicJsonDocument json(1024);
	auto error = deserializeJson(json, buf.get());
	if (error) {
		Serial.print("deserializeJson() failed with code ");
		Serial.println(error.c_str());
		return false;
	}
  
  String temp;
	serializeJsonPretty(json,temp);
	Serial.println(temp);
	DeviceName = json["deviceName"].as<const char *>();
  
	Serial.println("\r\n/**********load_config**********/\r\n");
	return true;
}

void send_general_configuration_html() 
{
  Serial.println("\r\n/*************send_general_configuration_html**************/\r\n");
  if (server.args() > 0)  // Save Settings
  {
    for (uint8_t i = 0; i < server.args(); i++) 
    {
      Serial.print("Arg ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(server.arg(i).c_str());
      if (server.argName(i) == "devicename") 
      {
        DeviceName = urldecode(server.arg(i));
        continue;
      }
    }
    server.sendHeader("Access-Control-Allow-Origin","*");
    server.send(200, "text/html", Page_Restart);
    save_config();
    FILESYSTEM.end();
    delay(100);
    ESP.restart();
  }
  else 
  {
    String uri = WebServer::urlDecode(server.uri());
    handleFileRead(uri);
  }
  Serial.println("\r\n/*************send_general_configuration_html**************/\r\n");
}

// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c) {
	if (c >= '0' && c <= '9') {
		return((unsigned char)c - '0');
	}
	if (c >= 'a' && c <= 'f') {
		return((unsigned char)c - 'a' + 10);
	}
	if (c >= 'A' && c <= 'F') {
		return((unsigned char)c - 'A' + 10);
	}
	return(0);
}

String urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
	char c;
	String ret = "";

	for (byte t = 0; t < input.length(); t++) {
		c = input[t];
		if (c == '+') c = ' ';
		if (c == '%') {


			t++;
			c = input[t];
			t++;
			c = (h2int(c) << 4) | h2int(input[t]);
		}

		ret.concat(c);
	}
	return ret;

}
