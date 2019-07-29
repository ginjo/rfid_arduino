  /***  Classes derived from Reader are individual reader specifications  ***/

  #include "reader.h"

  RDM6300::RDM6300() :
    Reader("RDM-6300", 14, 3, 10, 1)
  { ; }

  uint32_t RDM6300::processTagData(uint8_t _tag[24]) {

    DPRINT(F("RDM6300 prcs tag inpt: "));
    DPRINTLN(strtol((char *)_tag, NULL, HEX));

    uint8_t id_len = id_end - id_begin;
    char tmp_str[id_len+1] = "";
    
    for (int n=id_begin; n<=id_end; n++) {
      sprintf(tmp_str + strlen(tmp_str), "%c", _tag[n]);
    }

    uint32_t tag_id = strtol((char *)tmp_str, NULL, 16);
  
    DPRINT(F("RDM6300 tag read: "));
    DPRINT((char *)tmp_str);
    DPRINT(", ");
    DPRINTLN(tag_id);
    
    return tag_id;
  }


  R7941E::R7941E() :
    Reader("7941E", 10, 4, 7, 1)
  { ; }
    
  uint32_t R7941E::processTagData(uint8_t _tag[]) {
    DPRINT(F("R7941E prcs tag inpt: "));
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
    
    char id_hex[id_len+1] = ""; // need to initialize this to empty.

    for (int n=id_begin; n<=id_end; n++) {
      sprintf(id_hex + strlen(id_hex), "%02x", _tag[n]);
    }

    uint32_t tag_id = strtol(id_hex, NULL, 16);
  
    DPRINT(F("R7941E tag read: "));
    DPRINT(id_hex);
    DPRINT(", ");
    DPRINTLN(tag_id);
          
    return tag_id;
  }


  // NOTE: Consider that this reader (and the rdm6300)
  // yield a string of ascii characters (0-9, A-F),
  // and those characters are THEN used to build the ID
  // of the tag.
  //
  WL125::WL125() :
    Reader("WL-125", 13, 3, 10, 0)
  { ; }

  uint32_t WL125::processTagData(uint8_t _tag[]) {

    //Reader::preProcessTagData();

    DPRINT(F("WL125 prcs tag inpt: "));
    DPRINT(F("id_begin: "));
    DPRINT(id_begin);
    DPRINT(F(", id_end: "));
    DPRINT(id_end);
    
    uint8_t id_len = (id_end - id_begin +1);
    
    DPRINT(F(", id_len: "));
    DPRINT(id_len);
    
    DPRINT(F(", checksum (raw bytes): "));
    DPRINT(_tag[11]); DPRINT(","); DPRINTLN(_tag[12]);

    char id_char[id_len+1] = ""; // need to initialize this to empty.
    
    for (int n=id_begin; n<=id_end; n++) {
      sprintf(id_char + strlen(id_char), "%c", _tag[n]);
    }

    uint32_t tag_id = strtol(id_char, NULL, 16);
  
    DPRINT(F("WL125 read: "));
    DPRINT(id_char);
    DPRINT(", ");
    DPRINTLN(tag_id);
    
    return tag_id;
  }
