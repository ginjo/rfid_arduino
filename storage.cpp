#include <EEPROM.h>
#include "storage.h"

  Storage *Storage::Load(Storage * object_ref, int eeprom_address, int checksum_size) {
    uint16_t stored_checksum = GetStoredChecksum(eeprom_address);
    uint16_t loaded_checksum;
    EEPROM.get(eeprom_address+checksum_size, object_ref);
    loaded_checksum = object_ref->calculateChecksum();

    Serial.print(F("Storage::Load() ")); Serial.print(object_ref->storage_name);
    Serial.print(F(" from address ")); Serial.print(eeprom_address+checksum_size);
    Serial.print(F(" with stored/calc chksm 0x")); Serial.print(stored_checksum, 16);
    Serial.print(F(" 0x")); Serial.print(loaded_checksum, 16);
    Serial.print(F(" of size ")); Serial.println(sizeof(*object_ref));
    
    if (stored_checksum != loaded_checksum) {
      if (Serial) Serial.println(F("Storage::Load() checksum mismatch"));
    }
    
    // Should this be de-referenced?
    return object_ref;
  }
	
	uint16_t Storage::GetStoredChecksum(int eeprom_address) {
		uint16_t stored_checksum;
		return EEPROM.get(eeprom_address, stored_checksum);
	}


  Storage::Storage(const char *_storage_name) {
    strncpy(storage_name, _storage_name, STORAGE_NAME_SIZE);
  }
  
  // Saves this Storage instance to the correct storage address.
  // Sub-classes, like Settings, should carry the info about
  // what address to use.
  int Storage::save(int eeprom_address, int checksum_size) {
		uint16_t stored_checksum;
		EEPROM.get(eeprom_address, stored_checksum);
		uint16_t calculated_checksum = calculateChecksum();

    Serial.print(F("Storage::save() ")); Serial.print(storage_name);
    Serial.print(F(" to address ")); Serial.print(eeprom_address+checksum_size);
    Serial.print(F(" with stored/calc chksm 0x")); Serial.print(stored_checksum, 16);
    Serial.print(F(" 0x")); Serial.print(calculated_checksum, 16);
    Serial.print(F(" of size ")); Serial.println(sizeof(*this));
		
		if (stored_checksum != calculated_checksum) {
      Serial.print(F("Storage::save() (would be) performing EEPROM.put()"));
	    //EEPROM.put(eeprom_address, calculated_checksum);
			// Must dereference here, or you store pointer address instead of data.
		  //EEPROM.put(eeprom_address+checksum_size, *this);
      return 0;
		} else {
      Serial.print(F("Storage::save() EEPROM update not necessary"));
			return 1;
		}
  }

	uint16_t Storage::calculateChecksum() {
    unsigned char *obj = (unsigned char *) this;
    uint16_t len = sizeof(*this); // de-references pointer, so we get size of actual data.
    uint16_t xxor = 0;

    // Converts to 16-bit checksum, and handles odd bytes at end of obj.
    for ( uint16_t i = 0 ; i < len ; i+=2 ) {
      xxor = xxor ^ ((obj[i]<<8) | (i==len-1 ? 0 : obj[i+1]));
    }
    
    return xxor;		
	}
