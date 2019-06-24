/***  READERS  ***/

#include "reader.h"
  
  // Defines Reader Constructor
  Reader::Reader(char _name[], uint8_t _raw_tag_length, uint8_t _id_begin, uint8_t _id_end, bool _control_logic) :
    //reader_name(_name),
    raw_tag_length(_raw_tag_length),
    id_begin(_id_begin),
    id_end(_id_end),
    power_control_logic(_control_logic)
  {
    strncpy(reader_name, _name, sizeof(reader_name));
    DPRINT(F("Constructing Reader for: "));
    DPRINTLN(_name);
  }

  int Reader::echo(int _dat) {
    return _dat;
  }

  uint32_t Reader::processTagData(uint8_t[]) {
    DPRINTLN("Error: You are attempting to call processTagData() on the generic Reader class");
  }

  // Defines Readers array.
  // NOTE: Using 'new' keyword usually requires a 'delete' somewhere to prevent memory leaks.
  extern Reader** Readers = new Reader*[3];
  //extern Reader * Readers[3] = {};
  
  // Defines global function for Readers array setup.
  extern void readerArraySetup() {
    Serial.println(F("readerArraySetup() building array of readers."));
    // These work with Reader as abstract class (with pure virtual functions).
    Readers[0] = new RDM6300;
    Readers[1] = new R7941E;
    Readers[2] = new WL125;
    //
    // These compile, but they won't work.
    //  Readers[0] = &RDM6300();
    //  Readers[1] = &R7941E();
    //  Readers[2] = &WL125();
    //
    // These don't seem to work either.
    //  RDM6300 * rdm6300;
    //  Readers[0] = rdm6300;
    //  
    //  R7941E * r7941e;
    //  Readers[1] = r7941e;
    //  
    //  WL125 * wl125;
    //  Readers[2] = wl125;
  }


  /***  Derived classes for reader specifications  ***/

  RDM6300::RDM6300() :
    Reader("RDM-6300", 14, 3, 10, 1)
  { ; }

  uint32_t RDM6300::processTagData(uint8_t _tag[24]) {

    DPRINT(F("RDM6300::processTagData() with input: "));
    DPRINTLN(strtol((char *)_tag, NULL, HEX));

    uint8_t id_len = id_end - id_begin;
    char tmp_str[id_len] = "";
    
    for (int n=id_begin; n<=id_end; n++) {
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
    Reader("7941E", 10, 4, 7, 1)
  { ; }
    
  uint32_t R7941E::processTagData(uint8_t _tag[]) {
    DPRINT(F("R7941E::processTagData() with "));
    //DPRINT((char *)_tag);
    DPRINT(F("id_begin: "));
    DPRINT(id_begin);
    DPRINT(F(", id_end: "));
    DPRINT(id_end);
    
    uint8_t id_len = (id_end - id_begin +1)*2;
    DPRINT(F(", id_len: "));
    DPRINT(id_len);
    
    DPRINT(F(", checksum (dec): "));
    DPRINTLN(_tag[8]);
    
    char id_hex[id_len] = ""; // need to initialize this to empty.

    for (int n=id_begin; n<=id_end; n++) {
      sprintf(id_hex + strlen(id_hex), "%02x", _tag[n]);
    }

    uint32_t tag_id = strtol(id_hex, NULL, 16);
  
    Serial.print(F("R7941E Tag read: "));
    Serial.print(id_hex);
    Serial.print(", ");
    Serial.println(tag_id);
    
    //strncpy(tmp_str, NULL, id_len);
      
    return tag_id;
  }


  // TODO: Consider that this reader (and the rdm6300)
  // yield a string of ascii characters (0-9, A-F),
  // and those characters are THEN used to build the ID
  // of the tag.
  //
  WL125::WL125() :
    Reader("WL-125", 13, 3, 10, 0)
  { ; }

  uint32_t WL125::processTagData(uint8_t _tag[]) {

    DPRINT(F("WL125::processTagData() with input: "));
    DPRINT(F("id_begin: "));
    DPRINT(id_begin);
    DPRINT(F(", id_end: "));
    DPRINT(id_end);
    
    uint8_t id_len = (id_end - id_begin +1);
    
    DPRINT(F(", id_len: "));
    DPRINT(id_len);
    
    DPRINT(F(", checksum (chars representing hex): "));
    DPRINT(_tag[11]); DPRINTLN(_tag[12]);

    char id_char[id_len] = ""; // need to initialize this to empty.
    
    for (int n=id_begin; n<=id_end; n++) {
      sprintf(id_char + strlen(id_char), "%c", _tag[n]);
    }

    uint32_t tag_id = strtol(id_char, NULL, 16);
  
    Serial.print(F("WL125 Tag success: "));
    Serial.print(id_char);
    Serial.print(", ");
    Serial.println(tag_id);

    // Why this here?
    //strncpy(tmp_str, NULL, id_len);
    
    return tag_id;
  }


  /***  Global functions and variables  ***/

  //  Reader * GetReader(char _name[16]) {
  //    DPRINT(F("GetReader() called with name: "));
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
  //      DPRINT(F("GetReader() selected: "));
  //      DPRINTLN(result->reader_name);
  //    } else {
  //      DPRINTLN(F("No reader was selected"));
  //    }
  //    
  //    return result;
  //  }


 
