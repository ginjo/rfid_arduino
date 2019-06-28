#include "settings.h"

  // TODO: I think Storage should be generic storage class,
  // and Settings, State, whatever should be subclassed.

  // TODO: Implement actual EEPROM storage.
  
  Storage::Storage() :

    // This should be a constant per each sublcass of Storage
    storage_name("settings"),
  
    // ultimate valid-tag timeout
    TAG_LAST_READ_TIMEOUT(15), // seconds

    // time between attempts to listen to reader
    TAG_READ_SLEEP_INTERVAL(1000), // millis

    // off duration during reader power cycle
    READER_CYCLE_LOW_DURATION(150), // millis

    // on duration before reader next power cycle
    // also is duration before 'aging' stage begins
    READER_CYCLE_HIGH_DURATION(5), // seconds

    // controls reader power thru mosfet
    READER_POWER_CONTROL_PIN(5),

    // idle time before admin mode switches to run mode
    // should be greater than READER_CYCLE_HIGH_DURATION
    admin_timeout(60), // seconds
    
    // saved proximity state (TODO: should be separate setting)
    proximity_state(EEPROM.read(0)), // 0 = false, 1 = true

    // enables debug (if #define DEBUG was active at compile time).
    enable_debug(0),

    DEFAULT_READER("WL-125"),
    //DEFAULT_READER("R7941E")

    state_dev_tmp(1),

    LED_PIN(8),
    BT_RXTX {2,3},
    RFID_SERIAL_RX(4),
    HW_SERIAL_BAUD(57600),
    DEBUG_PIN(11),
    BT_BAUD(9600),
    RFID_BAUD(9600)


  { 
    // ONLY use this for debugging.
    proximity_state = 1;
  }

  // TODO: I think this ultimately needs to be integrated into Storage class EEPROM handling.
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

    DPRINT(F("S.updateSetting() called with index: "));
    DPRINTLN(_index);
    
    switch (_index) {
      case 7:
        Serial.print(F("S.updateSetting() updating 'admin_timeout' with: "));
        admin_timeout = strtol(_data, NULL, 10);
        // This setting should never be so low as to prevent admining at startup.
        if (admin_timeout < 10) { admin_timeout = 10; }
        Serial.println(admin_timeout);
        return true;
        break;
      case 1:
        Serial.print(F("S.updateSetting() updating 'TAG_LAST_READ_TIMEOUT' with: "));
        TAG_LAST_READ_TIMEOUT = strtol(_data, NULL, 10);
        Serial.println(TAG_LAST_READ_TIMEOUT);
        return true;
        break;
      case 4:
        Serial.print(F("S.updateSetting() updating 'READER_CYCLE_HIGH_DURATION' with: "));
        READER_CYCLE_HIGH_DURATION = (int)strtol(_data, NULL, 10);
        Serial.println(READER_CYCLE_HIGH_DURATION);
        return true;
        break;
      case 8:
        Serial.print(F("S.updateSetting() updating 'enable_debug' with: "));
        enable_debug = strtol(_data, NULL, 10);
        Serial.println(enable_debug);
        return true;
        break;
      //  case 9:
      //    Serial.print(F("S.updateSetting() updating 'RAW_TAG_LENGTH' with: "));
      //    RAW_TAG_LENGTH = strtol(_data, NULL, 10);
      //    Serial.println(RAW_TAG_LENGTH);
      //    return true;
      //    break;
    }

    return false;
  }

  // Saves this Storage instance to the correct storage address.
  // Sub-classes, like Settings, should carry the info about
  // what address to use.
  void Storage::save() {
    DPRINT(F("Storage::save() using EEPROM.put() with object name: ")); DPRINTLN(storage_name);
    //EEPROM.put(100, this);
  }

  void Storage::getSettingByIndex (int index, char _result[2][SETTINGS_NAME_SIZE]) {
    switch(index) {
      case 0 :
        sprintf(_result[0], "%s", F("TAG_LAST_READ_TIMEOUT"));
        sprintf(_result[1], "%lu", TAG_LAST_READ_TIMEOUT);
        break;
      case 1 :
        sprintf(_result[0], "%s", F("TAG_READ_SLEEP_INTERVAL"));
        sprintf(_result[1], "%lu", TAG_READ_SLEEP_INTERVAL);
        break;
      case 2 :
        sprintf(_result[0], "%s", F("READER_CYCLE_LOW_DURATION"));
        sprintf(_result[1], "%lu", READER_CYCLE_LOW_DURATION);
        break;
      case 3 :
        sprintf(_result[0], "%s", F("READER_CYCLE_HIGH_DURATION"));
        sprintf(_result[1], "%lu", READER_CYCLE_HIGH_DURATION);
        break;
      case 4 :
        sprintf(_result[0], "%s", F("READER_POWER_CONTROL_PIN"));
        sprintf(_result[1], "%u", READER_POWER_CONTROL_PIN);
        break;
      case 5 :
        sprintf(_result[0], "%s", F("admin_timeout"));
        sprintf(_result[1], "%lu", admin_timeout);
        break;
      case 6 :
        sprintf(_result[0], "%s", F("proximity_state"));
        sprintf(_result[1], "%i", proximity_state);
        break;
      case 7 :
        sprintf(_result[0], "%s", F("enable_debug"));
        sprintf(_result[1], "%i", enable_debug);
        break;
      case 8 :
        sprintf(_result[0], "%s", F("DEFAULT_READER"));
        sprintf(_result[1], "%s", DEFAULT_READER);
        break;
      case 9 :
        sprintf(_result[0], "%s", F("LED_PIN"));
        sprintf(_result[1], "%i", LED_PIN);
        break;
      case 10 :
        sprintf(_result[0], "%s", F("BT_RXTX"));
        sprintf(_result[1], "%i", BT_RXTX);
        break;
      case 11 :
        sprintf(_result[0], "%s", F("RFID_SERIAL_RX"));
        sprintf(_result[1], "%i", RFID_SERIAL_RX);
        break;
      case 12 :
        sprintf(_result[0], "%s", F("HW_SERIAL_BAUD"));
        sprintf(_result[1], "%li", HW_SERIAL_BAUD);
        break;
      case 13 :
        sprintf(_result[0], "%s", F("DEBUG_PIN"));
        sprintf(_result[1], "%i", DEBUG_PIN);
        break;
      case 14 :
        sprintf(_result[0], "%s", F("BT_BAUD"));
        sprintf(_result[1], "%li", BT_BAUD);
        break;
      case 15 :
        sprintf(_result[0], "%s", F("RFID_BAUD"));
        sprintf(_result[1], "%li", RFID_BAUD);
        break;

      default:
        break;
    }
  }


  Storage loadStorage(const char _name[]) {
    Storage result;
    // if _name == blabla
       EEPROM.get(100, result);
    // else if blabla
    //   return EEPROM.get(something);
    // endif

    // TODO: Use a different test to validate it's a real Storage object.
    if (strcmp(result.storage_name, "settings") == 0) {
      DPRINT(F("loadStorage() loaded: ")); DPRINTLN(result.storage_name);
      return result;
    } else {
      Storage result;
      DPRINTLN(F("loadStorage() saving new settings object"));
      result.save();
      return result;
    }
  }

  // TODO: This is temp for testing.
  // The data should ultimately be pulled from EEPROM.
  // Defaults should be in constructor, if that works.
  Storage Settings = loadStorage("settings");

  // a reference (alias?) from S to Settings
  Storage& S = Settings;
