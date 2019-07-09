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

    // Constructor(s)
    Reader(const char[], uint8_t, uint8_t, uint8_t, bool);

    // Functions
    virtual uint32_t processTagData(uint8_t[]);
    void preProcessTagData();
    int echo(int);
  };


  // External Global vars & functions.
  // TODO: Convert these to static Reader functions. See TODO in .cpp file.
  extern Reader** Readers;
  //extern Reader * Readers[3];
  extern void ReaderArraySetup();
  extern Reader * GetReader(const char[]);


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
