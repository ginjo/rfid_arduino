#ifndef __GL_H__
#define __GL_H__

  // TODO: Everything in this file can (should?) be in main ino file.


  /* Arduino Pin Assignments */

  //#include <Arduino.h>
  //#include <SoftwareSerial.h>
  // Don't load any custom classes here, as global.h should load before anything custom.
  // Use forward declarations instead, if you need to refer to custom classes here.fre
  
  #define DEBUG_PIN 8

  #define BEEPER_PIN 9
  
  #define LED_RED_PIN 3
  #define LED_GREEN_PIN 5
  #define LED_BLUE_PIN 6

  #define OUTPUT_SWITCH_PIN 13

  #define RFID_RX_PIN 4
  #define RFID_TX_PIN A6 // RFID readers generally don't receive input.
  #define READER_POWER_CONTROL_PIN 7

  #define SW_RX_PIN 11
  #define SW_TX_PIN 12 // remember to reduce arduino output (at BT RX pin) to 3.3v with voltage divider.
  #define BT_STATUS_PIN 2 // low == connected.

  
#endif

  
