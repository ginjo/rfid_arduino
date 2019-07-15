  #include "tags.h"

  /***  Static Class Vars & Functions  ***/

  // Initializes Tags::TagSet static var.
  Tags Tags::TagSet = Tags();
  

  Tags Tags::Load() { 
    Storage::Load(&TagSet, TAGS_EEPROM_ADDRESS);

    for (int i=0; i < TAG_LIST_SIZE; i++) {
      Serial.print(TagSet.tags[i]); Serial.print(",");
    }
    Serial.println();

    if (GetStoredChecksum() != TagSet.calculateChecksum()) {
      Serial.println(F("LoadTags() checksum mismatch"));
    }

    TagSet.compactTags();
    
    return TagSet;
  } // Load()
  

  uint16_t Tags::GetStoredChecksum() {
    return Storage::GetStoredChecksum(TAGS_EEPROM_ADDRESS);
  }



  /***  Instance Vars & Functions  ***/

  // A tag-id is 32 bit for a max of 4,294,967,295 unique combinations
  // NOTE: The API here may change in future, when the higher-frequency
  //       (UHF?) readers are accomodated in this class.
  //uint32_t tag_array[TAG_LIST_SIZE];
  
  // Creates alias of tag_array. See https://stackoverflow.com/questions/6827610/when-declaring-a-reference-to-an-array-of-ints-why-must-it-be-a-reference-to-a
  //typedef uint32_t array_type[TAG_LIST_SIZE];
  //array_type& tags = tag_array;

  Tags::Tags() : Storage("tags") {}

  void Tags::saveTags() {
    compactTags();
    
    unsigned int stored_checksum;
    EEPROM.get(TAGS_EEPROM_ADDRESS, stored_checksum);
    unsigned int loaded_checksum = calculateChecksum();

    if (loaded_checksum == stored_checksum) {
      Serial.print(F("Tags::TagSet.saveTags() aborted, checksums already match: 0x"));
      Serial.print(loaded_checksum, 16);
      return;
    }

    Serial.print(F("Saving tags with checksum 0x"));
    Serial.print(loaded_checksum, 16);
    Serial.print(F(" to address "));
    Serial.println(TAGS_EEPROM_ADDRESS);
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      Serial.print(tags[i]); Serial.print(",");
    }
    Serial.println();
    
    EEPROM.put(TAGS_EEPROM_ADDRESS, loaded_checksum);
    EEPROM.put(TAGS_EEPROM_ADDRESS+4, tags);
  }

  int Tags::countTags(){
    int n = 0;
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      if (tags[i] > 0) n++;
    }
    return n;
  }

  int Tags::getTagIndex(uint32_t tag) {
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      if (tags[i] == tag) return i;
    }
    return -1;
  }

  void Tags::compactTags() {
    int n=-1;
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      if (tags[i] == 0 && n < 0) {
        n=i;
      } else if ( tags[i] > 0 && i>0 && n>=0) {
        tags[n] = tags[i];
        tags[i] = 0;
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

    tags[tag_count] = new_tag;
    if (tags[tag_count] == new_tag) {
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
      tags[index] = 0;
      saveTags();
      return 0;
    } else {
      return 1;
    }
  }

  int Tags::deleteAllTags() {
    Serial.println(F("deleteAllTags()"));
    memset(tags, 0, TAG_LIST_SIZE*4);
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
