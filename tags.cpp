  #include "tags.h"


  /***  Static Class Vars & Functions  ***/

  // Initializes Tags::TagSet static var.
  Tags Tags::TagSet = Tags();
  

  Tags Tags::Load() {
    Serial.println(F("Tags::Load() BEGIN"));
    Tags * tag_set = Storage::Load(&TagSet, TAGS_EEPROM_ADDRESS);

    for (int i=0; i < TAG_LIST_SIZE; i++) {
      Serial.print(tag_set->tag_array[i]); Serial.print(",");
    }
    Serial.println();

    if (GetStoredChecksum() != tag_set->calculateChecksum()) {
      Serial.println(F("LoadTags() checksum mismatch"));
    }

    tag_set->compactTags();

    Serial.println(F("Tags::Load() END"));
    return TagSet;
  } // Load()
  

  uint16_t Tags::GetStoredChecksum() {
    return Storage::GetStoredChecksum(TAGS_EEPROM_ADDRESS);
  }



  /***  Instance Vars & Functions  ***/

  Tags::Tags() :
    Storage("tags"),
    tag_array {}
  {;}

  void Tags::saveTags() {
    compactTags();
    
    //  unsigned int stored_checksum;
    //  EEPROM.get(TAGS_EEPROM_ADDRESS, stored_checksum);
    //  unsigned int loaded_checksum = calculateChecksum();

    //  if (loaded_checksum == stored_checksum) {
    //    Serial.print(F("Tags::TagSet.saveTags() aborted, checksums already match: 0x"));
    //    Serial.print(loaded_checksum, 16);
    //    return;
    //  }

    Serial.print(F("Saving tags with checksum 0x"));
    Serial.print(calculateChecksum(), 16);
    Serial.print(F(" to address "));
    Serial.println(TAGS_EEPROM_ADDRESS);
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      Serial.print(tag_array[i]); Serial.print(",");
    }
    Serial.println();

    Storage::save(TAGS_EEPROM_ADDRESS);
    
    //  EEPROM.put(TAGS_EEPROM_ADDRESS, loaded_checksum);
    //  EEPROM.put(TAGS_EEPROM_ADDRESS+4, tag_array);
  }


  int Tags::countTags(){
    int n = 0;
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      if (tag_array[i] > 0) n++;
    }
    return n;
  }

  int Tags::getTagIndex(uint32_t tag) {
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      if (tag_array[i] == tag) return i;
    }
    return -1;
  }

  void Tags::compactTags() {
    int n=-1;
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      if (tag_array[i] == 0 && n < 0) {
        n=i;
      } else if ( tag_array[i] > 0 && i>0 && n>=0) {
        tag_array[n] = tag_array[i];
        tag_array[i] = 0;
        i = n;
        n = -1;
      }
    }
  }

  int Tags::addTag(uint32_t new_tag) {
    Serial.print(F("addTag() "));
    Serial.println(new_tag);
    compactTags();
    int tag_count = countTags();
    
    if(new_tag < 1) {
      Serial.println(F("addTag() aborted: Invalid code"));
      return 1;
    } else if (tag_count >= TAG_LIST_SIZE) {
      Serial.println(F("addTag() failed: Full"));
      return 2;
    } else if (getTagIndex(new_tag) >=0) {
      Serial.println(F("addTag() failed: Dupe"));
      return 3;
    }

    tag_array[tag_count] = new_tag;
    if (tag_array[tag_count] == new_tag) {
      saveTags();
      Serial.println(F("addTag() success"));
      return 0;
    } else {
      Serial.println(F("addTag() failed: Unknown error"));
      return -1;
    }
  } // addTag()

  int Tags::deleteTag(uint32_t deleteable_tag) {
    Serial.print(F("deleteTag(): "));
    Serial.println(deleteable_tag);
    int tag_index = getTagIndex(deleteable_tag);
    return deleteTagIndex(tag_index);
  }

  int Tags::deleteTagIndex(int index) {
    Serial.print(F("deleteTagIndex(): "));
    Serial.println(index);
    if (index >= 0) {
      tag_array[index] = 0;
      saveTags();
      return 0;
    } else {
      return 1;
    }
  }

  int Tags::deleteAllTags() {
    Serial.println(F("deleteAllTags()"));
    memset(tag_array, 0, TAG_LIST_SIZE*4);
    //Tags = new uint32_t[TAG_LIST_SIZE];
    saveTags();
    return 0;
  }

  // This was the original RFID::getTagsChecksum()
  //
  //  unsigned int getTagsChecksum() {
  //    unsigned char *obj = (unsigned char *) Tags;
  //    unsigned int len = sizeof(Tags);
  //    unsigned int xxor = 0;
  //
  //    // Converts to 16-bit checksum, and handles odd bytes at end of obj.
  //    for ( unsigned int i = 0 ; i < len ; i+=2 ) {
  //      xxor = xxor ^ ((obj[i]<<8) | (i==len-1 ? 0 : obj[i+1]));
  //    }
  //    
  //    return xxor;
  //  }
