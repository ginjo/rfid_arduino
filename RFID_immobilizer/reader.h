// RFID Reader struct

#ifndef __READER_H__
#define __READER_H__

  #include <Arduino.h>
  #include "settings.h"

  struct Reader {
  public:
    // vars
    char* reader_name;
    uint8_t raw_tag_length;
    uint8_t id_begin;
    uint8_t id_end;

    // constructor
    Reader(char*, uint8_t, uint8_t, uint8_t);

    // functions
    uint32_t processTagData(uint8_t[]);
  };

  extern Reader Readers[3];
  extern Reader GetReader(char[]);


  // For inherited class constructor syntax, see:
  // https://www.learncpp.com/cpp-tutorial/114-constructors-and-initialization-of-derived-classes/


  struct RDM6300 : public Reader {
  public:
    RDM6300() :
      Reader("RDM6300", 14, 3, 10)
    { ; }

    uint32_t processTagData(uint8_t[]);
  };


  struct R7941E : public Reader {
  public:
    R7941E() :
      Reader("R7941E", 10, 4, 7)
    { ; }

    uint32_t processTagData(uint8_t[]);
  };

  
  struct WL125 : public Reader {
  public:
    WL125() :
      Reader("WL-125", 13, 3, 10)
    { ; }

    uint32_t processTagData(uint8_t[]);
  };

#endif
