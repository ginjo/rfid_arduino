  #include "tags.h"


  /***  Static Class Vars & Functions  ***/

  // Defines/initializes Tags::TagSet static var.
  Tags Tags::TagSet = Tags();  

  // TODO: Decouple these functions from the TagSet static var.
  // We should be able to load any tag set from any address
  // without affecting Tags::TagSet var.
  //
  // Ok, I think this (below) provides decoupled solution as an option.
  //
  // Tags::TagSet initializes with Tags() (see above),
  // and Tags::Load() defaults to filling Tags::TagSet with loaded result (see tags.h).
  // Then the following is called from the main .ino file.
  //   Tags::Load();
  // Note that Tags::Load can be passed any eeprom-address, and any pointer to a Tags var,
  // and it will happily use those custom vars/values to load a custom tag-set into a custom var.
  //
  // TODO: Is this decoupled option appropriate for the Settings class too?
  //
  Tags* Tags::Load(Tags* tag_set, int _eeprom_address) {
    Serial.println(F("Tags::Load() BEGIN"));
    
    Storage::Load(tag_set, _eeprom_address);

    for (int i=0; i < TAG_LIST_SIZE; i++) {
      Serial.print(tag_set->tag_array[i]); Serial.print(",");
    }
    Serial.println();

    if (! tag_set->checksumMatch()) {
      Serial.println(F("Tags::Load() checksum mismatch, creating new tag-set"));
      tag_set = new Tags();
      tag_set->save();
    }

    tag_set->compactTags();

    Serial.println(F("Tags::Load() END"));
    return tag_set;
  } // Load()
  

  //  uint16_t Tags::GetStoredChecksum() {
  //    return Storage::GetStoredChecksum(TAGS_EEPROM_ADDRESS);
  //  }



  /***  Instance Vars & Functions  ***/


  /* Constructors */

  Tags::Tags() :
    Storage("tags", TAGS_EEPROM_ADDRESS),
    tag_array {}
  {
    if (strcmp(storage_name, "") == 0 || storage_name[0] == 0) {
      strlcpy(storage_name, "tags-default", sizeof(storage_name));
    }
  }


  /* Storage operations */

  void Tags::save() {
    compactTags();
    
    //  if (strcmp(storage_name, "") == 0 || storage_name[0] == 0) {
    //    strlcpy(storage_name, "tags-saved", sizeof(storage_name));
    //  }
    
    //  unsigned int stored_checksum;
    //  EEPROM.get(TAGS_EEPROM_ADDRESS, stored_checksum);
    //  unsigned int loaded_checksum = calculateChecksum();

    //  if (loaded_checksum == stored_checksum) {
    //    Serial.print(F("Tags::TagSet.save() aborted, checksums already match: 0x"));
    //    Serial.print(loaded_checksum, 16);
    //    return;
    //  }

    Serial.print(F("Saving tags with checksum 0x"));
    Serial.print(calculateChecksum(), 16);
    Serial.print(F(" to address "));
    Serial.println(eeprom_address);
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      Serial.print(tag_array[i]); Serial.print(",");
    }
    Serial.println();

    Storage::save(eeprom_address);
  }


  /* Tag operations */

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
      save();
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
      save();
      return 0;
    } else {
      return 1;
    }
  }

  int Tags::deleteAllTags() {
    Serial.println(F("deleteAllTags()"));
    memset(tag_array, 0, TAG_LIST_SIZE*4);
    //Tags = new uint32_t[TAG_LIST_SIZE];
    save();
    return 0;
  }
