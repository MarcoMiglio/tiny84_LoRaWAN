#include "TinyWireM.h"

#include <AHT20.h>
#include "Arduino.h"


// construtor definition
AHT20::AHT20(){

}

// Start I2C and initialize AHT20
bool AHT20::begin(){
  TinyWireM.begin();
  return initialize();
}

/* 
 * Trigger measurement mode and convert raw humidity and temperature values
 * The corresponding measurement are then available through getTemperature()
 * and getHumidity() functions.
 */
bool AHT20::readData(){
  // Trigger measurement mode:
  if(!triggerMeasurement()) return false;

  // read six bytes raw measurements:
  uint8_t buff[6];
  TinyWireM.requestFrom(AHT20_ADDRESS, 6);

  // check if all 6 bytes are ready:
  if (TinyWireM.available() < 6) return false;

  for (uint8_t i = 0; i < 6; i++) {
    if (TinyWireM.available() > 0) {
      buff[i] = TinyWireM.read();
    }
  }

  // ignore crc (byte7 not requested)
  // ignore byte1 dedicated to status reg

  // Extract humidity and temperature values:
  humidity_raw = buff[1];
  humidity_raw <<= 8;
  humidity_raw |= buff[2];
  humidity_raw <<= 4;
  humidity_raw |= buff[3] >> 4;

  temperature_raw = buff[3] & 0x0F;
  temperature_raw <<= 8;
  temperature_raw |= buff[4];
  temperature_raw <<= 8;
  temperature_raw |= buff[5];

  return true;
}

/* 
 * Combine raw informations to return relative humidity (in %)
 */
float AHT20::getHumidity(){
  return ((float)humidity_raw * 100/1048576);
}

/* 
 * Combine raw informations to return temperature (in °C)
 */
float AHT20::getTemperature(){
  return ((float)temperature_raw * 200/1048576) - 50;
}

/* 
 * Reset AH20 and close I2C interface
 */
void AHT20::reset(){
  /*
   * Avoid the subsequent instruction: by resetting the sensor 
   * the I2C resources are not correctly released
   */
  TinyWireM.beginTransmission(AHT20_ADDRESS);
  TinyWireM.write(0xBA);
  TinyWireM.endTransmission();

  // delay 20 ms to conclude reset process
  delay(20);
}
 

/* ############### PRIVATE FUNCTIONS ################## */
 
 /* 
  * Send initialization sequence:
  */
bool AHT20::initialize(){
  TinyWireM.beginTransmission(AHT20_ADDRESS);
  // Write initialization sequence {0xBE, 0x08, 0x00}
  TinyWireM.write(0xBE); 
  TinyWireM.write(0x08); 
  TinyWireM.write(0x00); 
  TinyWireM.endTransmission();

  // wait 10ms as suggested in the datasheet
  delay(10);
  uint8_t status = readStatus();

  // check if bit[3] is set
  return (status & 0x08) != 0;
}

/* 
 * Read AHT20 status reg (0x71)
 * bit7:BusyIndication (0 = free, 1 = busy in measurement mode)
 * bit6:Remained
 * bit5:Remained
 * bit4:Remained
 * bit3:CalibrationEnabled (1 = enabled,0 = uncalibrated)
 * bit2:Remained
 * bit1:Remained
 * bit0:Remained
 */
uint8_t AHT20::readStatus(){
  TinyWireM.beginTransmission(AHT20_ADDRESS);
  TinyWireM.write(0x71); // Contact status reg 0x71 
  TinyWireM.endTransmission();

  TinyWireM.requestFrom(AHT20_ADDRESS, 1);
  uint8_t status;
  while(TinyWireM.available() > 0) {
    status = TinyWireM.read();
  }

  return status;
}

// Trigger measurment mode. If logic high is returned humidity and temperature
// values can be read using the dedicated function
bool AHT20::triggerMeasurement(){
  TinyWireM.beginTransmission(AHT20_ADDRESS);
  // Write measurement sequence {0xAC, 0x33, 0x00}
  TinyWireM.write(0xAC); 
  TinyWireM.write(0x33); 
  TinyWireM.write(0x00); 
  TinyWireM.endTransmission();

  // wait 80ms as suggested in the DS
  delay(80);

  //verify measuremnt status:
  uint8_t status = readStatus();
  
  // bit[7] = 0 indicates measurement completed:
  if ((status & 0x80) == 0) return true;
  return false;
}

