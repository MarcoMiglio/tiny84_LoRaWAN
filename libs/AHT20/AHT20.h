#ifndef AHT20_H
#define AHT20_H

#include <TinyWireM.h>  // Include TinyWireM for ATTINY I2C

class AHT20 {
  public:
    // Constructor
    AHT20();
    
    /*!
    * @brief Initialize AHT20
    * @return Returns 1 when successful
    */
    bool begin();
    
    /*!
    * @brief Read humidity and temperature raw values
    * @return Returns 1 if 6 bytes are correctly read
    */
    bool readData();

    /*!
    * @brief Function to get temperature (in Celsius)
    * @return Returns temperature value (float, °C)
    */
    float getTemperature();
    
    /*!
    * @brief Function to get humidity (in percentage)
    * @return Returns relative humidity value (float, %)
    */
    float getHumidity();

    /*!
    * @brief Function to reset sensor and release I2C USI
    */
    void reset();


  private:
    
    /*!
    * @brief Initialize I2C and send start sequence to AHT20
    * @return Returns 1 when successful
    */
    bool initialize();

    /*!
    * @brief Triggers humidity and temperature measurement
    * @return Returns 1 when successful
    */
    bool triggerMeasurement();

    /*!
    * @brief Reads status reg (0x71)
    * @return Returns status reg byte
    */
    uint8_t readStatus();

    // Raw humidity and temperature
    uint32_t humidity_raw;
    uint32_t temperature_raw;

    // Address of the AHT20 sensor
    const uint8_t AHT20_ADDRESS = 0x38;
};

#endif