#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <SoftwareSerial.h>

#include "LoRaWAN.h"
#include "secconfig.h"
#include "tinySPI.h"

/* Add your include statements here */
/* USER CODE BEGIN */

/* USER CODE END */



/* Debug serial port */
//SoftwareSerial serial(2, 3); // RX, TX

/* RFM95 instance */
const byte DIO0 = 0;
const byte NSS = 1;
RFM95 rfm(DIO0, NSS);

/* Define LoRaWAN layer */
LoRaWAN lora = LoRaWAN(rfm);
uint32_t Frame_Counter_Tx = 0x0000;
const byte SF = 7;        // set spreading factor
uint8_t PA_boost_on = 1;  // set power amplifier 
uint8_t power_level = 14; // set Tx power

/* Sleep counters */
const uint16_t sleep_total = 2;                  // Set sleep time (depends on your wdt settings)
volatile uint16_t sleep_count = sleep_total + 1; // count elapsed sleep cycles



/* Functions implemented at the bottom of the page */
void goToSleep();         // Enables sleep state
void wdtSetup();          // Setup watchdog timer
void setup_unused_pins(); // Initializes unused pins to save power

/* USER CODE BEGIN */
// Add your functions definition here

/* USER CODE END */


void setup() {
  // // initialize debug serial
  // serial.begin(9600);
  // while (!serial) {
  //   delay(100);
  // }

  /* Initialize your sensors here */
  /* USER CODE BEGIN */
  setup_unused_pins();  // Modify according to your pin map

  /* USER CODE END */

  // Initialize RFM module
  rfm.init(power_level, PA_boost_on);
  lora.setKeys(NwkSkey, AppSkey, DevAddr);
}


void loop() {

  // sleep:
  goToSleep();

  // Once awake check sleep counter:
  if (sleep_count >= sleep_total) { // if time to awake:

    /* Read and prepare your sensor data here */
    /* USER CODE BEGIN */
    uint8_t Data_Length = 0x10;
    unsigned char Data[Data_Length];
    Data_Length = sprintf(Data, "test");

    /* USER CODE END */

    // prepara LoRa:
    rfm.init(power_level, PA_boost_on);
    delay(1);

    // transmit data
    lora.Send_Data(Data, Data_Length, Frame_Counter_Tx, SF);

    Frame_Counter_Tx++;

    // reset sleep count
    sleep_count = 0;
  }
  // come back to sleep
}



/* 
  Setup functions (Do not remove!)
  if needed:
  - modify wdtSetup() to adjust watchdog internal timer
  - modify setup_unused_pins() accoring to your pinmap
*/

/* 
  This function allows to setup the interanl watchdog timer: 
   - modify WDPx accoring to the following map to adjust the timer duration
*/
void wdtSetup()
{
  // Table for clearing/setting bits
  //WDP3 - WDP2 - WPD1 - WDP0 - time
  // 0      0      0      0      16 ms
  // 0      0      0      1      32 ms
  // 0      0      1      0      64 ms
  // 0      0      1      1      0.125 s
  // 0      1      0      0      0.25 s
  // 0      1      0      1      0.5 s
  // 0      1      1      0      1.0 s
  // 0      1      1      1      2.0 s
  // 1      0      0      0      4.0 s
  // 1      0      0      1      8.0 s

  // Prevent new interrupt generation
  SREG &= ~(1 << SREG_I);

  // Reset the watchdog reset flag
  bitClear(MCUSR, WDRF);
  // Start timed sequence
  bitSet(WDTCSR, WDCE); //Watchdog Change Enable to clear WD
  bitSet(WDTCSR, WDE); //Enable WD

  // Set new watchdog timeout value to 8 second
  bitSet(WDTCSR, WDP3);
  bitClear(WDTCSR, WDP2);
  bitClear(WDTCSR, WDP1);
  bitSet(WDTCSR, WDP0);
  // Enable interrupts instead of reset
  bitSet(WDTCSR, WDIE);

  // re-enable interrupts:
  SREG |= (1 << SREG_I);
}

/* 
  This function allows to enter deep-sleep state 
   - No modifications are needed.
*/
void goToSleep()
{
  //Disable ADC, saves ~230uA
  ADCSRA &= ~(1 << ADEN);
  wdtSetup(); //enable watchDog
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // sleep_mode does set and reset the SE bit.
  // sleep_enable and sleep_disable are not needed in this case
  sleep_mode();
  //disable watchdog after sleep
  wdt_disable();
  // enable ADC
  ADCSRA |=  (1 << ADEN);
}

/*
  This function initializes unused pins:
    - Enable input pull-ups on all the inactive pins
    - Turn off digital input buffer on analog channels
  Comment on the lines corresponding to the pins you are actually implementing
*/
void setup_unused_pins(){
  // ---> Unused digital lines: (comments on pins you are actually implementing)
  pinMode(10, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);

  // ---> Unused analog lines: (comments on pins you are actually implementing)
  
  /* Enable pull-ups */
  pinMode(7, INPUT_PULLUP);
  //pinMode(6, INPUT_PULLUP);  // Used as MOSI
  //pinMode(5, INPUT_PULLUP);  // Used as MISO
  //pinMode(4, INPUT_PULLUP);  // Used as SCK
  pinMode(3, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  //pinMode(1, INPUT_PULLUP);  // Used as NSS
  //pinMode(0, INPUT_PULLUP);  // Used as DIO0

  /* Turn off digital input buffers: DIDRO0 register */
  bitSet(DIDR0, ADC7D);
  //bitSet(DIDR0, ADC6D);  // Used as MOSI
  //bitSet(DIDR0, ADC5D);  // Used as MISO
  //bitSet(DIDR0, ADC4D);  // Used as SCK
  bitSet(DIDR0, ADC3D);
  bitSet(DIDR0, ADC2D);
  //bitSet(DIDR0, ADC1D);  // Used as NSS
  //bitSet(DIDR0, ADC0D);  // Used as DIO0
}

/* 
  Interrupt handler: 
  - code here must be executed as fast as possible (increment counter or set a flag)!
  - declare sleep_count as volatile variable for faster execution;
*/
ISR(WDT_vect) {
  sleep_count++; // keep track of how many sleep cycles have been completed.
}



/*
  User functions:
*/

/* USER CODE BEGIN*/
// Implement your own functions here...

/* USER CODE END*/