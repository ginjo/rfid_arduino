#ifndef __STORAGE_H__
#define __STORAGE_H__

  #include <Arduino.h>
  #include <EEPROM.h>

	#define STORAGE_CHECKSUM_SIZE 9
  #define STORAGE_NAME_SIZE 16

  /* All calls to static vars from base class must use templates
   *  to qualify the derived class name. See my subclassing & template
   *  cpp examples.
   */

  struct Storage {
    
		static uint16_t GetStoredChecksum(int);
    
    //static Storage * Load(Storage*, int, int);
    template <class T>
    static T* Load (T * object_ref, int eeprom_address, int checksum_size) {
      Serial.println(F("Storage::Load() BEGIN"));
      uint16_t stored_checksum = GetStoredChecksum(eeprom_address);
  
      uint16_t loaded_checksum;
      EEPROM.get(eeprom_address+checksum_size, object_ref);
      loaded_checksum = object_ref->calculateChecksum();
  
      Serial.print(F("eeprom_address ")); Serial.println(eeprom_address);
      Serial.print(F("checksum_size ")); Serial.println(checksum_size);
      Serial.print(F("stored_checksum ")); Serial.println(stored_checksum, 16);
      Serial.print(F("loaded_checksum ")); Serial.println(loaded_checksum, 16);
      Serial.print(F("object_ref->storage_name '")); Serial.print(object_ref->storage_name); Serial.println("'");
      Serial.print(F("sizeof(*object_ref) ")); Serial.println(sizeof(*object_ref));
  
      if (stored_checksum != loaded_checksum) {
        Serial.println(F("Storage::Load() checksum mismatch"));
      }
  
      Serial.println(F("Storage::Load() END"));
      return object_ref;
    }

    char storage_name[STORAGE_NAME_SIZE];

    Storage(const char*);
    int save(int, int);
		uint16_t calculateChecksum();
    
  };

#endif // __STORAGE_H__
