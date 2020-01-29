#ifndef __READER_H__
#define __READER_H__

  #include <Arduino.h>
  #include <string.h>
  
  #include "settings.h"
  #include "logger.h"
  #include "tags.h"
  #include "led_blinker.h"

  // See logger.h for master debug controls.
  #ifdef RD_DEBUG
    #define RD_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define RD_LOG(...)
  #endif

  #define MAX_TAG_LENGTH 16
  #define READER_NAME_LENGTH 16
  #define READER_COUNT 3


  struct Reader {
  public:
    /***  Static Vars & Functions  ***/

    //static constexpr char *Name = {( char *)"base"};
    //static const char *Name;
    static const char Name[READER_NAME_LENGTH];
    
    //static Reader *GetReader(const char*);
    static Reader *GetReader(int index);

    static char *NameFromIndex(int index);

    static void PrintReaders(Stream *sp);
    
    //  template <class T>
    //  static bool TestReader(const char* _name) {
    //    T obj = T();
    //    FREERAM("after TestReader");
    //    //printf("obj.name: %s, _name: %s\n", obj.name, _name);
    //    if (strcmp(obj.name(), _name) == 0) {
    //      return true;
    //    } else {
    //      return false;
    //    }
    //  }


    /***  Constructors  ***/
    
    Reader(uint8_t, uint8_t, uint8_t, bool);

  
    /*** Instance Vars & Functions ***/

    // Absolute length in 8-bit bytes of the Controller data
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
    
    uint8_t buff[MAX_TAG_LENGTH];
    uint8_t buff_index;
    uint32_t current_ms;
    uint32_t last_tag_read_ms;
    uint32_t last_reader_power_cycle_ms;
    uint32_t reader_power_cycle_high_duration; // seconds
    //uint32_t ms_since_last_tag_read;
    //uint32_t ms_since_last_reader_power_cycle;
    uint32_t ms_reader_cycle_total;
    uint32_t cycle_low_finish_ms;
    uint32_t cycle_high_finish_ms;
    //uint32_t tag_last_read_timeout_x_1000;
    uint32_t last_tag_read_id;
    SoftwareSerial * serial_port;


    /*  Instance Functions  */
    
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
    
    virtual uint32_t processTagData(uint8_t[]);
    //virtual const char *name() {return Name;}
    virtual char *name();

    void loop();

  }; // class Reader

#endif
