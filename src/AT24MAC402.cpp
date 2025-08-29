#include "AT24MAC402.h"
#define EUI48_ADDRESS 0x9A
#define EUI64_ADDRESS 0x98
#define UUID_ADDRESS 0x80
#define I2C_ADDRESS 0x50
#define I2C_ADDRESS_EXTENDED 0x58

AT24MAC402::AT24MAC402(uint8_t a0, uint8_t a1, uint8_t a2) {
	i2cAddress = I2C_ADDRESS;
	bitWrite(i2cAddress, 0, a0);
	bitWrite(i2cAddress, 1, a1);
	bitWrite(i2cAddress, 2, a2);
	i2cAddressExt = I2C_ADDRESS_EXTENDED;
	bitWrite(i2cAddressExt, 0, a0);
	bitWrite(i2cAddressExt, 1, a1);
	bitWrite(i2cAddressExt, 2, a2);
	}

void AT24MAC402::begin(TwoWire &wire) {
	this->wire = &wire;
	this->wire->begin();
	}

void AT24MAC402::writeByte(uint8_t address, uint8_t data) {
	if (address <= 255) {
		wire->beginTransmission(i2cAddress);
		wire->write(address);
		wire->write(data);
		wire->endTransmission();
		delay(5);
		}
	}

uint8_t AT24MAC402::readByte(uint8_t address) {
	if (address <= 255) {
		uint8_t data;
		wire->beginTransmission(i2cAddress);
		wire->write(address);
		wire->endTransmission();
		wire->requestFrom(i2cAddress, 1);
		return wire->read();
		}
	return 0;
	}

void AT24MAC402::readMac(uint8_t mac[]) {
	wire->beginTransmission(i2cAddressExt);
	wire->write(EUI48_ADDRESS); // Location of the EUI-48
	wire->endTransmission();
	wire->requestFrom(i2cAddressExt, 6);
	for (uint8_t i = 0; i < 6; i++) {
		mac[i] = wire->read();
		}
	}


uint8_t* AT24MAC402::readMac() {
	static uint8_t mac[6];
	readMac(mac);
	return mac;
	}

void AT24MAC402::readEUI64(uint8_t eui64[]) {
	wire->beginTransmission(i2cAddressExt);
	wire->write(EUI64_ADDRESS); // Location of the EUI-48
	wire->endTransmission();
	wire->requestFrom(i2cAddressExt, 8);
	for (uint8_t i = 0; i < 8; i++) {
		eui64[i] = wire->read();
		}
	}

uint8_t* AT24MAC402::readEUI64() {
	static uint8_t eui64[8];
	readEUI64(eui64);
	return eui64;
	}

void AT24MAC402::readUUID(uint8_t uuid[]) {
	wire->beginTransmission(i2cAddressExt);
	wire->write(UUID_ADDRESS); // Location of the EUI-48
	wire->endTransmission();
	wire->requestFrom(i2cAddressExt, 16);
	for (uint8_t i = 0; i < 16; i++) {
		uuid[i] = wire->read();
		}
	}

uint8_t* AT24MAC402::readUUID() {
	static uint8_t uuid[16];
	readUUID(uuid);
	return uuid;
	}

void MACtoString(uint8_t mac[], char macString[]) {
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

char* MACtoString(uint8_t mac[]) {
	static char macString[31];
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return macString;
	}

void UUIDtoString(uint8_t uuid[], char uuidString[]) {
	sprintf(uuidString, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
	uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	}

char* UUIDtoString(uint8_t uuid[]) {
	static char uuidString[70];
	sprintf(uuidString, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
	uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	return uuidString;
	}