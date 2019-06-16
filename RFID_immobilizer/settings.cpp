#include "settings.h"

  // TODO: I think Storage should be generic storage class,
  // and Settings, State, whatever should be subclassed.

  // TODO: Implement actual EEPROM storage.

  Storage::Storage() :
    // ultimate valid-tag timeout
    TAG_LAST_READ_TIMEOUT(30), // seconds

    // time between attempts to listen to reader
    TAG_READ_SLEEP_INTERVAL(1000), // millis

    // off duration during reader power cycle
    READER_CYCLE_LOW_DURATION(150), // millis

    // on duration before reader next power cycle
    // also is duration before 'aging' stage begins
    READER_CYCLE_HIGH_DURATION(15), // seconds

    // controls reader power thru mosfet
    READER_POWER_CONTROL_PIN(5),

    // saved proximity state (TODO: should be separate setting)
    //proximity_state(EEPROM.read(0)), // 0 = false, 1 = true
    // Use this for debugging
    proximity_state(1),
    
    // idle time before admin mode switches to run mode
    // should be greater than READER_CYCLE_HIGH_DURATION
    admin_timeout(30) // seconds
  {;}

  // TODO: This is temp for testing.
  // The data should ultimately be pulled from EEPROM.
  // Defaults should be in constructor, if that works.
  extern Storage Settings = {};

  // a reference (alias?) from S to Settings
  extern Storage& S = Settings;


  int Storage::updateProximityState(int _state) {
    int previous_proximity_state = proximity_state;
    proximity_state = _state;
    //  Serial.print(F("Storing proximity_state: "));
    //  Serial.println(proximity_state);
    if (proximity_state != previous_proximity_state) {
      Serial.print(F("Calling EEPROM.update with proximity_state: "));
      Serial.println(proximity_state);
      // Disable this for debugging,
      //EEPROM.update(0, proximity_state);
      // and enable this for debugging.
      state_dev_tmp = proximity_state;
    }
    return proximity_state;
  }

  // Updates a setting given setting index and data.
  bool Storage::updateSetting(int _index, char _data[]) {

    Serial.print("S.updateSetting() with index: ");
    Serial.println(_index);
    
    switch (_index) {
      case 7:
        Serial.print("S.updateSetting() updating 'admin_timeout' with: ");
        admin_timeout = strtol(_data, NULL, 10);
        Serial.println(admin_timeout);
        return true;
        break;
      case 1:
        Serial.print("S.updateSetting() updating 'TAG_LAST_READ_TIMEOUT' with: ");
        TAG_LAST_READ_TIMEOUT = strtol(_data, NULL, 10);
        Serial.println(TAG_LAST_READ_TIMEOUT);
        return true;
        break;
      case 4:
        Serial.print("S.updateSetting() updating 'READER_CYCLE_HIGH_DURATION' with: ");
        READER_CYCLE_HIGH_DURATION = strtol(_data, NULL, 10);
        Serial.println(READER_CYCLE_HIGH_DURATION);
        return true;
        break;
    }

    return false;
  }


  
  
