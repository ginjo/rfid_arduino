// RFID Reader struct

#ifndef __READER_H__
#define __READER_H__

  #include <Arduino.h>
  // Not sure why this was needed, but seems to work without.
  //#include <stdlib.h>
  #include <string.h>
  #include "settings.h"
  #include "tags.h"

  #define MAX_TAG_LENGTH 16


  struct Reader {
  public:
    // Vars

    // Name or model of the reader.
    char reader_name[16]; // also see S.DEFAULT_READER.

    // Absolute length in 8-bit bytes of the RFID data
    // received from reader for each tag.
    uint8_t raw_tag_length;

    // 0-based array position of first and last byte of tag ID.
    uint8_t id_begin;
    uint8_t id_end;

    // Adjusts logic of reader power control,
    // so you can use it for direct power control
    // or for pull-to-ground-to-reset control.
    // 1 == switch is normally closed and opens briefly to cycle power (direct control of reader power).
    // 0 == switch is normally open and closes briefly to cycle power (to pull a reset pin to ground).
    bool power_control_logic;



    /****  From RFID  ***/
    
    uint8_t buff[MAX_TAG_LENGTH];
    uint8_t buff_index;
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
    uint32_t current_tag_id;
    SoftwareSerial * serial_port;

    void resetBuffer();
    void pollReader();
    void cycleReaderPower();
    // void processTagData(uint8_t []); // Had to change name after moving to Reader.
    void processTag(uint8_t []);
    uint32_t msSinceLastTagRead();
    uint32_t msSinceLastReaderPowerCycle();
    uint32_t msReaderCycleTotal();
    uint32_t readerPowerCycleHighDuration();
    uint32_t tagLastReadTimeoutX1000();
    //uint32_t cycleLowFinishMs();
    //uint32_t cycleHighFinishMs();
    


    // Constructor(s)
    Reader(const char[], uint8_t, uint8_t, uint8_t, bool);

    // Functions
    virtual uint32_t processTagData(uint8_t[]);
    //int echo(int);
    void loop();


    /***  Static Vars & Functions  ***/

    static Reader* GetReader(const char*);
    
    template <class T>
    static bool TestReader(const char* _name) {
      T obj = T();
      FreeRam("after TestReader");
      //printf("obj.name: %s, _name: %s\n", obj.name, _name);
      if (strcmp(obj.reader_name, _name) == 0) {
        return true;
      } else {
        return false;
      }
    }

  }; // class Reader



  // For inherited class constructor syntax, see:
  // https://www.learncpp.com/cpp-tutorial/114-constructors-and-initialization-of-derived-classes/

  struct RDM6300 : public Reader {
  public:
    RDM6300();
    uint32_t processTagData(uint8_t[]);
  };

  struct R7941E : public Reader {
  public:
    R7941E();
    uint32_t processTagData(uint8_t[]);
    //int echo(int) override;
  };

  struct WL125 : public Reader {
  public:
    WL125();
    uint32_t processTagData(uint8_t[]);
  };


#endif
