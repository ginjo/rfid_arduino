// RFID Tag Class

/*
 * Read, save, and delete tags to/from EEPROM
 * Receive tags from serial port reader hardware
 * Interface with hardware using Reader structs
 * Decode, process, validate tags against stored values
 * 
 * This also controls the mast output switch and all of its timing
 * I think the output controls should be moved to a separate class.
 */

#ifndef __RFID_H__
#define __RFID_H__

  #include <Arduino.h>
  //#include <Stream.h>
  #include <string.h>
  #include <SoftwareSerial.h>

  #include "settings.h"
  #include "reader.h"
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "tags.h"


  class RFID {
  public:
    /***  Variables  ***/
    
    //  uint8_t buff[MAX_TAG_LENGTH];
    //  uint8_t buff_index;
    int proximity_state;
    
    //  uint32_t current_ms;
    //  uint32_t last_tag_read_ms;
    //  uint32_t last_reader_power_cycle_ms;
    //  uint32_t reader_power_cycle_high_duration; // seconds
    //
    //  uint32_t ms_since_last_tag_read;
    //  uint32_t ms_since_last_reader_power_cycle;
    //  uint32_t ms_reader_cycle_total;
    //  uint32_t cycle_low_finish_ms;
    //  uint32_t cycle_high_finish_ms;
    //  //uint32_t tag_last_read_timeout_x_1000;
    //
    //  Stream * serial_port;

    Reader * reader;
    Led * blinker;

    
    /***  Constructors  ***/
    
    //RFID(Stream*, Led*, Reader*);
    //RFID(Stream*, Led*);
    RFID(Reader*, Led*);

    /***  Functions  ***/
    
    void begin();
    void loop();
    //  void resetBuffer();
    //  void pollReader();
    //  void cycleReaderPower();
    void setProximityState(int);
    void proximityStateController();
    //  void processTagData(uint8_t []);
    void initializeOutput();
    //  uint32_t msSinceLastTagRead();
    //  uint32_t msSinceLastReaderPowerCycle();
    //  uint32_t msReaderCycleTotal();
    //  uint32_t readerPowerCycleHighDuration();
    //  uint32_t tagLastReadTimeoutX1000();
    //  //uint32_t cycleLowFinishMs();
    //  //uint32_t cycleHighFinishMs();



    /***  Static Vars & Functions  ***/

    // This is experimental for the new Tags class
    // See tags.h for typedef TagArray.
    //typedef uint32_t TagArray[TAG_LIST_SIZE];
    static TagArray& Tags;
  
  };  // RFID

#endif
