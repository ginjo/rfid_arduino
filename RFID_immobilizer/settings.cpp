#include "settings.h"

  int Storage::updateProximityState(int _state) {
    int previous_proximity_state = proximity_state;
    proximity_state = _state;
    //  Serial.print(F("Storing proximity_state: "));
    //  Serial.println(proximity_state);
    if (proximity_state != previous_proximity_state) {
      Serial.print(F("Calling EEPROM.update with proximity_state: "));
      Serial.println(proximity_state);
      // Disable this for debugging,
      EEPROM.update(0, proximity_state);
      // and enable this for debugging.
      //state_dev_tmp = proximity_state;
    }
    return proximity_state;
  }

  Storage::Storage() :
    TAG_LAST_READ_TIMEOUT(25),
    TAG_READ_SLEEP_INTERVAL(1000),
    READER_CYCLE_LOW_DURATION(150),
    READER_CYCLE_HIGH_DURATION(5000),
    READER_POWER_CONTROL_PIN(5),
    proximity_state(EEPROM.read(0)),
    admin_timeout(15)
  {;}

  // TODO: This is temp for testing.
  // The data should ultimately be pulled from EEPROM.
  // Defaults should be in constructor, if that works.
  extern Storage Settings = {};
  //  extern Storage Settings = {
  //
  //    // RFID class constants
  //    //  14,  // RDM63000
  //    //  10,    // 7941E
  //    
  //    25,    // TAG_LAST_READ_TIMEOUT       seconds
  //    1000,  // TAG_READ_SLEEP_INTERVAL           ms
  //    150,   // READER_CYCLE_LOW_DURATION   ms
  //    5000,  // READER_CYCLE_HIGH_DURATION  ms
  //    6,     // READER_POWER_CONTROL_PIN    ms
  //
  //    // TODO: Temp for testing only.
  //    // Ultimately this should have its own eeprom space
  //    // and not be part of Settings.
  //    EEPROM.read(0)      // proximity_state             boolean (0 or 1)
  //
  //    // Led class constants
  //    //  10,    //
  //
  //    // SerlialMenu class constants
  //    //  16,    //
  //    //  8,     //
  //    //  20,    //
  //    //  5      //
  //  };

  // a reference (alias?) from S to Settings
  extern Storage& S = Settings;

  //  extern const int TAG_LAST_READ_TIMEOUT = S.TAG_LAST_READ_TIMEOUT;
  //  extern const int TAG_READ_SLEEP_INTERVAL = S.TAG_READ_SLEEP_INTERVAL;
  //  extern const int READER_CYCLE_LOW_DURATION = S.READER_CYCLE_LOW_DURATION;
  //  extern const int READER_CYCLE_HIGH_DURATION = S.READER_CYCLE_HIGH_DURATION;
  //  extern const int READER_POWER_CONTROL_PIN = S.READER_POWER_CONTROL_PIN;

  
