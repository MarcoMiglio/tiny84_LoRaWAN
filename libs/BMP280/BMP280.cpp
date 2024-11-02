// BMP280.cpp

#include "BMP280.h"



/* Constructor: initializes device address */
BMP280::BMP280(const Settings& my_settings, uint8_t address) : m_settings(my_settings),  _address(address) {}

/* 
    ############## PUBLIC FUNCTIONS ###############
 */

/*
 * Initialize I2C bus, verify chip ID and readTrim data for calibration
 */
bool BMP280::begin(){
  // start I2C bus
  TinyWireM.begin();

  // verify chip ID
  bool stat = readChipID();
  
  // read trim registers and save calibration values
  stat &= readCalibrationData();

  // write settings defined in the Settings struct:
  WriteSettings();

  return stat;
}


/*
 * use this function to read temeprature value
 * Argument can be either TempUnit_Celsius or TempUnit_Fahrenheit (see .h file)
 */
float BMP280::readTemperature(TempUnit unit) {
   int32_t data[SENSOR_DATA_LENGTH];
   int32_t t_fine;

   if(!ReadData(data)){ return NAN; }

   uint32_t rawTemp   = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);

   return CalculateTemperature(rawTemp, t_fine, unit);
}


/*
 * use this function to read pressure value
 * Argument PresUnit can be one of the following:
 * PresUnit_Pa
 * PresUnit_hPa
 * PresUnit_in
 * PresUnit_atm
 * PresUnit_bar
 * PresUnit_torr
 * PresUnit_psi
 */
float BMP280::readPressure(PresUnit unit) {
   int32_t data[SENSOR_DATA_LENGTH];
   int32_t t_fine;

   if(!ReadData(data)){ return NAN; }

   uint32_t rawTemp       = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
   uint32_t rawPressure = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);

   CalculateTemperature(rawTemp, t_fine);

   return CalculatePressure(rawPressure, t_fine, unit);
}

/* 
  ############## PRIVATE FUNCTIONS ###############
*/


/*
 *  Read ChipID from chip_id register (should be 0x58 for BMP280)
 */
bool BMP280::readChipID(){
  uint8_t id[1];

  bool stat = readRegister(ID_ADDR, &id[0], 1);

  return (stat && (id[0] == CHIP_MODEL));
}


/*
 *  Calculate values to be written to configuration registers
 *  based on the settings structure (see m_settings in BMP280.h)
 */
void BMP280::CalculateRegisters(uint8_t& ctrlMeas, uint8_t& config) {
   // ctrl_meas register. (ctrl_meas[7:5] = temperature oversampling rate, ctrl_meas[4:2] = pressure oversampling rate, ctrl_meas[1:0] = mode.)
   ctrlMeas = ((uint8_t) m_settings.tempOSR << 5) | ((uint8_t) m_settings.presOSR << 2) | (uint8_t) m_settings.mode;
  
   // config register. (config[7:5] = standby time, config[4:2] = filter, ctrl_meas[0] = spi enable.)
   config = ((uint8_t) m_settings.standbyTime << 5) | ((uint8_t) m_settings.filter << 2) | (uint8_t) m_settings.spiEnable;
}


/*
 *  Write the settings defined in your settings structure to the chip
 */
void BMP280::WriteSettings(){
   uint8_t ctrlMeas, config;

   CalculateRegisters(ctrlMeas, config);

   writeRegister(CTRL_MEAS_ADDR, ctrlMeas);
   writeRegister(CONFIG_ADDR, config);
}


/*
 * Read calibration data needed to convert raw temeprature and 
 * Pressure measurements
 */
bool BMP280::readCalibrationData(){
  uint8_t m_dig[DIG_LENGTH];

  bool stat = readRegister(TEMP_DIG_ADDR, &m_dig[0], TEMP_DIG_LENGTH);

  stat &= readRegister(PRESS_DIG_ADDR, &m_dig[TEMP_DIG_LENGTH], PRESS_DIG_LENGTH);

  // set calibration coefficients:
  dig_T1 = (m_dig[1] << 8) | m_dig[0];
  dig_T2 = (m_dig[3] << 8) | m_dig[2];
  dig_T3 = (m_dig[5] << 8) | m_dig[4];

  dig_P1 = (m_dig[7]   << 8) | m_dig[6];
  dig_P2 = (m_dig[9]   << 8) | m_dig[8];
  dig_P3 = (m_dig[11] << 8) | m_dig[10];
  dig_P4 = (m_dig[13] << 8) | m_dig[12];
  dig_P5 = (m_dig[15] << 8) | m_dig[14];
  dig_P6 = (m_dig[17] << 8) | m_dig[16];
  dig_P7 = (m_dig[19] << 8) | m_dig[18];
  dig_P8 = (m_dig[21] << 8) | m_dig[20];
  dig_P9 = (m_dig[23] << 8) | m_dig[22];

  return stat;
}


/*
 * Calculate temperature based on raw value and the calibration process
 * described on the BMP280 datasheet 
 */
float BMP280::CalculateTemperature(int32_t raw, int32_t& t_fine, TempUnit unit) {
  // Code based on calibration algorthim provided by Bosch.
  int32_t var1, var2, final;
 
  var1 = ((((raw >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
  var2 = (((((raw >> 4) - ((int32_t)dig_T1)) * ((raw >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;

  t_fine = var1 + var2;

  final = (t_fine * 5 + 128) >> 8;

  return unit == TempUnit_Celsius ? final/100.0 : final/100.0*9.0/5.0 + 32.0;
}


/*
 * Returns pressure compensated using the procedure described in the DS
 */
float BMP280::CalculatePressure(int32_t adc_P, int32_t t_fine, PresUnit unit) {
  typedef int32_t BMP280_S32_t;  // 32-bit signed integer
  typedef uint32_t BMP280_U32_t;  // 32-bit unsigned integer

  BMP280_S32_t var1, var2;
  BMP280_U32_t p;

  var1 = (((BMP280_S32_t)t_fine)>>1) - (BMP280_S32_t)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BMP280_S32_t)dig_P6);
  var2 = var2 + ((var1*((BMP280_S32_t)dig_P5))<<1);
  var2 = (var2>>2)+(((BMP280_S32_t)dig_P4)<<16);
  var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BMP280_S32_t)dig_P2) * var1)>>1))>>18;
  var1 =((((32768+var1))*((BMP280_S32_t)dig_P1))>>15);

  if (var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }

  p = (((BMP280_U32_t)(((BMP280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
  
  if (p < 0x80000000) {
    p = (p << 1) / ((BMP280_U32_t)var1);
  } else {
    p = (p / (BMP280_U32_t)var1) * 2;
  }

  var1 = (((BMP280_S32_t)dig_P9) * ((BMP280_S32_t)(((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((BMP280_S32_t)(p>>2)) * ((BMP280_S32_t)dig_P8))>>13;

  p = (BMP280_U32_t)((BMP280_S32_t)p + ((var1 + var2 + dig_P7) >> 4));

  // Conversion units courtesy of www.endmemo.com.
  switch(unit){
    case PresUnit_hPa: /* hPa */
        p /= 100.0;
        break;
    case PresUnit_inHg: /* inHg */
        p /= 3386.3752577878;          /* p pa * 1inHg/3386.3752577878Pa */
        break;
    case PresUnit_atm: /* atm */
        p /= 101324.99766353;          /* p pa * 1 atm/101324.99766353Pa */
        break;
    case PresUnit_bar: /* bar */
        p /= 100000.0;                 /* p pa * 1 bar/100kPa */
        break;
    case PresUnit_torr: /* torr */
        p /= 133.32236534674;           /* p pa * 1 torr/133.32236534674Pa */
        break;
    case PresUnit_psi: /* psi */
        p /= 6894.744825494;            /* p pa * 1psi/6894.744825494Pa */
        break;
    default: /* Pa (case: 0) */
        break;
  }
  
  return p;
}


/*
 * Read raw teperature and pressure values in a single operation
 * starting from pressure data register
 */
bool BMP280::ReadData(int32_t data[SENSOR_DATA_LENGTH])
{
  bool status;
  uint8_t buffer[SENSOR_DATA_LENGTH];

  // For forced mode we need to write the mode to BMP280 register before reading
  if (m_settings.mode == Mode_Forced) {
    WriteSettings();
  }

  // Registers are in order. So we can start at the pressure register and read 6 bytes.
  status = readRegister(PRESS_ADDR, buffer, SENSOR_DATA_LENGTH);

  for(int i = 0; i < SENSOR_DATA_LENGTH; ++i) {
    data[i] = static_cast<int32_t>(buffer[i]);
  }

  return status;
}


/*
 * Request length-bytes from the specified register
 * If length > 1 the bmp280 will start reading from the register "reg"
 * after which the slave sends out data from auto-incremented register addresses
 * until the specified number of bytes has been transmitted
 */
bool BMP280::readRegister(uint8_t reg, uint8_t *buff, uint8_t length){
  // Write to register  
  TinyWireM.beginTransmission(_address);
  TinyWireM.write(reg);
  TinyWireM.endTransmission();

  // request bytes:
  TinyWireM.requestFrom(_address, length);

  if(TinyWireM.available() < length) return false;

  uint8_t idx = 0;
  while(TinyWireM.available() > 0) {
    buff[idx++] = TinyWireM.read();
  }

  return idx == length;
}


/*
 * Request length-bytes from the specified register
 * If length > 1 the bmp280 will start reading from the register "reg"
 * after which the slave sends out data from auto-incremented register addresses
 * until the specified number of bytes has been transmitted
 */
void BMP280::writeRegister(uint8_t reg, uint8_t value){
  TinyWireM.beginTransmission(_address);
  TinyWireM.write(reg);
  TinyWireM.write(value);
  TinyWireM.endTransmission();
}
