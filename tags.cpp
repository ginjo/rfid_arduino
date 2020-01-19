  #include "tags.h"


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


  /* Storage Operations */

  // Saves this stored tag set in EEPROM.
  void Tags::save() {
    compactTags();

    LOG(4, F("Saving tags with checksum 0x"));
    LOG(4, calculateChecksum(), 16);
    LOG(4, F(" to address "));
    LOG(4, eeprom_address, true);
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      LOG(4, tag_array[i]); LOG(4, ",");
    }
    LOG(4, "", true);

    Storage::save(eeprom_address);
  }


  /* Tag Operations */

  // Counts number of stored tags in this set.
  int Tags::countTags(){
    int n = 0;
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      if (tag_array[i] > 0) n++;
    }
    return n;
  }

  // Gets the array index of the given tag.
  int Tags::getTagIndex(uint32_t tag) {
    for (int i=0; i < TAG_LIST_SIZE; i++) {
      if (tag_array[i] == tag) return i;
    }
    return -1;
  }

  // Removes empty space from tag set, and slides all tags towards front of array (0).
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

  // Adds tag to this set, and saves the set in EEPROM.
  int Tags::addTag(uint32_t new_tag) {
    LOG(4, F("addTag() "));
    LOG(4, new_tag, true);
    compactTags();
    int tag_count = countTags();
    
    if(new_tag < 1) {
      LOG(3, F("addTag() aborted: Invalid code"), true);
      return 1;
    } else if (tag_count >= TAG_LIST_SIZE) {
      LOG(3, F("addTag() failed: Full"), true);
      return 2;
    } else if (getTagIndex(new_tag) >=0) {
      LOG(3, F("addTag() failed: Dupe"), true);
      return 3;
    }

    tag_array[tag_count] = new_tag;
    if (tag_array[tag_count] == new_tag) {
      save();
      LOG(4, F("addTag() success"), true);
      return 0;
    } else {
      LOG(2, F("addTag() failed: Unknown error"), true);
      return -1;
    }
  } // addTag()

  // Deletes a tag from this set.
  int Tags::deleteTag(uint32_t deleteable_tag) {
    LOG(4, F("deleteTag(): "));
    LOG(4, deleteable_tag, true);
    int tag_index = getTagIndex(deleteable_tag);
    return deleteTagIndex(tag_index);
  }

  // Deletes a tag, by index, from this set.
  int Tags::deleteTagIndex(int index) {
    LOG(4, F("deleteTagIndex(): "));
    LOG(4, index, true);
    if (index >= 0) {
      tag_array[index] = 0;
      save();
      return 0;
    } else {
      return 1;
    }
  }

  // Deletes all tags from this set.
  int Tags::deleteAllTags() {
    LOG(4, F("deleteAllTags()"), true);
    memset(tag_array, 0, TAG_LIST_SIZE*4);
    //Tags = new uint32_t[TAG_LIST_SIZE];
    save();
    return 0;
  }



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
    TA_LOG(6, F("Tags::Load() BEGIN"), true);
    
    Storage::Load(tag_set, _eeprom_address);

    LOG(4, F("Tags loaded"), true);
    //if (S.debugMode()) {
    #ifdef TA_DEBUG
      if (LogLevel() >= 5) {
        for (int i=0; i < TAG_LIST_SIZE; i++) {
          LOG(5, tag_set->tag_array[i]); LOG(4, ",");
        }
      }
      LOG(4, "", true);
    #endif

    if (! tag_set->checksumMatch()) {
      LOG(2, F("Tags::Load() checksum mismatch, creating new tag-set"), true);
      tag_set = new Tags();
      /* Don't save tag_set automatically, cuz it overwrites all saved tags.
         Let the user decide whether to overwrite all saved tags by adding a new tag
      */
      //tag_set->save();
    }

    tag_set->compactTags();

    TA_LOG(4, F("Tags::Load() END"), true);
    
    return tag_set;
  } // Load()

  
