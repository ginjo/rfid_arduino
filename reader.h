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
    char reader_name[16];
    uint8_t raw_tag_length;
    uint8_t id_begin;
    uint8_t id_end;

    // Constructor
    Reader(char[], uint8_t, uint8_t, uint8_t);

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
