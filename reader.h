// RFID Reader struct

#ifndef __READER_H__
#define __READER_H__

  #include <Arduino.h>
  #include <stdlib.h>
  #include <string.h>
  #include "settings.h"

  struct Reader {
  public:
    // Vars

    // Name or model of the reader.
    char reader_name[16];

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


    // Constructor
    Reader(char[], uint8_t, uint8_t, uint8_t, bool);

    // Functions
    virtual uint32_t processTagData(uint8_t[]) = 0;
    int echo(int);
  };


  // External Global vars & functions.
  extern Reader** Readers;
  extern void readerArraySetup();


  // For inherited class constructor syntax, see:
  // https://www.learncpp.com/cpp-tutorial/114-constructors-and-initialization-of-derived-classes/

  struct RDM6300 : public Reader {
  public:
    RDM6300();
    uint32_t processTagData(uint8_t[]) override;
  };

  struct R7941E : public Reader {
  public:
    R7941E();
    uint32_t processTagData(uint8_t[]) override;
    //int echo(int) override;
  };

  struct WL125 : public Reader {
  public:
    WL125();
    uint32_t processTagData(uint8_t[]) override;
  };

#endif
