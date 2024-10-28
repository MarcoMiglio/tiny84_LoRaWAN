/*
  RFM95.h - Library header file for RFM95 LoRa module.
  Created by Leo Korbee, March 31, 2018.
  Released into the public domain.
  @license Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)
  Thanks to all the folks who contributed beforme me on this code.

*/

#ifndef RFM95_h
#define RMF95_h

#include "Arduino.h"

class RFM95
{
  public:
    RFM95(int DIO0, int NSS);
    
    // MODIFICA: variables added to set tx power
    void init(uint8_t level, uint8_t PA_boost_on);
    void RFM_Write(unsigned char RFM_Address, unsigned char RFM_Data);
    unsigned char RFM_Read(unsigned char RFM_Address);

    // MODIFICA: variabile "SF" dell func. Send_Package
    void RFM_Send_Package(unsigned char *RFM_Tx_Package, unsigned char Package_Length, uint8_t SF);

    // MODIFICA: aggiunta funzione per aggiustare potenza in trasmissione
    void RFM_Set_Tx_Power(uint8_t output_power, uint8_t PA_select);

    // MODIFICA: aggiunta funzione per aggiustare OCP
    void RFM_Set_OCP(uint8_t mA);

  private:
    int _DIO0;
    int _NSS;
};


#endif
