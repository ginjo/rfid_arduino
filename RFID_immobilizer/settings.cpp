#include "settings.h"

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
      EEPROM.update(0, proximity_state);
      // and enable this for debugging.
      //state_dev_tmp = proximity_state;
    }
    return proximity_state;
  }

  // Updates a setting given setting index and data.
  // TODO: Figure out how to convert selecte_setting from ascii '7' to int 7,
  // then make sure to store that in selected_setting (in SerialMenu).

  // Using Variadic Arguments:
  //
  //  bool Storage::updateSetting(char _index, ...) {
  //    va_list args;
  //    va_start(args, _index);
  //
  //    Serial.print("S.updateSetting() with index: ");
  //    Serial.println(_index);
  //    
  //    switch (_index) {
  //      case '7':
  //        Serial.print("S.updateSetting() updating 'admin_timeout' with: ");
  //        admin_timeout = va_arg(args, char);
  //        Serial.println(admin_timeout);
  //        break;
  //      case '1':
  //        break;
  //
  //      // case ...
  //      //   break;
  //    }
  //  }


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
        break;

      // case ...
      //   break;
    }

    return false;
  }


  
  
