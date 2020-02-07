  #include "tags.h"
  #include "storage.h"
  #include "logger.h"
  
  
  /*  Constructors  */

  Tags::Tags() :
    Storage("tags", TAGS_EEPROM_ADDRESS),
    tag_array {}
  {
    // Why do we need this?
    //  if (strcmp(storage_name, "") == 0 || storage_name[0] == 0) {
    //    strlcpy(storage_name, "tags-default", sizeof(storage_name));
    //  }
  }
  
  
	/*  Static Vars & Functions  */
  
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
  Tags *Tags::Load(Tags* _tag_set, int _eeprom_address) {
    TA_LOG(5, F("Tags.Load() BEGIN"), true);

    
    
    bool rslt = Storage::Load(_tag_set, _eeprom_address);
		
		#ifdef TA_DEBUG
			if (LogLevel() >= 5U) {
				for (int i=0; i < TAG_LIST_SIZE; i++) {
					TA_LOG(5, (uint32_t)(_tag_set->tag_array[i])); TA_LOG(5, ",", false);
				}
			}
			TA_LOG(5, "", true);
		#endif

    if (! rslt) {
      LOG(3, F("Tags.Load() checksum mismatch"), true);
    } else {
      LOG(4, F("Tags loaded"), true);
    }

    _tag_set->compactTags();

    TA_LOG(6, F("Tags.Load() END"), true);
    
    return _tag_set;
  } // Load()


  /*  Instance Functions  */

  // Saves this stored tag set in EEPROM.
  void Tags::save() {
    compactTags();

    TA_LOG(5, F("Saving tags with chksm 0x"), false);
    TA_LOG(5, calculateChecksum(), 16, false);
    TA_LOG(5, F(" to address "), false);
    TA_LOG(5, eeprom_address, true);
    #ifdef TA_DEBUG
      for (int i=0; i < TAG_LIST_SIZE; i++) {
        TA_LOG(5, tag_array[i]); LOG(4, ",", false);
      }
      TA_LOG(5, "", true);
    #endif

    Storage::save(eeprom_address);
  }

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
    LOG(5, F("addTag() "));
    LOG(5, new_tag, true);
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

  void Tags::printTags(Stream *serial_port) {
    serial_port->print(F("Tags, chksm 0x"));
    serial_port->print(checksum, 16);
    serial_port->print(F(", size "));
    serial_port->println(sizeof(this));
    
    for (int n = 0; n < TAG_LIST_SIZE; n ++) {
      if (tag_array[n] > 0) {
        char out[4 + 10 + 2 + 10 + 1] = "";
        sprintf_P(out, PSTR("%2i  %-10lu  0x%lx"), n+1, tag_array[n], tag_array[n]);
        serial_port->println(out);
      }
    }
  }
  
