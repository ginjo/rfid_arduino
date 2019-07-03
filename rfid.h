// RFID Tag Class

/*
 * Read, save, and delete tags to/from EEPROM
 * Receive tags from serial port reader hardware
 * Interface with hardware using Reader structs
 * Decode, process, validate tags against stored values
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
  #include "reader.h"
  #include "led_blinker.h"

  #define MAX_TAG_LENGTH 16
  

  class RFID {
  public:
    // Vars
    uint8_t buff[MAX_TAG_LENGTH];
    uint8_t buff_index;
    int proximity_state;
    
    uint32_t current_ms;
    uint32_t last_tag_read_ms;
    uint32_t last_reader_power_cycle_ms;
    uint32_t reader_power_cycle_high_duration; // seconds

    uint32_t ms_since_last_tag_read;
    uint32_t ms_since_last_reader_power_cycle;
    uint32_t ms_reader_cycle_total;
    uint32_t cycle_low_finish_ms;
    uint32_t cycle_high_finish_ms;
    //uint32_t tag_last_read_timeout_x_1000;
    
    Stream * serial_port;
    Led * blinker;
    Reader * reader;

    
    // Constructors
    RFID(); // dummy for initialization
    //RFID(Stream*, Led*, Reader*);
    RFID(Stream*, Led*);

    // Functions
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
    uint32_t readerPowerCycleHighDuration();
    uint32_t tagLastReadTimeoutX1000();
    //uint32_t cycleLowFinishMs();
    //uint32_t cycleHighFinishMs();
  };

#endif
