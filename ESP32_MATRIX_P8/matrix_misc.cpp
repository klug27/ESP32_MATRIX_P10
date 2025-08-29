#include "matrix_misc.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define Temp_Max 70
#define Temp_start_Fan 45

// Replace with your network credentials
const char* ssid     = "BE_HIGH_COM";
const char* password = "Mtress1265mac89";
// REPLACE with your Domain name and URL path or IP address with path
const char* serverName = "http://192.168.1.162/post-esp-data.php";


// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "tPmAT5Ab3j7F9";

String sensorName = "DHT22";
String sensorLocation = "Office";
extern bool scenario_read;
extern int Brightness;
extern uint32_t blocked;

#define DHTTYPE DHT22  

DHT dht(pinDATA, DHTTYPE);

  // configure LED PWM functionalitites

  
  // attach the channel to the GPIO to be controlled


void LDR_Init()
{
  analogReadResolution(12);
  Serial.println("LDR_Init");
}

void FAN_Init()
{
  int dutyCycle = 175;
  Serial.println("FAN_Init");
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(FAN, ledChannel);

  FAN_PWM(dutyCycle);
  delay(3000);

  for(dutyCycle = 0; dutyCycle <= 255; dutyCycle+=25)
  {   
    // changing the LED brightness with PWM
    Serial.printf("dutyCycle: %d\r\n",dutyCycle);
    FAN_PWM(dutyCycle);
    delay(500);
  }
}

void FAN_PWM(int dutyCycle)
{
  ledcWrite(ledChannel, dutyCycle);
}

void LED_LOW()
{
  digitalWrite(FAN, LOW);
}
void LED_HIGH()
{
  digitalWrite(LED_BUILTIN, HIGH);
}

// void FAN_LOW()
// {
//   digitalWrite(LED_BUILTIN, LOW);
// }
void dht22_init()
{
  dht.begin();
}

// float Humidity=0;
// float temperature=0;

void Send_constance(float Humidity,float temperature,int Brightness )
{

    //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED)
  {
  WiFiClient client;
  HTTPClient http;
  
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Prepare your HTTP POST request data
  String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                        + "&location=" + sensorLocation + "&brightness=" + String(((Brightness*100)/255))
                        + "&temperature=" + String(temperature) + "&Humidity=" + String(Humidity) + "";
  Serial.print("httpRequestData: ");
  Serial.println(httpRequestData);
  
  // You can comment the httpRequestData variable above
  // then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
  //String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&location=Office&Brightness=24.75&temperature=49.54&Humidity=1005.14";

  // Send HTTP POST request
  int httpResponseCode = http.POST(httpRequestData);
    
  // If you need an HTTP request with a content type: text/plain
  //http.addHeader("Content-Type", "text/plain");
  //int httpResponseCode = http.POST("Hello, World!");
  
  // If you need an HTTP request with a content type: application/json, use the following:
  //http.addHeader("Content-Type", "application/json");
  //int httpResponseCode = http.POST("{\"Voltage\":\"19\",\"Curent\":\"67\",\"Power\":\"78\"}");
      
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  }
  else 
  {
    Serial.println("WiFi Disconnected");
  }     

}

void take_constance(uint32_t time)
{
  uint64_t counter=0,counter2=10;
  float Humidity=0,temperature=0;
  while(counter<time)
  {
      // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    Humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temperature = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float FahrenheitTemp = dht.readTemperature(true);
        // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(FahrenheitTemp, Humidity);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(temperature, Humidity, false);

    // Check if any reads failed and exit early (to try again).
    if (isnan(Humidity) || isnan(temperature) || isnan(FahrenheitTemp)) 
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      delay(4000);
      return;
    }
    counter++;
    delay(1000);


    if((temperature<(Temp_start_Fan+5))&&(temperature>=Temp_start_Fan))
      FAN_PWM(175);
    else if((temperature<(Temp_start_Fan+10))&&(temperature>=(Temp_start_Fan+5)))
      FAN_PWM(200);
    else if((temperature<(Temp_start_Fan+15))&&(temperature>=(Temp_start_Fan+10)))
      FAN_PWM(225);
    else if(temperature>=(Temp_start_Fan+15))
      FAN_PWM(250);

    if(temperature>=Temp_Max)
    { 
      Send_constance(Humidity,temperature,((Brightness*100)/255));
      FAN_PWM(250);
      scenario_read=false;
      blocked=600000;
    }
    
    if(counter==counter2)
    {
      counter2+=10;
      Serial.print("Sensor: ");Serial.println(sensorLocation);
      Serial.print("Temperature:   "); Serial.print(temperature); Serial.println(" °C");
      Serial.print("Brightness: "); Serial.print(((Brightness*100)/255)); Serial.println(" %");
      Serial.print("Humidity:  "); Serial.print(Humidity); Serial.println(" %");
      Serial.println("");
    }
  }
  Send_constance(Humidity,temperature,((Brightness*100)/255));
}

void Init_OTA(String myesp32, String admin)
{
// Port defaults to 3232
  ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(myesp32.c_str());

  // No authentication by default
  ArduinoOTA.setPassword(admin.c_str());

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  }