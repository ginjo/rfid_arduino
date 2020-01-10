#ifndef __IO_H__
#define __IO_H__


  /* Arduino Pin Assignments */

  #include <Arduino.h>
  #include <SoftwareSerial.h>
  
  #define DEBUG_PIN 11
  #define FAILSAFE_PIN 12

  #define BEEPER_PIN 9
  
  #define LED_RED_PIN 8
  #define LED_GREEN_PIN 7
  #define LED_BLUE_PIN 6

  #define OUTPUT_SWITCH_PIN 13

  #define RFID_RX_PIN 4
  #define RFID_TX_PIN A6 // generally not used
  #define READER_POWER_CONTROL_PIN 5

  #define BT_RX_PIN 2
  #define BT_TX_PIN 3 // remember to reduce arduino output (BT input) to 3.3v with voltage divider
  #define BT_STATUS_PIN 10 // low == connected


  /* Global Vars */

  // See https://stackoverflow.com/questions/12290451/access-extern-variable-in-c-from-another-file
  extern SoftwareSerial *BTserial; // = new SoftwareSerial(BT_RX_PIN, BT_TX_PIN);

#endif
