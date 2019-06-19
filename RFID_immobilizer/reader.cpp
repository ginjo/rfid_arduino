/***  READERS  ***/

#include "reader.h"

  // TODO: I think readers need their own file(s).
  // TODO: I think each reader type needs is own sub-class (or sub-struct?),
  //       since it will likely have its own custom functions.

  Reader::Reader(char* _name, uint8_t _raw_tag_length, uint8_t _id_begin, uint8_t _id_end) :
    reader_name(_name),
    raw_tag_length(_raw_tag_length),
    id_begin(_id_begin),
    id_end(_id_end)
  {
    ;  
  }

  Reader GetReader(char _name[]) {
    for (int n=0; n < 3; n++) {
      if (_name == Readers[n].reader_name) { return Readers[n]; }
    }
  }

  uint32_t Reader::processTagData(uint8_t _tag[]) {
    
    /*** Taken from RFID class but not yet refactored for Reader class ***/
    
    //  if (S.RAW_TAG_LENGTH == 14 || S.RAW_TAG_LENGTH == 13) {  // readers: RDM6300, ZocoRFID (aliexpres) WL-125 
    //    //DPRINTLN((char *)_tag);
    //    DPRINTLN(strtol((char *)_tag, NULL, HEX));
    //    id_begin = 3;
    //    id_end   = 10;
    //    
    //  } else if (S.RAW_TAG_LENGTH == 10) {  // readers: 7941E
    //    id_begin = 4;
    //    id_end   = 7;
    //  }
    //
    //  uint8_t id_len = id_end - id_begin;
    //  char tmp_str[id_len] = "";
    //  
    //  for(int n=id_begin; n<=id_end; n++) {
    //    sprintf(tmp_str + strlen(tmp_str), "%x", _tag[n]);
    //  }
    //
    //  Serial.print(F("Tag success: "));
    //  Serial.print((char *)tmp_str);
    //  Serial.print(", ");
    //  Serial.print(strtol((char *)tmp_str, NULL, 16));
    //  Serial.print(F(", at ms "));
    //  Serial.println(current_ms);
    //  
    //  strncpy(tmp_str, NULL, id_len);
    
    return 123456789;
  }

  uint32_t WL125::processTagData(uint8_t _tag[]) {
    
    DPRINTLN(strtol((char *)_tag, NULL, HEX));

    uint8_t id_len = id_end - id_begin;
    char tmp_str[id_len] = "";
    
    for(int n=id_begin; n<=id_end; n++) {
      sprintf(tmp_str + strlen(tmp_str), "%x", _tag[n]);
    }
  
    Serial.print(F("Tag success: "));
    Serial.print((char *)tmp_str);
    Serial.print(", ");
    Serial.println(strtol((char *)tmp_str, NULL, 16));

    
    strncpy(tmp_str, NULL, id_len);
    
    return 123456789;
  }

  Reader Readers[3] = {
    RDM6300(),
    R7941E(),
    WL125()
  };
