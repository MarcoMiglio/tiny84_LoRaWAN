/*
  RFM95.cpp - Library for RFM95 LoRa module.
  Created by Leo Korbee, March 31, 2018.
  Released into the public domain.
  @license Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)
  Thanks to all the folks who contributed on the base of this code.
  (Gerben den Hartog, et al - Ideetron.nl)
*/


#include "Arduino.h"
#include "RFM95.h"

/*
      MODIFICA QUI PER COMPILARE PER IL TINY O PER ARDUINO UNO!!!
*/
#define MEGA 0
#define TINY 1

#if MEGA
  #include <SPI.h>
  static const SPISettings settings(10E6, MSBFIRST, SPI_MODE0);
#endif
#if TINY 
  #include <tinySPI.h>    //  MODIFICA  
#endif


// constructor
RFM95::RFM95(int DIO0, int NSS)
{
  _DIO0 = DIO0;
  _NSS = NSS;
  // init tinySPI
  SPI.setDataMode(SPI_MODE0);
  SPI.begin();
}


/*
*****************************************************************************************
* Description: Function used to initialize the RFM module on startup
*****************************************************************************************
*/
// MODIFICA: level and PA_boost_on added to allow to adjust power levels
void RFM95::init(uint8_t level, uint8_t PA_boost_on)
{
  // set pinmodes input/output
  pinMode(_NSS, OUTPUT);
  pinMode(_DIO0, INPUT);

  // NSS for starting and stopping communication with the RFM95 module
  digitalWrite(_NSS, HIGH);

  //Switch RFM to sleep
  RFM_Write(0x01,0x00);

  //Set RFM in LoRa mode
  RFM_Write(0x01,0x80);

  //Set RFM in Standby mode wait on mode ready
  RFM_Write(0x01,0x81);
  /*
  while (digitalRead(DIO5) == LOW)
  {
  }
  */
  delay(10);

  //Set carrair frequency
  // 868.100 MHz / 61.035 Hz = 14222987 = 0xD9068B
  RFM_Write(0x06,0xD9);
  RFM_Write(0x07,0x06);
  RFM_Write(0x08,0x8B);

  //PA pin (maximal power)
  //RFM_Write(0x09,0xFF);
  // MOD: Rather than setting max power, allows to adjust power level:
  RFM_Set_Tx_Power(level, PA_boost_on);

  //BW = 125 kHz, Coding rate 4/5, Explicit header mode
  RFM_Write(0x1D,0x72);

  // MOD here:
  //Spreading factor 7, PayloadCRC On
  //RFM_Write(0x1E,0xB4);              

  //Rx Timeout set to 37 symbols
  RFM_Write(0x1F,0x25);

  //Preamble length set to 8 symbols
  //0x0008 + 4 = 12
  RFM_Write(0x20,0x00);
  RFM_Write(0x21,0x08);

  //Low datarate optimization off AGC auto on
  RFM_Write(0x26,0x04);

  //Set LoRa sync word
  RFM_Write(0x39,0x34);

  //Set IQ to normal values
  RFM_Write(0x33,0x27);
  RFM_Write(0x3B,0x1D);

  //Set FIFO pointers
  //TX base adress
  RFM_Write(0x0E,0x80);
  //Rx base adress
  RFM_Write(0x0F,0x00);

  //Switch RFM to sleep
  RFM_Write(0x01,0x00);
}


/*
*****************************************************************************************
* Description : Funtion that writes a register from the RFM
*
* Arguments   : RFM_Address Address of register to be written
*         RFM_Data    Data to be written
*****************************************************************************************
*/

void RFM95::RFM_Write(unsigned char RFM_Address, unsigned char RFM_Data)
{
  #if MEGA
    SPI.beginTransaction(settings);  // MODIFICA
  #endif

  //Set NSS pin Low to start communication
  digitalWrite(_NSS,LOW);

  //Send Addres with MSB 1 to make it a write command
  SPI.transfer(RFM_Address | 0x80);
  //Send Data
  SPI.transfer(RFM_Data);

  //Set NSS pin High to end communication
  digitalWrite(_NSS,HIGH);

  #if MEGA
    SPI.endTransaction();  // MODIFICA
  #endif
}

/*
*****************************************************************************************
* Description : Funtion that reads a register from the RFM and returns the value
*
* Arguments   : RFM_Address Address of register to be read
*
* Returns   : Value of the register
*****************************************************************************************
*/

unsigned char RFM95::RFM_Read(unsigned char RFM_Address)
{
  unsigned char RFM_Data;

  #if MEGA
    SPI.beginTransaction(settings);  // MODIFICA
  #endif

  //Set NSS pin low to start SPI communication
  digitalWrite(_NSS,LOW);

  //Send Address
  SPI.transfer(RFM_Address);
  //Send 0x00 to be able to receive the answer from the RFM
  RFM_Data = SPI.transfer(0x00);

  //Set NSS high to end communication
  digitalWrite(_NSS,HIGH);

  #if MEGA
    SPI.endTransaction();  // MODIFICA
  #endif

  //Return received data
  return RFM_Data;
}

/*
*****************************************************************************************
* Description : Function for sending a package with the RFM
*
* Arguments   : *RFM_Tx_Package Pointer to arry with data to be send
*               Package_Length  Length of the package to send
*****************************************************************************************
*/
// MODIFICA: variabile "SF" dell func. Send_Package
void RFM95::RFM_Send_Package(unsigned char *RFM_Tx_Package, unsigned char Package_Length, uint8_t SF)
{
  unsigned char i;
  // unsigned char RFM_Tx_Location = 0x00;

  //Set RFM in Standby mode wait on mode ready

  RFM_Write(0x01,0x81);
  /*
  while (digitalRead(DIO5) == LOW)
  {
  }
  */
  delay(10);

  //Switch DIO0 to TxDone
  RFM_Write(0x40,0x40);
  //Set carrier frequency
  delay(10);

  /*
  fixed frequency
  // 868.100 MHz / 61.035 Hz = 14222987 = 0xD9068B
  _rfm95.RFM_Write(0x06,0xD9);
  _rfm95.RFM_Write(0x07,0x06);
  _rfm95.RFM_Write(0x08,0x8B);
  */

  // TCNT0 is timer0 continous timer, kind of random selection of frequency

  // EU863-870 specifications
  switch (TCNT0 % 8)
  {
      case 0x00: //Channel 0 868.100 MHz / 61.035 Hz = 14222987 = 0xD9068B
          RFM_Write(0x06,0xD9);
          RFM_Write(0x07,0x06);
          RFM_Write(0x08,0x8B);
          break;
        case 0x01: //Channel 1 868.300 MHz / 61.035 Hz = 14226264 = 0xD91358
          RFM_Write(0x06,0xD9);
          RFM_Write(0x07,0x13);
          RFM_Write(0x08,0x58);
          break;
        case 0x02: //Channel 2 868.500 MHz / 61.035 Hz = 14229540 = 0xD92024
          RFM_Write(0x06,0xD9);
          RFM_Write(0x07,0x20);
          RFM_Write(0x08,0x24);
          break;
        // added five more channels
        case 0x03: // Channel 3 867.100 MHz / 61.035 Hz = 14206603 = 0xD8C68B
          RFM_Write(0x06,0xD8);
          RFM_Write(0x07,0xC6);
          RFM_Write(0x08,0x8B);
          break;
        case 0x04: // Channel 4 867.300 MHz / 61.035 Hz = 14209880 = 0xD8D358
          RFM_Write(0x06,0xD8);
          RFM_Write(0x07,0xD3);
          RFM_Write(0x08,0x58);
          break;
        case 0x05: // Channel 5 867.500 MHz / 61.035 Hz = 14213156 = 0xD8E024
          RFM_Write(0x06,0xD8);
          RFM_Write(0x07,0xE0);
          RFM_Write(0x08,0x24);
          break;
        case 0x06: // Channel 6 867.700 MHz / 61.035 Hz = 14216433 = 0xD8ECF1
          RFM_Write(0x06,0xD8);
          RFM_Write(0x07,0xEC);
          RFM_Write(0x08,0xF1);
          break;
        case 0x07: // Channel 7 867.900 MHz / 61.035 Hz = 14219710 = 0xD8F9BE
          RFM_Write(0x06,0xD8);
          RFM_Write(0x07,0xF9);
          RFM_Write(0x08,0xBE);
          break;
        // FSK       868.800 Mhz => not used in this config
        // 869.525 - SF9BW125 (RX2 downlink only) for package received

    }
 
  // SF, BW 125 kHz
  // MOD: Set different SF accoring to user requirement:
  switch (SF){
    case 7: 
      RFM_Write(0x1E,0x74); //SF7 CRC On 
      break;

    case 8:
      RFM_Write(0x1E,0x84); //SF8 CRC On 
      break;

    case 9: 
      RFM_Write(0x1E,0x94); //SF9 CRC On 
      break;

    case 10:
      RFM_Write(0x1E,0xA4); //SF10 CRC On 
      break;

    case 11: 
      RFM_Write(0x1E,0xB4); //SF11 CRC On 
      RFM_Write(0x26,0x0C); //Low datarate optimization on AGC auto on
      break;

    case 12:
      RFM_Write(0x1E,0xC4); //SF12 CRC On 
      RFM_Write(0x26,0x0C); //Low datarate optimization on AGC auto on
      break;

    default:
      RFM_Write(0x1E,0x74); //SF7 CRC On 
      break;
  }

  //RFM_Write(0x1E,0xB4); //SF7 CRC On 
  RFM_Write(0x1D,0x72); //125 kHz 4/5 coding rate explicit header mode
  //RFM_Write(0x26,0x04); //Low datarate optimization off AGC auto on

  //Set IQ to normal values
  RFM_Write(0x33,0x27);
  RFM_Write(0x3B,0x1D);

  //Set payload length to the right length
  RFM_Write(0x22,Package_Length);

  //Get location of Tx part of FiFo
  //RFM_Tx_Location = RFM_Read(0x0E);

  //Set SPI pointer to start of Tx part in FiFo
  //RFM_Write(0x0D,RFM_Tx_Location);
  RFM_Write(0x0D,0x80); // hardcoded fifo location according RFM95 specs

  //Write Payload to FiFo
  for (i = 0;i < Package_Length; i++)
  {
    RFM_Write(0x00,*RFM_Tx_Package);
    RFM_Tx_Package++;
  }

  //Switch RFM to Tx
  RFM_Write(0x01,0x83);

  //Wait for TxDone
  while( digitalRead(_DIO0) == LOW )
  {
  }

  //Switch RFM to sleep
  RFM_Write(0x01,0x00);
}


// MOD: function used to set the TxPower level
void RFM95::RFM_Set_Tx_Power(uint8_t output_power, uint8_t PA_select)
{
  // If power amplifier !selected -> power limited below 14 dBm
  if (PA_select == 0) {
    // check if output power in range 0-14 dBm
    if (output_power < 0) {
      output_power = 0;
    } else if (output_power > 14) {
      output_power = 14;
    }

    /*
     0x70 = 0-111-output_power (last 4 bits)
     - 0 = PA_select
     - 111 = PA_max, set to max possible value s.t as in the DS: Pout = output_power
     - last four bits reserved for power level.
    */
    RFM_Write(0x09, 0x70 | output_power);

  } else {  // Power amplifier enabled
    // PA_select = 1 : power range € [2-20] dBm
    if (output_power > 17) {
      output_power = 17;      // limit to 17, then enable High power operation to boost to +20 dBm

      // High Power +20 dBm Operation (Semtech SX1276/77/78/79 5.4.3.)
      RFM_Write(0x4d, 0x87);
      RFM_Set_OCP(140);
    } else {
      if (output_power < 2) { // Output power must be >= 2 dBm when PA enabled
        output_power = 2;
      }
      //Default value PA_HF/LF or +17dBm
      RFM_Write(0x4d, 0x84);
      RFM_Set_OCP(100);
    }

    /*
      - 1st bit to 1 to set high power mode
      - bits from 1-3 not relevant since power boosted
      - last 4 bits set power level: Pout=17-(15-OutputPower) -> OutputPower=Pout-2
                                     (This ensures that Pout corresponds to output_power defined by user)
    */
    RFM_Write(0x09, 0x80 | (output_power - 2));  //PA Boost mask

  }
}


//MOD: Function used to adjust the overcurrent protection limit:
void RFM95::RFM_Set_OCP(uint8_t mA)
{
  // If not specified ocpTrim = 27 -> Imax >= 270 mA
  uint8_t ocpTrim = 27;

  
  if (mA <= 120) {

    // from DS: if Imax € [45, 120] mA : Imax = 45 + 5*OcpTrim [mA]
    ocpTrim = (mA - 45) / 5;
  } else if (mA <=240) {
    
    // from DS: if Imax € [130, 240] mA : Imax = -30 + 10*OcpTrim [mA]
    ocpTrim = (mA + 30) / 10;
  }

  /*
    - bits 6-7 not used
    - 5th bit = 1 OCP ON
    - bits 0-4 used for the OCPtrim computed above
  */
  RFM_Write(0x0B, 0x20 | (0x1F & ocpTrim));
}
