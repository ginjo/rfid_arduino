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

  // TODO: Should these be stored in Settigs?
  //#define RAW_TAG_LENGTH 14  // RDM63000
  #define RAW_TAG_LENGTH 10  // 7941E

  class RFID {
  public:
    uint8_t buff[RAW_TAG_LENGTH];
    int buff_index;
    int proximity_state;
    uint32_t current_ms;
    uint32_t last_tag_read_ms;
    uint32_t last_reader_power_cycle_ms;
    Stream * serial_port;
    Led * blinker;

    // constructor
    RFID(Stream*, Led*);

    void begin();
    void loop();
    void resetBuffer();
    void pollReader();
    void cycleReaderPower();
    void setProximityState(int);
    void proximityStateController();
    void processTagData(uint8_t []);
    uint32_t msSinceLastTagRead();
    uint32_t msSinceLastReaderPowerCycle();
    uint32_t msReaderCycleTotal();
  };

#endif
