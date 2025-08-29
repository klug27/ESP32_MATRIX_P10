
#include "matrix_pixel_lib.h"
#include "FSWebServer_Lib.h"
#include "matrix_misc.h"
#include "At24.h"

#define Une_minute 60
#define Cinq_minute 300
#define Dix_minute 600

File gifFile, root;
extern bool scenario_read;
extern uint32_t blocked;
extern MatrixPanel_I2S_DMA *dma_display;
extern int Brightness;
extern uint16_t rotation;
uint16_t LDR=0;
char filePath[256] = { 0 };
String gifDir = "/Effets";  // play all GIFs in this directory on the SD card
static uint8_t u8Mac[6];
static uint8_t u8Uuid[16];
static uint8_t u8TxBuffer[6] = {10,20,30,40,50,60};
static uint8_t u8RxBuffer[6] = {0};


TaskHandle_t Task0;
TaskHandle_t Task1;

bool load_Scenario() {
  Serial.print("/**********load_Scenario**********/\r\n");
  int Scrolldirection = 0, Start_Y = 0;
  char _tab[255];
  char Scrolldir[25], Col[25], Size = 0;
  memset(_tab, 0, 255);
  String Scenario_write = "";
  File SceneFile = FILESYSTEM.open(SCENARIO_FILE, "r");
  if (!SceneFile) {
    Serial.print("Failed to open config file\r");
    return false;
  }

  size_t size = SceneFile.size();
  char buf[size];
  SceneFile.readBytes(buf, size);
  SceneFile.close();
  Serial.printf("191 JSON file size: %lu bytes\r\n", size);
  DynamicJsonDocument json(2048);
  //auto error = deserializeJson(json, buf.get());
  auto error = deserializeJson(json, buf);
  if (error) {
    Serial.printf("deserializeJson() failed with code %s", error.c_str());
    return false;
  }

  int status = 0;
  String temp;
  serializeJsonPretty(json, temp);
  Serial.println(temp);
  String Scene = "";

  for (int i = 0; i < 10; i++) {
    Scene = json[i]["type"].as<const char *>();
    Serial.printf("Type: %s\r\n", Scene);
    if (Scene == "Texte") {
      clearMatrix();
      char bcolor[20];
      memset(_tab, 0, 255);
      memset(Scrolldir, 0, sizeof(Scrolldir));
      memset(Col, 0, sizeof(Col));

      strcat(_tab, json[i]["text"].as<const char *>());
      strcat(Scrolldir, json[i]["animation"].as<const char *>());
      strcat(Col, json[i]["col"].as<const char *>());

      Serial.printf("Texte write: %s\r\n", _tab);

      if (!strcmp(Scrolldir, "Droite vers Gauche")) {
        Scrolldirection = scr_left;
      } else if (!strcmp(Scrolldir, "Gauche vers Droite")) {
        Scrolldirection = scr_right;
      } else if (!strcmp(Scrolldir, "Haut vers Bas")) {
        Scrolldirection = scr_down;
      } else if (!strcmp(Scrolldir, "Bas vers Haut")) {
        Scrolldirection = scr_up;
      }

      Size = json[i]["size"].as<long>();

      if (!strcmp(Col, "start")) {
        status = 1;
        Start_Y = ((IMG_HEIGHT / Pannel_para) - Size) / 2;
      } else if (!strcmp(Col, "center")) {
        status = 2;
        Start_Y = ((IMG_HEIGHT / 2) - (Size / 2)) - 1;
      } else if (!strcmp(Col, "end")) {
        status = 3;
        Start_Y = (IMG_HEIGHT - Pannel_Height) + (((IMG_HEIGHT / Pannel_para) - Size) / 2);
      }
      for (int j = 0; j < (json[i]["timing"].as<long>()); j++) {
        //Serial.printf("Start_Y: %d\r\n", Start_Y);
        if (!scenario_read) break;
        Matrix_scrolling_Text(Start_Y, IMG_HEIGHT, 0, IMG_WIDTH, _tab, color24to565(strtol(json[i]["fcolor"].as<const char *>(), NULL, 16)), color24to565(strtol(json[i]["bcolor"].as<const char *>(), NULL, 16)), json[i]["size"].as<long>(), json[i]["speed"].as<long>(), Scrolldirection, status);
      }
    } else if (Scene == "Effet") {
      clearMatrix();
      memset(_tab, 0, 255);
      strcat(_tab, (String(EFFET_FOLDER) + String("/") + String(json[i]["link"].as<const char *>())).c_str());
      Serial.printf("File: %s \n", _tab);
      if (!scenario_read) break;
      dump_effet(_tab, json[i]["timing"].as<long>());
    } else if (Scene == "Image") {
      clearMatrix();
      memset(_tab, 0, 255);
      strcat(_tab, (String(IMAGE_FOLDER) + String("/") + String(json[i]["link"].as<const char *>())).c_str());
      Serial.printf("File: %s\n", _tab);
      if (!scenario_read) break;
      dump_image(_tab, json[i]["timing"].as<long>());
    } else if (Scene == "") {
      break;
    }
  }
  Serial.println("\r\n/**********load_Scenario**********/\r\n");
  return true;
}

bool Status=true;

void loop0(void *parameter) 
{
  for (;;) 
  {
    Server_handler();
    ArduinoOTA.handle();
    dma_display->setBrightness8(Brightness); //0-255
    //LDR= analogRead(LDR_PIN);
    //Brightness=((255*LDR)/4096); //connecter la LDR pour pouvoir l'utilisé
    Brightness=255;
  }
}


void loop1(void *parameter) 
{
  for (;;) 
  {
 // take_constance(Une_minute);
    Serial.printf("LDR: %u Brightness: %u\r\n", LDR,((Brightness*100)/255));
    if((blocked == 0) && (scenario_read)) 
    {
      if (scenario_read) 
      {
        Serial.printf("Free Heap space: %u\r\n", ESP.getFreeHeap());
        if (load_Scenario()) {
          Serial.println("File Open");
        } else {
          delay(1000);
          Serial.println("File not Open");
        }
        Serial.printf("After Free Heap space: %u\r\n", ESP.getFreeHeap());
      }
    } 
    else if ((blocked == 0) && (!scenario_read)) 
      scenario_read = true;
    else 
      blocked--;
    delay(1);
  }
}

int compteur=0;

void setup() 
{
  /************** SERIAL **************/
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
/*  WiFi.mode(WIFI_STA);
  WiFi.begin("Orange-92DE", "2MADFG2qmEd");
  Serial.println("Connecting");
  while ((WiFi.status() != WL_CONNECTED)&&(compteur<30)) 
  {
    delay(500);
    Serial.print(".");
    compteur++;
  }
  
  if(compteur<30) Init_OTA("myesp32", "admin");
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
*/

  /************** Start filesystem**************/
  Serial.println("Loading SPIFFS");
  if (!FILESYSTEM.begin())
    Serial.println("SPIFFS Mount Failed");
  if (!FILESYSTEM.exists(SCENARIO_FILE))
    defaultScenario();
  if (!load_config())
    defaultConfig();  // Load defaults if any error

  //set_rotation(0);
  Print_JsonFile();
  configureWifiAP();
  Init_OTA("myesp32", "admin");
  rotation=degre_180;
  FAN_Init();
  LDR_Init();
  Init_Server();
  init_Matrix();
  dht22_init();

  i2cInit();
  Config_eeprom();

  readMemMacAddress(u8Mac);
  readMemSerialNumber(u8Uuid);
  writeMem(&u8TxBuffer[0], 0x7A, 6);
  readMem(&u8RxBuffer[0], 0x7A, 6);
  Serial.printf("Buffer Read: %s\r\n",MACtoString(u8RxBuffer));

  xTaskCreatePinnedToCore(
    loop0,   /* Function to implement the task */
    "Task0", /* Name of the task */
    10000,   /* Stack size in words */
    NULL,    /* Task input parameter */
    0,       /* Priority of the task */
    &Task0,  /* Task handle. */
    0);      /* Core where the task should run */

  delay(500);
  scenario_read = true; 
  blocked =0;
  xTaskCreatePinnedToCore(
    loop1,   /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,   /* Stack size in words */
    NULL,    /* Task input parameter */
    1,       /* Priority of the task */
    &Task1,  /* Task handle. */
    1);      /* Core where the task should run */
  //  /************** DISPLAY **************/
}


void loop() 
{
}
