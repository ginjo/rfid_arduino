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
      LOG(F("Loading reader: "));
      LOG(name(), true);  
    }
  
    uint32_t processTagData(uint8_t _tag[24]) {
  
      RD_PRINT(F("RDM6300 prcs tag inpt: "));
      RD_PRINTLN(strtol((char *)_tag, NULL, HEX));
  
      uint8_t id_len = id_end - id_begin;
      char tmp_str[id_len+1] = "";
      
      for (int n=id_begin; n<=id_end; n++) {
        sprintf(tmp_str + strlen(tmp_str), "%c", _tag[n]);
      }
  
      uint32_t tag_id = strtol((char *)tmp_str, NULL, 16);
    
      RD_PRINT(F("RDM6300 tag read: "));
      RD_PRINT((char *)tmp_str);
      RD_PRINT(", ");
      RD_PRINTLN(tag_id);
      
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
      LOG(F("Loading reader: "));
      LOG(name(), true);  
    }
      
    uint32_t processTagData(uint8_t _tag[]) {
      RD_PRINT(F("R7941E prcs tag inpt: "));
      //RD_PRINT((char *)_tag);
      RD_PRINT(F("id_begin: "));
      RD_PRINT(id_begin);
      RD_PRINT(F(", id_end: "));
      RD_PRINT(id_end);
      
      uint8_t id_len = (id_end - id_begin +1)*2;
      RD_PRINT(F(", id_len: "));
      RD_PRINT(id_len);
      
      RD_PRINT(F(", checksum (dec): "));
      RD_PRINTLN(_tag[8]);
      
      char id_hex[id_len+1] = ""; // need to initialize this to empty.
  
      for (int n=id_begin; n<=id_end; n++) {
        sprintf(id_hex + strlen(id_hex), "%02x", _tag[n]);
      }
  
      uint32_t tag_id = strtol(id_hex, NULL, 16);
    
      RD_PRINT(F("R7941E tag read: "));
      RD_PRINT(id_hex);
      RD_PRINT(", ");
      RD_PRINTLN(tag_id);
            
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
      LOG(F("Loading reader: "));
      LOG(name(), true);  
    }
  
    uint32_t processTagData(uint8_t _tag[]) {
  
      //Reader::preProcessTagData();
  
      RD_PRINT(F("WL125 prcs tag inpt: "));
      RD_PRINT(F("id_begin: "));
      RD_PRINT(id_begin);
      RD_PRINT(F(", id_end: "));
      RD_PRINT(id_end);
      
      uint8_t id_len = (id_end - id_begin +1);
      
      RD_PRINT(F(", id_len: "));
      RD_PRINT(id_len);
      
      RD_PRINT(F(", checksum (raw bytes): "));
      RD_PRINT(_tag[11]); RD_PRINT(","); RD_PRINTLN(_tag[12]);
  
      char id_char[id_len+1] = ""; // need to initialize this to empty.
      
      for (int n=id_begin; n<=id_end; n++) {
        sprintf(id_char + strlen(id_char), "%c", _tag[n]);
      }
  
      uint32_t tag_id = strtol(id_char, NULL, 16);
    
      RD_PRINT(F("WL125 read: "));
      RD_PRINT(id_char);
      RD_PRINT(", ");
      RD_PRINTLN(tag_id);
      
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

  
