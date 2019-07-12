#include "settings.h"
// This is moved here by suggestion to stop EEPROM warnings.
#include <EEPROM.h>
  
  Settings::Settings() :
    Storage("settings"),
  
    // See for explanation: https://stackoverflow.com/questions/7405740/how-can-i-initialize-base-class-member-variables-in-derived-class-constructor
    settings_name("default_settings"),
  
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
    //storage_name = "settings";
    
    // ONLY use this for debugging.
    // Always comment this out for production.
    proximity_state = 1;
  }


  // TODO: I think this ultimately needs to be integrated into Storage class EEPROM handling.
  int Settings::updateProximityState(int _state) {
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
  bool Settings::updateSetting(int _index, char _data[]) {

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
        //strcpy(DEFAULT_READER, (char *)_data);
        strncpy(DEFAULT_READER, (char *)_data, sizeof(DEFAULT_READER));
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
      case 17:
        OUTPUT_SWITCH_PIN = (int)strtol(_data, NULL, 10);
        break;
      default :
        return false;
    }

    strcpy(settings_name, "custom_settings");
    save();
    
    return true;
  }

  void Settings::getSettingByIndex (int index, char *setting_name, char *setting_value) {
    strcpy_P(setting_name, (char *)pgm_read_word(&(SETTING_NAMES[index-1])));
    DPRINT(F("Settings::getSettingByIndex: ")); DPRINT(index); DPRINT(", "); DPRINT(setting_name);
    
    switch(index) {
      case 1 :
        sprintf(setting_value, "%lu", TAG_LAST_READ_TIMEOUT);
        break;
      case 2 :
        sprintf(setting_value, "%lu", TAG_READ_SLEEP_INTERVAL);
        break;
      case 3 :
        sprintf(setting_value, "%lu", READER_CYCLE_LOW_DURATION);
        break;
      case 4 :
        sprintf(setting_value, "%lu", READER_CYCLE_HIGH_DURATION);
        break;
      case 5 :
        sprintf(setting_value, "%u", READER_POWER_CONTROL_PIN);
        break;
      case 6 :
        sprintf(setting_value, "%lu", admin_timeout);
        break;
      case 7 :
        sprintf(setting_value, "%i", proximity_state);
        break;
      case 8 :
        sprintf(setting_value, "%i", enable_debug);
        break;
      case 9 :
        sprintf(setting_value, "%s", DEFAULT_READER);
        break;
      case 10 :
        sprintf(setting_value, "%i", LED_PIN);
        break;
      case 11 :
        sprintf(setting_value, "%s", (char *)BT_RXTX);
        break;
      case 12 :
        sprintf(setting_value, "%i", RFID_SERIAL_RX);
        break;
      case 13 :
        sprintf(setting_value, "%li", HW_SERIAL_BAUD);
        break;
      case 14 :
        sprintf(setting_value, "%i", DEBUG_PIN);
        break;
      case 15 :
        sprintf(setting_value, "%li", BT_BAUD);
        break;
      case 16 :
        sprintf(setting_value, "%li", RFID_BAUD);
        break;
      case 17 :
        sprintf(setting_value, "%i", OUTPUT_SWITCH_PIN);
        break;
        
      default:
        break;
    } // switch

    DPRINT(", "); DPRINTLN(setting_value);
  } // function

  // Prints out one line of settings. Can use variable int '*' in format string here,
  // since it works in onlinegdb.com. See my getSettingByIndex.cpp example.
  // Iterate over number-of-settings with this to get entire settings list.
  // Pass this an initialized output string to return via:
  //   char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
  void Settings::displaySetting(int index, char *output) {
    char setting_name[SETTINGS_NAME_SIZE], setting_value[SETTINGS_VALUE_SIZE];
    getSettingByIndex(index, setting_name, setting_value);
    sprintf(output, "%2i. %-32s %s", index, setting_name, setting_value);
    DPRINT(F("Settings::displaySetting() gathered: "));
    DPRINT(setting_name); DPRINT(", "); DPRINTLN(setting_value);
    DPRINT(F("Settings::displaySetting() returning: "));
    DPRINTLN(output);
  }

  int Settings::debugMode() {
    if (digitalRead(DEBUG_PIN) == LOW || enable_debug == 1) {
      return 1;
    } else {
      return 0;
    }
  }

  int Settings::save() {
    Serial.println(F("Settings::save() BEGIN"));
    int result = Storage::save(SETTINGS_EEPROM_ADDRESS, STORAGE_CHECKSUM_SIZE);
    Serial.println(F("Settings::save() END"));
    return result;
  }

  

  /***  Static & Extern  ***/

  // This is a settings-specific wrapper for Storage::Load().
  // It handles settings-specific behavior, like saving default
  // settings if checksum mismatch.
  void Settings::Load() {
    Serial.println(F("Settings::Load() BEGIN"));

    // Dunno if we need to qualify 'current' with the class name, but just to be safe.
    Storage::Load(&Settings::current, SETTINGS_EEPROM_ADDRESS, STORAGE_CHECKSUM_SIZE);
    
    uint16_t calculated_checksum = current.calculateChecksum();   

    DPRINT(F("Settings::Load() current.storage_name '"));
    DPRINT((char *)current.storage_name);
    DPRINT(F("' settings_name '"));
    DPRINT((char *)current.settings_name);
    DPRINT(F("' calc chksm 0x"));
    DPRINTLN(calculated_checksum, 16);

    // TEMP: Prints out all settings in tabular format.
    Serial.println("Settings::Load() printing all values in Settings::current");
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
      current.displaySetting(n, output);
      Serial.println(output);
    }
    
    if (GetStoredChecksum() != calculated_checksum) {
      Serial.println(F("Settings::Load() chksm mismatch so creating default Settings()"));
      current = Settings();
      strcpy(current.settings_name, "saved_settings");
      current.save();
      Serial.print(F("Settings::Load() using default settings saved as '"));
      
    } else {
      Serial.print(F("Settings::Load() using loaded settings '"));
      
    }
    
    Serial.print(current.settings_name); Serial.println("'");
    Serial.println(F("Settings::Load() END"));
    
  } // Settings::Load()

  uint16_t Settings::GetStoredChecksum() {
    if (Serial) Serial.println(F("Settings::GetStoredChecksum()"));
    return Storage::GetStoredChecksum(SETTINGS_EEPROM_ADDRESS);
  }
  
  // It seems necessary to initialize static member vars
  // before using them (seems kinda wasteful).
  // Maybe we don't need this if we move .ino items into setup().
  //Settings Settings::current;// = *Settings::Load();
  Settings Settings::current = Settings();

  // a reference (alias?) from S to CurrentSettings
  Settings& S = Settings::current;


  
