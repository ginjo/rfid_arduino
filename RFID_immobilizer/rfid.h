// RFID Tag Class

/*
 * Load list of tags from EEPROM
 * Save tag to EEPROM
 * Delete tag from EEPROM
 * Match tag aginst existing list
 * Validate incoming encoded tag read from rfid serial
 * 
 * TODO: Should this class be split into RFIDTagsList and RFIDTag?
 * I don't think so, at least not yet, since a list of tags is just an array,
 * and the storage is a single EEPROM service.
 * 
 */

#ifndef __RFID_H__
#define __RFID_H__

  #include <Arduino.h>
  //#include <Stream.h>
  #include <SoftwareSerial.h>

  #include "settings.h"
  #include "led_blinker.h"
  
  //#define RAW_TAG_LENGTH 14  // RDM63000
  #define RAW_TAG_LENGTH 10  // 7941E
  
  //#define TAG_LAST_READ_TIMEOUT 20 // seconds
  //#define TAG_READ_INTERVAL 1000
  //#define READER_CYCLE_LOW_DURATION 150
  //#define READER_CYCLE_HIGH_DURATION 5000
  //#define READER_POWER_CONTROL_PIN 6

  class RFID {
  public:
    uint8_t buff[RAW_TAG_LENGTH];
    int buff_index;
    int fuel_pump_state;
    unsigned long last_tag_read_ms;
    unsigned long last_reader_power_cycle;
    Stream * serial_port;
    Led * blinker;

    // constructor
    RFID(Stream*, Led*);

    void begin();
    void loop();
    void resetBuffer();
    void cycleReaderPower();
    void timeoutFuelPump();
    void processTagData(uint8_t * []);
  };

#endif
