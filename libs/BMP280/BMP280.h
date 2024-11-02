// BMP280.h

#ifndef BMP280_H
#define BMP280_H

#include <TinyWireM.h>  // For ATtiny I2C communication
#include <stdint.h>

class BMP280 {
public:

    /*****************************************************************/
    /* ENUMERATIONS                                                  */
    /*****************************************************************/


    /*
     *  Define temperature meas-unit for user returned value
     */
    enum TempUnit {
      TempUnit_Celsius,
      TempUnit_Fahrenheit
    };

    /*
     *  Define pressure meas-unit for user returned value
     */
    enum PresUnit {
      PresUnit_Pa,
      PresUnit_hPa,
      PresUnit_inHg,
      PresUnit_atm,
      PresUnit_bar,
      PresUnit_torr,
      PresUnit_psi
    };

    /*
     *  See datasheet, in general higher OSR provides better resolution
     *  but involves longer measurement time (i.e. higher consumption)
     */
    enum OSR {
      OSR_X1 =  1,
      OSR_X2 =  2,
      OSR_X4 =  3,
      OSR_X8 =  4,
      OSR_X16 = 5
    };

    /*
     *  - sleep mode: BMP can be read, but measurements are stopped
     *  - forced mode: single shot measurement, then back to sleep
     *  - normal: swap between measurment and standby mode
     */
    enum Mode {
      Mode_Sleep  = 0,
      Mode_Forced = 1,
      Mode_Normal = 3
    };

    /*
     *  Possible standby times for standby duration 
     *  while operating in normal mode
     */
    enum StandbyTime {
      StandbyTime_500us   = 0,
      StandbyTime_62500us = 1,
      StandbyTime_125ms   = 2,
      StandbyTime_250ms   = 3,
      StandbyTime_50ms    = 4,
      StandbyTime_1000ms  = 5,
      StandbyTime_10ms    = 6,
      StandbyTime_20ms    = 7
    };

    /*
     *  Configurations for the IIR filter: to save power, keep it off
     */
    enum Filter {
      Filter_Off = 0,
      Filter_2   = 1,
      Filter_4   = 2,
      Filter_8   = 3,
      Filter_16  = 4
    };

    /*
     *  Needed to specify wchich interface is used (I2C or SPI)
     */ 
    enum SpiEnable{
      SpiEnable_False = 0,
      SpiEnable_True = 1
    };

    // Struct to hold configurations for OSR, IIR and operating mode
    struct Settings {
      Settings(
          OSR _tosr       = OSR_X1,
          OSR _posr       = OSR_X1,
          Mode _mode      = Mode_Forced,
          StandbyTime _st = StandbyTime_1000ms,
          Filter _filter  = Filter_Off,
          SpiEnable _se   = SpiEnable_False
        ):  tempOSR(_tosr),
            presOSR(_posr),
            mode(_mode),
            standbyTime(_st),
            filter(_filter),
            spiEnable(_se) {}

      OSR tempOSR;
      OSR presOSR;
      Mode mode;
      StandbyTime standbyTime;
      Filter filter;
      SpiEnable spiEnable;
    };


    /*****************************************************************/
    /* INIT FUNCTIONS                                                */
    /*****************************************************************/

    BMP280(const Settings& my_settings = Settings(), uint8_t address = 0x76); // I2C address if SDO is connected to Vdd

    /*!
    * @brief Initialize BMP280
    * @return Returns 1 when successful
    */
    bool begin();


    /*****************************************************************/
    /* ENVIRONMENTAL FUNCTIONS                                       */
    /*****************************************************************/

    /*!
    * @brief Read temperature and return float in readeble format
    * @return temperature in float format
    */
    float readTemperature(TempUnit unit);

    /*!
    * @brief Read pressure and return float in readeble format
    * @return pressure in float format
    */
    float readPressure(PresUnit unit);

private:

    /*****************************************************************/
    /* CONSTANTS                                                     */
    /*****************************************************************/

    static const uint8_t CHIP_MODEL = 0x58;

    static const uint8_t CTRL_MEAS_ADDR = 0xF4;
    static const uint8_t CONFIG_ADDR    = 0xF5;
    static const uint8_t PRESS_ADDR     = 0xF7;
    static const uint8_t TEMP_ADDR      = 0xFA;
    static const uint8_t TEMP_DIG_ADDR  = 0x88;
    static const uint8_t PRESS_DIG_ADDR = 0x8E;
    static const uint8_t ID_ADDR        = 0xD0;

    static const uint8_t TEMP_DIG_LENGTH    = 6;
    static const uint8_t PRESS_DIG_LENGTH   = 18;
    static const uint8_t DIG_LENGTH         = 24;
    static const uint8_t SENSOR_DATA_LENGTH = 6;


    /*****************************************************************/
    /* VARIABLES                                                     */
    /*****************************************************************/

    Settings m_settings; // store all the configurations for the IIR, OSR and operating mode

    uint8_t _address;    // specify your device address according to SDO connection

    // Calibration parameters
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;

    /*****************************************************************/
    /* WORKER FUNCTIONS                                              */
    /*****************************************************************/

    /*!
    * @brief Verify chipID (should be 0x58)
    * @return 1 if sucessfull
    */
    bool readChipID();

    /*!
     * @brief Calculate configuration registers values based on settings
     * @return void
     */
    void CalculateRegisters(uint8_t& ctrlMeas, uint8_t& config);

    /*!
     * @brief Write the settings to the chip
     * @return void
     */
    void WriteSettings();

    /*!
    * @brief Read calibration data for Temp and Pressure raw readings
    * @return 1 if sucessfull
    */
    bool readCalibrationData();

    /*!
    * @brief Return final temperature value based on the BMP calibration process
    * @return float correpsonding to the measured temperature
    */
    float CalculateTemperature(int32_t raw, int32_t& t_fine, TempUnit unit = TempUnit_Celsius);

    /*!
    * @brief Return final pressure value based on the BMP calibration process
    * @return float correpsonding to the measured pressure
    */
    float CalculatePressure(int32_t adc_P, int32_t t_fine, PresUnit unit);

    /*!
    * @brief Read raw temperature and pressure values:
    * @return 1 if sucessfull
    */
    bool ReadData(int32_t data[SENSOR_DATA_LENGTH]);

    /*!
    * @brief Read the specified amount of bytes from register "reg"
    * @return 1 if sucessfull
    */
    bool readRegister(uint8_t reg, uint8_t *buff, uint8_t length);

    /*!
    * @brief Write specified byte into the register "reg"
    * @return void
    */
    void writeRegister(uint8_t reg, uint8_t value);
};

#endif