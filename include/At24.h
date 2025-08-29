// Arduino library for Microchip AT24MAC402/602 EEPROM with build in EUI-48/EUI-64 and Serialnumber
/*
MIT License

Copyright (c) 2023 Stefan Staub

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// #pragma once

#ifndef __AT24_H
#define __AT24_H

#include "AT24MAC402.h"

#define SDA_PIN              21
#define SCL_PIN              22
#define A0_PIN                LEVEL_GND
#define A1_PIN                LEVEL_GND
#define A2_PIN                LEVEL_GND
#define LEVEL_GND             0
#define LEVEL_HIGH            1



void i2cInit(void);
void  Config_eeprom(void);
void readMemMacAddress(uint8_t u8Mac[]);
void readMemSerialNumber(uint8_t u8SerialNumber[]);
void readMem(uint8_t *u8PtrDest, uint8_t u8StartAdress, uint16_t u16DataLen);
void writeMem(const uint8_t *u8PtrSrc, uint8_t u8StartAdress, uint16_t u16DataLen);

#endif