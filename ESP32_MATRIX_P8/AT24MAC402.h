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

#ifndef AT24MAC402_H
#define AT24MAC402_H

#include "Arduino.h"
#include "Wire.h"

class AT24MAC402 {
	public:
		/**
		 * @brief Construct a new AT24MAC402 object
		 * 
		 * @param i2cAddressExtMask 
		 */
		AT24MAC402(uint8_t a0 = 0, uint8_t a1 = 0, uint8_t a2 = 0);

		/**
		 * @brief Start the Wire connectopn
		 * 
		 */
		void begin(TwoWire &wire = Wire);

		/**
		 * @brief Write a single byte to the EEPROM
		 * 
		 * @param address 
		 * @param data 
		 */
		void writeByte(uint8_t address, uint8_t data);

		/**
		 * @brief Read a single byte from the EEPROM
		 * 
		 * @param address 
		 * @return uint8_t 
		 */
		uint8_t readByte(uint8_t address);

		/**
		 * @brief Read the MAC address 
		 * 
		 * @param macaddr 
		 */
		void readMac(uint8_t mac[]);
	
		/**
		 * @brief Read the MAC address
		 * 
		 * @return uint8_t* MAC address
		 */
		uint8_t* readMac();

		/**
		 * @brief Read the EUI64 address (only for AT24MAC602)
		 * 
		 * @param eui64address 
		 */
		void readEUI64(uint8_t eui64[]);

		/**
		 * @brief Read the EUI64 address (only for AT24MAC602)
		 * 
		 * @return uint8_t* EUI64 address
		 */
		uint8_t* readEUI64();


		void readUUID(uint8_t uuid[]);


		uint8_t* readUUID();

	private:
		TwoWire *wire;
		uint8_t i2cAddress;
		uint8_t i2cAddressExt;
	};


/**
 * @brief converts the MAC to a string
 * 
 * @param mac 
 * @param macString 
 */
void MACtoString(uint8_t mac[], char macString[]);

/**
 * @brief converts the MAC to a string
 * 
 * @param mac 
 * @return char* MAC string
 */
char* MACtoString(uint8_t mac[]);

/**
 * @brief converts the uuid / UUID to a string
 * 
 * @param uuid 
 * @param uuidString 
 */
void UUIDtoString(uint8_t uuid[], char uuidString[]);

/**
 * @brief converts the uuid / UUID to a string
 * 
 * @param uuid 
 * @return char* uuid string 
 */
char* UUIDtoString(uint8_t uuid[]);

#endif