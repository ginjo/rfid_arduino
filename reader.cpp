/***  READERS  ***/

#include "reader.h"

  // Defines Reader Constructor
  Reader::Reader(char _name[], uint8_t _raw_tag_length, uint8_t _id_begin, uint8_t _id_end) :
    //reader_name(_name),
    raw_tag_length(_raw_tag_length),
    id_begin(_id_begin),
    id_end(_id_end)
  {
    strncpy(reader_name, _name, sizeof(reader_name));
    DPRINT(F("Constructing Reader for: "));
    DPRINTLN(_name);
  }

  int Reader::echo(int _dat) {
    return _dat;
  }


  // Defines Readers array.
  extern Reader** Readers = new Reader*[3];

  // Defines global function for Readers array setup.
  extern void readerArraySetup() {
    Serial.println(F("readerArraySetup() building array of readers."));
    Readers[0] = new RDM6300;
    Readers[1] = new R7941E;
    Readers[2] = new WL125;    
  }


  /***  Derived classes for reader specifications  ***/

  RDM6300::RDM6300() :
    Reader("RDM-6300", 14, 3, 10)
  { ; }

  uint32_t RDM6300::processTagData(uint8_t _tag[24]) {

    DPRINT("RDM6300::processTagData() with input: ");
    DPRINTLN(strtol((char *)_tag, NULL, HEX));

    uint8_t id_len = id_end - id_begin;
    char tmp_str[id_len] = "";
    
    for(int n=id_begin; n<=id_end; n++) {
      sprintf(tmp_str + strlen(tmp_str), "%c", _tag[n]);
    }

    uint32_t tag_id = strtol((char *)tmp_str, NULL, 16);
  
    Serial.print(F("RDM6300 Tag success: "));
    Serial.print((char *)tmp_str);
    Serial.print(", ");
    Serial.println(tag_id);

    // Why this here?
    //strncpy(tmp_str, NULL, id_len);
    
    return tag_id;
  }


  R7941E::R7941E() :
    Reader("7941E", 10, 4, 7)
  { ; }
  //{ Readers[1] = this; } // doesn't work
    
  uint32_t R7941E::processTagData(uint8_t _tag[]) {

    DPRINT(F("R7941E::processTagData() with input: "));
    DPRINTLN();
    
    // TEMPORARY !!!
    //return 22334455; // TEMPORARY !!!

    DPRINTLN(strtol((char *)_tag, NULL, 10));
    
    uint8_t id_len = id_end - id_begin;
    char tmp_str[id_len] = "";

    // or maybe it's this?
//    for(int n=id_begin; n<=id_end; n++) {
//      sprintf(tmp_str + strlen(tmp_str), "%x", _tag[n]);
//    }

    // ... or this?
    uint32_t tag_id = strtol((char *)tmp_str, NULL, 16);
  
    Serial.print(F("R7941E Tag success: "));
    Serial.print((char *)tmp_str);
    Serial.print(", ");
    Serial.print(tag_id);
    
    //strncpy(tmp_str, NULL, id_len);
      
    return tag_id;
  }

  
  WL125::WL125() :
    Reader("WL-125", 13, 3, 10)
  { ; }

  uint32_t WL125::processTagData(uint8_t _tag[]) {

    DPRINT(F("WL125::processTagData() with input: "));
    DPRINTLN(strtol((char *)_tag, NULL, HEX));

    uint8_t id_len = id_end - id_begin;
    char tmp_str[id_len] = "";
    
    for(int n=id_begin; n<=id_end; n++) {
      sprintf(tmp_str + strlen(tmp_str), "%c", _tag[n]);
    }

    uint32_t tag_id = strtol((char *)tmp_str, NULL, 16);
  
    Serial.print(F("WL125 Tag success: "));
    Serial.print((char *)tmp_str);
    Serial.print(", ");
    Serial.println(tag_id);

    // Why this here?
    //strncpy(tmp_str, NULL, id_len);
    
    return tag_id;
  }


  /***  Global functions and variables  ***/

  //  Reader * GetReader(char _name[16]) {
  //    DPRINT("GetReader() called with name: ");
  //    DPRINTLN((char *)_name);
  //
  //    Reader * result;
  //    int n;
  //
  //    for (n=0; n < 3; n++) {
  //      if (_name == Readers[n]->reader_name) {
  //        result = Readers[n];
  //        break;
  //      }
  //    }
  //
  //    if (n < 3) {
  //      DPRINT("GetReader() selected: ");
  //      DPRINTLN(result->reader_name);
  //    } else {
  //      DPRINTLN("No reader was selected");
  //    }
  //    
  //    return result;
  //  }

  //  Reader* Readers[3] = {
  //    &RDM6300(),
  //    &R7941E(),
  //    &WL125()
  //  };

  //  Reader * Readers[0] = new *RDM6300();
  //  Reader * Readers[1] = new *R7941E();
  //  Reader * Readers[2] = new *WL125();

  //  Readers[0] = new RDM6300();
  //  Readers[1] = new R7941E();
  //  Readers[2] = new WL125();

 
