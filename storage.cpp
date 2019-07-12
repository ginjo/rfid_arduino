#include <EEPROM.h>
#include "storage.h"

	// Make sure you put this line in each subclass definition,
	// and customize it with an appropriate name.
	const char *storage_name = "base-class";
	
	uint16_t Storage::GetStoredChecksum() {
		uint16_t stored_checksum;
		return EEPROM.get(eeprom_address, stored_checksum);
	}

  Storage::Storage() {}
  
  // Saves this Storage instance to the correct storage address.
  // Sub-classes, like Settings, should carry the info about
  // what address to use.
  int Storage::save() {
		uint16_t stored_checksum;
		EEPROM.get(eeprom_address, stored_checksum);
		uint16_t calculated_checksum = calculateChecksum();

    Serial.print(F("Storage::save() ")); Serial.print(storage_name);
    Serial.print(F(" to address ")); Serial.print(eeprom_address+checksum_size);
    Serial.print(F(" with stored/calc chksm 0x")); Serial.print(stored_checksum, 16);
    Serial.print(F(" 0x")); Serial.print(calculated_checksum, 16);
    Serial.print(F(" of size ")); Serial.println(sizeof(*this));
		
		if (stored_checksum != calculated_checksum) {
      Serial.print(F("Storage::save() performing EEPROM.put()"));
	    EEPROM.put(eeprom_address, calculated_checksum);
			// Must dereference here, or you store pointer address instead of data.
		  EEPROM.put(eeprom_address+checksum_size, *this);
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
