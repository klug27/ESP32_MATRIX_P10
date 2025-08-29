#include "At24.h"

#define SDA_PIN              21
#define SCL_PIN              22
#define A0_PIN                LEVEL_GND
#define A1_PIN                LEVEL_GND
#define A2_PIN                LEVEL_GND
#define LEVEL_GND             0
#define LEVEL_HIGH            1 
#define MEMORY_ADDR           (0x50)

static AT24MAC402 sAt24mac402(A0_PIN, A1_PIN, A2_PIN); // all address pins to GND
static bool i2cInitialized = false;
static bool memInitialized = false;

void i2cInit(void)
{
  if (i2cInitialized == false)
  {
    // set SDA and SCL pins
    Wire.setPins(SDA_PIN,SCL_PIN);
    // initialize i2c instance
    Wire.begin();
    // set flag
    i2cInitialized = true;
  }
}

void Config_eeprom(void)
{
  if (i2cInitialized == true)
  {
    uint8_t u8DeviceAddr = MEMORY_ADDR | (A2_PIN << 2) | (A1_PIN << 1) | (A0_PIN << 0);
    // send the device address and wait until it recognizes this address
    Wire.beginTransmission(u8DeviceAddr); 
    // Check if device is connected to the bus
    if (Wire.endTransmission () != 0)
    {
      Serial.println("FATAL: Fail to communicate with the external eeprom memory. Check wire connection and address pin A0, A1, A2");
    }
    else
    {
      // initialize AT24 memory
      sAt24mac402.begin();
      // set flag
      memInitialized = true;
    }
  }
  else
  {
    Serial.println("FATAL: Attempting to initialize EEPROM memory, while I2C is not initialized"); 
  }
}

void readMemMacAddress(uint8_t u8Mac[])
{
  if (memInitialized == true)
  {
    sAt24mac402.readMac(u8Mac);
  }
}


void readMemSerialNumber(uint8_t u8SerialNumber[])
{
  if (memInitialized == true)
  {
    sAt24mac402.readUUID(u8SerialNumber);
  }
}


void readMem(uint8_t *u8PtrDest, uint8_t u8StartAdress, uint8_t u8DataLen)
{
  if ((u8DataLen <= 128) && (u8DataLen > 0) && (u8PtrDest != nullptr))
  {
    for (uint8_t u8Idx= 0; u8Idx < u8DataLen; u8Idx++)
    {
      // fetch data
      u8PtrDest[u8Idx] = sAt24mac402.readByte(u8StartAdress);
      
      // increment adress
      u8StartAdress++;
    }
  }
}


void writeMem(const uint8_t *u8PtrSrc, uint8_t u8StartAdress, uint8_t u8DataLen)
{
  if ((u8DataLen <= 128) && (u8DataLen > 0) && (u8PtrSrc != nullptr))
  {
    for (uint8_t u8Idx= 0; u8Idx < u8DataLen; u8Idx++)
    {
      // write data in the memory
      sAt24mac402.writeByte(u8StartAdress, u8PtrSrc[u8Idx]);

      // increment adress
      u8StartAdress++;
    }
  }
}
