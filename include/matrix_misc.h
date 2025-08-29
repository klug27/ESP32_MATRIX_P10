
#pragma once

#ifndef __MATRIX_MISC_H
#define __MATRIX_MISC_H

#include "Arduino.h"
#include "DHT.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h> 

#define pinDATA 32 // SDA, or almost any other I/O pin
#define FAN 17
#define LED_BUILTIN 2
#define LDR_PIN  33


// the number of the LED pin
const int ledPin = 17;  // 16 corresponds to GPIO16

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
 

void FAN_PWM(int dutyCycle);
void take_constance(uint32_t time);
void Send_constance(float Humidity,float temperature,int Brightness );
void dht22_init();
void FAN_Init();
void LDR_Init();
void FAN_LOW();
void FAN_HIGH();
void Init_OTA(String myesp32, String admin);

#endif // __MATRIX_MISC_H

 
