#include "storage.h"

	// Make sure you put this line in each subclass definition,
	// and customize it with an appropriate name.
	//static constexpr char *storage_name = "base-class";
	
	static uint16_t Storage::GetStoredChecksum() {
		uint16_t stored_checksum;
		return EEPROM.get(eeprom_address, stored_checksum);
	}
	
	template <class Subclass>
	static Subclass Storage::Load(Subclass * object_ref) {
		uint16_t stored_checksum = GetStoredChecksum();
    uint16_t loaded_checksum;
    EEPROM.get(eeprom_address+CHECKSUM_SIZE, object_ref);
    loaded_checksum = object_ref.calculateChecksum();

    if (stored_checksum != loaded_checksum) {
      if (Serial) Serial.println(F("Storage::Load() checksum mismatch"));
    }
    
		// Should this be de-referenced?
    return object_ref;
	}


  Storage::Storage() {}
  
  // Saves this Storage instance to the correct storage address.
  // Sub-classes, like Settings, should carry the info about
  // what address to use.
  int Storage::save() {
		uint16_t stored_checksum;
		EEPROM.get(eeprom_address, stored_checksum);
		uint16_t calculated_checksum = calculateChecksum();
		
		if (stored_checksum != calculated_checksum) {}
	    EEPROM.put(address, checksum);
			// Must dereference here, or you store pointer address instead of data.
			return EEPROM.put(address+CHECKSUM_SIZE, *this);
		} else {
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

