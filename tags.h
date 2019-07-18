#ifndef __TAGS__
#define __TAGS__

  #define TAG_LIST_SIZE 10
  #define TAGS_EEPROM_ADDRESS 700

  #include <Arduino.h>
  #include <string.h>
  #include "storage.h"

  // A tag-id is a 32 bit unsigned integer for a max of 4,294,967,295 unique combinations
  //
  // TODO: √ Save the eeprom_address with the tag-set record.
  //       Make all tag functions instance methods, including getStoredChecksum().
  //       The only function that should be static is Load, and you pass it an eeprom_address.
  //       After that, the Storage instance handles everything about where to load/store
  //       the checksum and the data. This makes the persistance layer portable,
  //       extensible, and scaleable.
  //       
  // TODO: √ Split this into .h and .cpp files.
  // 
  // TODO: This class now works with RFID and SerialMenu, but storage_name is getting garbled.
  

  // This is for aliasing RFID::Tags to the new Tags class (Tags::TagSet.tags).
  // It can also be used as a type for any array that holds 32-bit tag ids.
  typedef uint32_t TagArray[TAG_LIST_SIZE];
  
 
  class Tags : public Storage<Tags> {
  public:


    /***  Static Class Vars & Functions  ***/
  
    static Tags TagSet;

    static Tags* Load(Tags* tag_set = &TagSet, int _eeprom_address = TAGS_EEPROM_ADDRESS);
    

    /***  Instance Vars & Functions  ***/

    // A tag-id is 32 bit for a max of 4,294,967,295 unique combinations
    // NOTE: The API here may change in future, when the higher-frequency
    //       (UHF?) readers are accomodated in this class.
    // This cannot be of type TagArray, since that is used to alias this variable.
    uint32_t tag_array[TAG_LIST_SIZE];
    
    // Creates alias of tag_array.
    // See typdef above.
    // See https://stackoverflow.com/questions/6827610/when-declaring-a-reference-to-an-array-of-ints-why-must-it-be-a-reference-to-a
    TagArray& tags = tag_array;

    Tags();

    void save();
    int countTags();
    int getTagIndex(uint32_t);
    void compactTags();
    int addTag(uint32_t);
    int deleteTag(uint32_t);
    int deleteTagIndex(int);
    int deleteAllTags();

  }; // Tags


#endif
