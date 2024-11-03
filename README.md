# LoRaWAN RFM95 - ATTiny84 

This repository provides libraries and example code for integrating LoRaWAN communication and various sensors with the ATTiny84 MCU, using the RFM95 LoRa module. 
The code is optimized for low power consumption, making it suitable for battery-powered applications.

## Directory Structure

- **libs/** - Contains libraries for the sensors and LoRaWAN integration.
  - **RFM95/** - Library for the RFM95 LoRa module based on LeoKorbee work [here](https://gitlab.com/iot-lab-org/ATtiny84_low_power_LoRa_node_OOP).
  - **LoRaWAN/** - Library for LoRaWAN communication based on LeoKorbee work [here](https://gitlab.com/iot-lab-org/ATtiny84_low_power_LoRa_node_OOP).
  - **AHT20/** - Library for the AHT20 temperature and humidity sensor, based on TinyWireM library with minimal memory requirement.
  - **BMP280/** - Library for the BMP280 barometric pressure sensor, based on TinyWireM library with minimal memory requirement.

    **Installation**: Each library should be installed in the specified libraries path (or directly inside the working directory). 

- **tiny84_RFM95/** - Contains a working example that demonstrates using the LoRaWAN RFM95 shield with the ATTiny84 MCU.
  - This example illustrates key functions for sending data over LoRaWAN and managing the ATTiny84's sleep state to optimize power consumption.
  - **Key Functions**:
    - **LoRaWAN Communication**: Explanation of how to initialize and send data using LoRaWAN.
    - **Power Management**: Guide on configuring the ATTiny84 to enter a low-power sleep state when not actively transmitting.

- **examples/** - Demonstrates how to integrate the sensors with the ATTiny84 and RFM95 setup.
  - **aht20_example/** - Code for integrating the AHT20 sensor to collect temperature and humidity data.
  - **bmp280_example/** - Code for integrating the BMP280 sensor for pressure data.

    **Customization**: These examples highlight sections in `tiny84_RFM95` that can be modified to send custom sensor data.
    Each example includes instructions on adapting the code for different sensor configurations.

## Getting Started

### Prerequisites

1. **Hardware**:
   - Arduino UNO board (to program ATTiny84)
   - ATTiny84 MCU
   - RFM95 LoRaWAN shield
   - AHT20 and/or BMP280 sensors (optional)

3. **Software**:
   - Arduino IDE
   - Support for ATTiny MCUs inside Arduino IDE (additional details [here]([https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json](https://github.com/MarcoMiglio/tiny84_LoRaWAN/wiki/Programming-ATTiny84-using-Arduino-IDE)))
   - RFM95 & LoRaWAN libraries are based on [LeoKorbee](https://gitlab.com/iot-lab-org/ATtiny84_low_power_LoRa_node_OOP).
     Few adjustements were done to include adjustable SF and Transmission power levels

### Installation

1. Clone the repository:
   ```
   git clone git@github.com:MarcoMiglio/tiny84_LoRaWAN.git
   cd your-repo-name
   ```
2. Install the libraries in the path /libs/* in your /libs folder (or directly inside your working directory).


### Usage

- **tiny84_RFM95 Example**:
  1. Open `tiny84_RFM95/tiny84_RFM95.ino` in Arduino IDE.
  2. Update your secconfig.h file (with either internal server or TTN session/application keys).
     Visit the Wiki page for additioal details about setting up a TTN end device.
  3. In the main file configure LoRaWAN and power management:
     ```
        /* Define LoRaWAN layer */
        LoRaWAN lora = LoRaWAN(rfm);
        uint32_t Frame_Counter_Tx = 0x0000;
        const byte SF = 7;        // Set your SF
        uint8_t PA_boost_on = 1;  // Still testing, do not modify this byte...
        uint8_t power_level = 14; // Set your Tx Power
     ```
  4. Update the code inside this function (bottom of the page):
     ```
        void setup_unused_pins(){ ... }
     ```
     Comment-out the pins you are actively using. (RETAIN all existing comments; do not delete any lines that are already commented).
  5. Feel free to add your personal code in the sections delimited by:
     ```
        /* USER CODE BEGIN */
          your code here....
        /* USER CODE END */
     ```
     Visit the /examples folder for additional details.
  
  6. Upload the code to the ATTiny84. Visit wiki page for additional details about programming ATTiny MCUs through UNO boards.
  7. Use the following default connections:
 
    | ATTiny84 Pin | RFM95 Pin | Description                |
    |--------------|-----------|----------------------------|
    | PA5 (DO)     | MOSI      | Master Data Output (TWI)   |
    | PA6 (DI)     | MISO      | Master Data Input (TWI)    |
    | PA4          | SCK       | Clock Signal               |
    | PA1          | NSS       | Chip Select                |
    | PA0          | DIO0      | Interrupt (for LoRaWAN)    |
    | GND          | GND       | Ground                     |
    | VCC          | VCC       | Power Supply               |



- **Sensor Examples**:
1. To add sensor data, refer to `examples/aht20_example` or `examples/bmp280_example`.
2. Modify the `tiny84_RFM95` code to include the necessary sensor headers and data collection logic.

Additional details will be added in the Wiki page.
