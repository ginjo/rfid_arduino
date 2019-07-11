#ifndef __STORAGE_H__
#define __STORAGE_H__

  #include <Arduino.h>
  #include <EEPROM.h>

	#define CHECKSUM_SIZE 9
  #define STORAGE_NAME_SIZE 16

  struct Storage {

		static constexpr const char *storage_name = "base-class";
    static constexpr const int eeprom_address = 0;
    static constexpr const int checksum_size = 0;
    //static const int eeprom_address;
    //static const int checksum_size;
		
		static uint16_t GetStoredChecksum();

    // Template implementations cannot be split between files,
    // therefore all template functions must be declared/defined together.
    template <class T>
    static T *Load(T * object_ref) {
      uint16_t stored_checksum = GetStoredChecksum();
      uint16_t loaded_checksum;
      EEPROM.get(eeprom_address+checksum_size, object_ref);
      loaded_checksum = object_ref->calculateChecksum();
  
      if (stored_checksum != loaded_checksum) {
        if (Serial) Serial.println(F("Storage::Load() checksum mismatch"));
      }
      
      // Should this be de-referenced?
      return object_ref;
    }

    Storage();

    int save();
		uint16_t calculateChecksum();
    
  };

#endif // __STORAGE_H__
