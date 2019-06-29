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
    RFID_BAUD(9600),

    OUTPUT_SWITCH_PIN(13)
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

    DPRINT(F("S.updateSetting() "));
    DPRINT(_index); DPRINT(", ");

    char setting_name[SETTINGS_NAME_SIZE];
    strcpy_P(setting_name, (char *)pgm_read_word(&(SETTING_NAMES[_index-1])));

    DPRINT(setting_name); DPRINT(", ");
    DPRINTLN(_data);

    // Note that this is a 1-based list (not 0-based).
    switch (_index) {
      case 1:
        TAG_LAST_READ_TIMEOUT = (uint32_t)strtol(_data, NULL, 10);
        break;
      case 2:
        TAG_READ_SLEEP_INTERVAL = (uint32_t)strtol(_data, NULL, 10);
        break;
      case 3:
        READER_CYCLE_LOW_DURATION = (uint32_t)strtol(_data, NULL, 10);
        break;
      case 4:
        READER_CYCLE_HIGH_DURATION = (uint32_t)strtol(_data, NULL, 10);
        break;
      case 5:
        READER_POWER_CONTROL_PIN = (uint8_t)strtol(_data, NULL, 10);
        break;
      case 6:
        admin_timeout = (uint32_t)strtol(_data, NULL, 10);
        // This setting should never be so low as to prevent admining at startup.
        if (admin_timeout < 10) { admin_timeout = 10; }
        break;
      case 7:
        proximity_state = (int)strtol(_data, NULL, 10);
        break;
      case 8:
        enable_debug = (int)strtol(_data, NULL, 10);
        break;
      case 9:
        strcpy(DEFAULT_READER, (char *)_data);
        break;
      case 10:
        LED_PIN = (int)strtol(_data, NULL, 10);
        break;
      case 11:
        //BT_RXTX = (int)strtol(_data, NULL, 10);
        Serial.println(F("BT_RXTX needs a proper setter for updateSetting()"));
        break;
      case 12:
        RFID_SERIAL_RX = (int)strtol(_data, NULL, 10);
        break;
      case 13:
        HW_SERIAL_BAUD = (long)strtol(_data, NULL, 10);
        break;
      case 14:
        DEBUG_PIN = (int)strtol(_data, NULL, 10);
        break;
      case 15:
        BT_BAUD = (long)strtol(_data, NULL, 10);
        break;
      case 16:
        RFID_BAUD = (long)strtol(_data, NULL, 10);
        break;
      default :
        return false;
    }

    return true;
  }

  // Saves this Storage instance to the correct storage address.
  // Sub-classes, like Settings, should carry the info about
  // what address to use.
  void Storage::save() {
    DPRINT(F("Storage::save() using EEPROM.put() with object name: ")); DPRINTLN(storage_name);
    //EEPROM.put(100, this);
  }

  void Storage::getSettingByIndex (int index, char _result[2][SETTINGS_NAME_SIZE]) {
    strcpy_P(_result[0], (char *)pgm_read_word(&(SETTING_NAMES[index-1])));
    DPRINT(F("Storage::getSettingByIndex: ")); DPRINT(index); DPRINT(", "); DPRINTLN(_result[0]);
    
    switch(index) {
      case 1 :
        sprintf(_result[1], "%lu", TAG_LAST_READ_TIMEOUT);
        break;
      case 2 :
        sprintf(_result[1], "%lu", TAG_READ_SLEEP_INTERVAL);
        break;
      case 3 :
        sprintf(_result[1], "%lu", READER_CYCLE_LOW_DURATION);
        break;
      case 4 :
        sprintf(_result[1], "%lu", READER_CYCLE_HIGH_DURATION);
        break;
      case 5 :
        sprintf(_result[1], "%u", READER_POWER_CONTROL_PIN);
        break;
      case 6 :
        sprintf(_result[1], "%lu", admin_timeout);
        break;
      case 7 :
        sprintf(_result[1], "%i", proximity_state);
        break;
      case 8 :
        sprintf(_result[1], "%i", enable_debug);
        break;
      case 9 :
        sprintf(_result[1], "%s", DEFAULT_READER);
        break;
      case 10 :
        sprintf(_result[1], "%i", LED_PIN);
        break;
      case 11 :
        sprintf(_result[1], "%s", (char *)BT_RXTX);
        break;
      case 12 :
        sprintf(_result[1], "%i", RFID_SERIAL_RX);
        break;
      case 13 :
        sprintf(_result[1], "%li", HW_SERIAL_BAUD);
        break;
      case 14 :
        sprintf(_result[1], "%i", DEBUG_PIN);
        break;
      case 15 :
        sprintf(_result[1], "%li", BT_BAUD);
        break;
      case 16 :
        sprintf(_result[1], "%li", RFID_BAUD);
        break;

      default:
        break;
    }
  }


  Storage loadStorage(const char _name[]) {
    Storage result;

    DPRINT("loadStorage() for: "); DPRINTLN(_name);
    
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


  
