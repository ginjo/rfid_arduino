#ifndef __STORAGE_H__
#define __STORAGE_H__

  #include <Arduino.h>
  #include <EEPROM.h>

	//#define STORAGE_CHECKSUM_SIZE 9
  #define STORAGE_NAME_SIZE 16

  /* All calls to static vars from base class must use templates
   *  to qualify the derived class name. See my subclassing & template
   *  cpp examples.
   *  
   *  This base class, Storage, provides EEPROM persistance for any sublclass instance.
   *  
   *  The key to making this work is to make the base class a template class, and to
   *  pass in the subclass name when declaring the subclass. This is referred to as the CRTP
   *  pattern "Curiously Recurring Template Pattern". 
   */

  template <class T>
  struct Storage {
    
		//static uint16_t GetStoredChecksum(int);
    
    //static Storage * Load(Storage*, int, int);
    //template <class T>
    static T* Load (T * object_ref, int eeprom_address) {  //, int checksum_size) {
    //static T* Load (int eeprom_address, int checksum_size) {
      Serial.println(F("Storage::Load() BEGIN"));
      
      uint16_t stored_checksum = GetStoredChecksum(eeprom_address);
      uint16_t loaded_checksum;
      int checksum_size = sizeof(stored_checksum);
      
      EEPROM.get(eeprom_address+checksum_size, *object_ref); // According to docs .get() also expects data, not pointer.
      loaded_checksum = object_ref->calculateChecksum();
  
      Serial.print(F("eeprom_address ")); Serial.println(eeprom_address);
      Serial.print(F("checksum_size ")); Serial.println(checksum_size);
      Serial.print(F("stored_checksum ")); Serial.println(stored_checksum, 16);
      Serial.print(F("loaded_checksum ")); Serial.println(loaded_checksum, 16);
      Serial.print(F("object_ref->storage_name '")); Serial.print(object_ref->storage_name); Serial.println("'");
      Serial.print(F("sizeof(*object_ref) ")); Serial.println(sizeof(*object_ref));
      Serial.print(F("sizeof(T) ")); Serial.println(sizeof(T));
  
      if (stored_checksum != loaded_checksum) {
        Serial.println(F("Storage::Load() checksum mismatch"));
        stored_checksum = (uint16_t)0;
        EEPROM.put(eeprom_address, stored_checksum);
      }
  
      Serial.println(F("Storage::Load() END"));
      return object_ref;
    }

    char storage_name[STORAGE_NAME_SIZE];
    //Storage(const char*);
    //int save(int, int);
		//uint16_t calculateChecksum();

    static uint16_t GetStoredChecksum(int eeprom_address) {
      uint16_t stored_checksum = 0;
      EEPROM.get(eeprom_address, stored_checksum);
      Serial.print(F("Storage::GetStoredChecksum(int): 0x")); Serial.println(stored_checksum, 16);
      return stored_checksum;
    }
  
  
    Storage(const char *_storage_name) {
      strlcpy(storage_name, _storage_name, STORAGE_NAME_SIZE);
    }
    
    // Saves this Storage instance to the correct storage address.
    // Sub-classes, like Settings, should carry the info about
    // what address to use.
    int save(int eeprom_address) {  //, int checksum_size) {
      Serial.println(F("Storage::save() BEGIN"));

      uint16_t stored_checksum = GetStoredChecksum(eeprom_address);
      uint16_t calculated_checksum = calculateChecksum();
      int checksum_size = sizeof(stored_checksum);

      Serial.print(F("Storage::save() '")); Serial.print(storage_name);
      Serial.print(F("' to address ")); Serial.print(eeprom_address+checksum_size);
      Serial.print(F(" with stored/calced chksm 0x")); Serial.print(stored_checksum, 16);
      Serial.print(F(" 0x")); Serial.print(calculated_checksum, 16);
      //Serial.print(F(" sizeof(*this) ")); Serial.println(sizeof(*this));
      Serial.print(F(" sizeof(T) ")); Serial.println(sizeof(T));
      
      if (stored_checksum != calculated_checksum) {
        Serial.println(F("Storage::save() chksm mismatch, performing EEPROM.put()"));
        
        EEPROM.put(eeprom_address, calculated_checksum);
        // Must use dereferenced data here, or you store pointer address instead of data.
        // The use of the class template is very important here, as the eepprom
        // functions don't seem to get it when called from a subclass.
        EEPROM.put(eeprom_address+checksum_size, *(T*)this);
  
        Serial.println(F("Storage::save() END"));
        return 0;
      } else {
        Serial.println(F("Storage::save() EEPROM update not necessary"));
        Serial.println(F("Storage::save() END"));
        return 1;
      }
    }
  
    uint16_t calculateChecksum() {
      unsigned char *obj = (unsigned char *) (T*)this;
      //uint16_t len = sizeof(*this); // de-references pointer, so we get size of actual data.
      uint16_t len = sizeof(T);
      uint16_t xxor = 00;
  
      // Converts to 16-bit checksum, and handles odd bytes at end of obj.
      for ( uint16_t i = 0 ; i < len ; i+=2 ) {
        xxor = xxor ^ ((obj[i]<<8) | (i==len-1 ? 0 : obj[i+1]));
      }
  
      Serial.print(F("Storage::claculateChecksum() 0x")); Serial.print(xxor, 16);
      Serial.print(F(" for storage_name '")); Serial.print(storage_name);
      Serial.print(F("' of size ")); Serial.println(len);
      
      return xxor;    
    }
    
  };

#endif // __STORAGE_H__
