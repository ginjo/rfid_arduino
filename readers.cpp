  /*
    Classes derived from Reader are individual reader specifications
    See below for Reader::GetReader() function.
  */


  #include "reader.h"


  struct RDM6300 : public Reader {
  public:
    static constexpr char *Name = {( char *)"RDM-6300"};
    //static const char Name[READER_NAME_LENGTH] = {"RDM-6300"};
  
    RDM6300() :
      Reader(14, 3, 10, 1)
    {
      LOG(4, F("Loading reader: "));
      LOG(4, name(), true);  
    }
  
    uint32_t processTagData(uint8_t _tag[24]) {
  
      LOG(5, F("RDM6300 prcs tag inpt: "));
      LOG(5, strtol((char *)_tag, NULL, HEX), true);
  
      uint8_t id_len = id_end - id_begin;
      char tmp_str[id_len+1] = "";
      
      for (int n=id_begin; n<=id_end; n++) {
        sprintf(tmp_str + strlen(tmp_str), "%c", _tag[n]);
      }
  
      uint32_t tag_id = strtol((char *)tmp_str, NULL, 16);
    
      RD_LOG(6, F("RDM6300 tag read: "), false);
      RD_LOG(6, (char *)tmp_str, false);
      RD_LOG(6, ", ", false);
      RD_LOG(6, tag_id, true);
      
      return tag_id;
    }

    //char *name() {return Name;} 
    //virtual const char *name() {return Name;}
    virtual char *name() {return Name;}
  };

  struct R7941E : public Reader {
  public:
    static constexpr char *Name = {( char *)"7941E"};
    
    R7941E() :
      Reader(10, 4, 7, 1)
    {
      LOG(4, F("Loading reader: "));
      LOG(4, name(), true);  
    }
      
    uint32_t processTagData(uint8_t _tag[]) {
      LOG(5, F("R7941E prcs tag inpt: "));
      //RD_PRINT((char *)_tag); // Looks weird cuz some bytes might not be readable ascii characters.
      LOG(5, F("id_begin: "));
      LOG(5, id_begin);
      LOG(5, F(", id_end: "));
      LOG(5, id_end);
      
      uint8_t id_len = (id_end - id_begin +1)*2;
      LOG(5, F(", id_len: "));
      LOG(5, id_len);
      
      LOG(5, F(", checksum (dec): "));
      LOG(5, _tag[8], true);
      
      char id_hex[id_len+1] = ""; // need to initialize this to empty.
  
      for (int n=id_begin; n<=id_end; n++) {
        sprintf(id_hex + strlen(id_hex), "%02x", _tag[n]);
      }
  
      uint32_t tag_id = strtol(id_hex, NULL, 16);
    
      RD_LOG(6, F("R7941E tag read: "), false);
      RD_LOG(6, id_hex, false);
      RD_LOG(6, ", ", false);
      RD_LOG(6, tag_id, true);
            
      return tag_id;
    }

    //char *name() {return Name;}
    //virtual const char *name() {return Name;}
    virtual char *name() {return Name;}
  };

  struct WL125 : public Reader {
  public:
    // NOTE: Consider that this reader (and the rdm6300)
    // yield a string of ascii characters (0-9, A-F),
    // and those characters are THEN used to build the ID
    // of the tag.
    //
    static constexpr char *Name = {( char *)"WL-125"};
    
    WL125() :
      Reader(13, 3, 10, 0)
    {
      LOG(4, F("Loading reader: "));
      LOG(4, name(), true);  
    }
  
    uint32_t processTagData(uint8_t _tag[]) {
    
      LOG(5, F("WL125 prcs tag inpt: "));
      LOG(5, F("id_begin: "));
      LOG(5, id_begin);
      LOG(5, F(", id_end: "));
      LOG(5, id_end);
      
      uint8_t id_len = (id_end - id_begin +1);
      
      LOG(5, F(", id_len: "));
      LOG(5, id_len);
      
      LOG(5, F(", checksum (hex): "));
      LOG(5, _tag[11], 16, true);
  
      char id_char[id_len+1] = ""; // need to initialize this to empty.
      
      for (int n=id_begin; n<=id_end; n++) {
        sprintf(id_char + strlen(id_char), "%c", _tag[n]);
      }
  
      uint32_t tag_id = strtol(id_char, NULL, 16);
    
      RD_LOG(6, F("WL125 read: "), false);
      RD_LOG(6, id_char, false);
      RD_LOG(6, ", ", false);
      RD_LOG(6, tag_id, true);
      
      return tag_id;
    }

    //char *name() {return Name;}
    //virtual const char *name() {return Name;}
    virtual char *name() {return Name;}
  };
  
  
  /***  Static Vars & Functions  ***/

  // See the following for how to create an array of classes, though we might not be using it.
  //   https://stackoverflow.com/questions/10722858/how-to-create-an-array-of-classes-types

  Reader *Reader::GetReader(int index) {
    switch (index) {
      case(1):
        return (new RDM6300);
        break;
      case(2):
        return (new R7941E);
        break;
      case(3):
        return (new WL125);
        break;
        
      default:
        return (new RDM6300);
    }
  }

  char *Reader::NameFromIndex(int index) {
    switch (index) {
      case(1):
        return RDM6300::Name;
        break;
      case(2):
        return R7941E::Name;
        break;
      case(3):
        return WL125::Name;
        break;
        
      default:
        return (char *)"error";
    }
  }

  
