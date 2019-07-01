#include "storage.h"

  void Storage::save() {;}

  Storage::Storage(const char _storage_name[])
  {
    strncpy(storage_name, _storage_name, storage_name_size);  
  }
  
  // Saves this Storage instance to the correct storage address.
  // Sub-classes, like Settings, should carry the info about
  // what address to use.
  void Storage::save(int eeprom_address) {
    DPRINT(F("Storage::save() object: ")); DPRINT(storage_name);
    DPRINT(F(", eeprom address: ")); DPRINTLN(eeprom_address);
    //EEPROM.put(eeprom_address, this);
  }

    //  static Storage Storage::load(int eeprom_address) {
    //    Storage result;
    //
    //    DPRINT("Storage::load() from eeprom adrs: "); DPRINTLN(eeprom_address);
    //    
    //    EEPROM.get(eeprom_address, result);
    //
    //    // TODO: Use a different test to validate it's a real Storage object.
    //    if (strcmp((char *)result.storage_name, "") == 1) { // 1 == if this test fails
    //      DPRINT(F("Storage::load() loaded: ")); DPRINTLN(result.storage_name);
    //      return result;
    //    // This part probably shouldn't go here.
    //    //} else {
    //    //  Storage result;
    //    //  DPRINTLN(F("loadStorage() saving new settings object"));
    //    //  result.save();
    //    //  return result;
    //    }
    //  }
