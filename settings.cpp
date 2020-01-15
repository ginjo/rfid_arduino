  #include "settings.h"
  #include "reader.h"
  // This is moved here by suggestion to stop EEPROM warnings.
  #include <EEPROM.h>

  //bool Settings::TempDebug; // = (bool)digitalRead(DEBUG_PIN); // static
  
  Settings::Settings() :
    Storage("settings", SETTINGS_EEPROM_ADDRESS),
  
    // See for explanation: https://stackoverflow.com/questions/7405740/how-can-i-initialize-base-class-member-variables-in-derived-class-constructor
    // I don't think you can initialize char arrays like this. See below for alternative.
    //settings_name("default-settings"),
  
    // ultimate valid-tag timeout
    tag_last_read_timeout(30), // seconds

    // time between attempts to listen to reader
    tag_read_sleep_interval(1000), // millis

    // off duration during reader power cycle
    reader_cycle_low_duration(150), // millis

    // on duration before reader next power cycle
    // also is duration before 'aging' stage begins
    reader_cycle_high_duration(15), // seconds

    // idle time before admin mode switches to run mode
    // should be greater than reader_cycle_high_duration
    admin_timeout(60), // seconds

    // Sets whether output switches off or on at startup.
    proximity_state_startup(1), // 0 = off, 1 = on, 2 = auto (uses last saved state)

    // enables debugging (separate from using DEBUG macro).
    enable_debug(0),

    // sets default reader index
    default_reader(0),

    hw_serial_baud(57600),
    bt_baud(9600),
    rfid_baud(9600),
    tone_frequency(2800), /* 2800, 2093, 1259, 1201 */
    admin_startup_timeout(7),
    log_to_bt(false)
  {     
    strlcpy(settings_name, "default-settings", sizeof(settings_name));
    //strlcpy(default_reader, "WL-125", sizeof(default_reader));
  }

  

  // Updates a setting given setting index and data.
  bool Settings::updateSetting(int _index, char _data[]) {

    ST_PRINT(F("S.updateSetting() "));
    ST_PRINT(_index); ST_PRINT(", ");

    char setting_name[SETTINGS_NAME_SIZE];
    // TODO: Is this safe? Is there a strlcpy_P that we can use?
    strcpy_P(setting_name, (char *)pgm_read_word(&(SETTING_NAMES[_index-1])));

    ST_PRINT(setting_name); ST_PRINT(", ");
    ST_PRINTLN(_data);

    // Note that this is a 1-based list, not 0-based.
    switch (_index) {
      case 1:
        tag_last_read_timeout = (uint32_t)strtol(_data, NULL, 10);
        break;
      case 2:
        tag_read_sleep_interval = (uint32_t)strtol(_data, NULL, 10);
        break;
      case 3:
        reader_cycle_low_duration = (uint32_t)strtol(_data, NULL, 10);
        break;
      case 4:
        reader_cycle_high_duration = (uint32_t)strtol(_data, NULL, 10);
        break;
      case 5:
        admin_timeout = (uint32_t)strtol(_data, NULL, 10);
        // This setting should never be so low as to prevent admining at startup.
        if (admin_timeout < 10) { admin_timeout = 10; }
        break;
      case 6:
        proximity_state_startup = (int)strtol(_data, NULL, 10);
        break;
      case 7:
        enable_debug = (int)strtol(_data, NULL, 10);
        break;
      case 8:
        //strlcpy(default_reader, (char *)_data, sizeof(default_reader));
        default_reader = (uint8_t)strtol(_data, NULL, 10);
        break;
      case 9:
        hw_serial_baud = (long)strtol(_data, NULL, 10);
        break;
      case 10:
        bt_baud = (long)strtol(_data, NULL, 10);
        break;
      case 11:
        rfid_baud = (long)strtol(_data, NULL, 10);
        break;
      case 12:
        tone_frequency = (int)strtol(_data, NULL, 10);
        break;
      case 13:
        admin_startup_timeout = (int)strtol(_data, NULL, 10);
        break;
      case 14:
        log_to_bt = (bool)strtol(_data, NULL, 10);
        break;
                        
      default :
        return false;
    }

    strlcpy(settings_name, "custom-settings", SETTINGS_NAME_SIZE);
    save();
    
    return true;
  }

  // Populates the passed-in *setting_name and *setting_value with data, per the given index.
  // Note that this does not return any values.
  //
  void Settings::getSettingByIndex (int index, char *setting_name, char *setting_value) {
    // TODO: Is this safe? Is there a strlcpy_P that we can use?
    strcpy_P(setting_name, (char *)pgm_read_word(&(SETTING_NAMES[index-1])));
    ST_PRINT(F("Settings::getSettingByIndex: ")); ST_PRINT(index); ST_PRINT(", "); ST_PRINT(setting_name);
    
    switch(index) {
      case 1 :
        sprintf(setting_value, "%lu", tag_last_read_timeout);
        break;
      case 2 :
        sprintf(setting_value, "%lu", tag_read_sleep_interval);
        break;
      case 3 :
        sprintf(setting_value, "%lu", reader_cycle_low_duration);
        break;
      case 4 :
        sprintf(setting_value, "%lu", reader_cycle_high_duration);
        break;
      case 5 :
        sprintf(setting_value, "%lu", admin_timeout);
        break;
      case 6 :
        sprintf(setting_value, "%i", proximity_state_startup);
        break;
      case 7 :
        sprintf(setting_value, "%i", enable_debug);
        break;
      case 8 :
        //sprintf(setting_value, "%s", default_reader);
        sprintf(setting_value, "%i (%s)", default_reader, Reader::NameFromIndex((int)default_reader));
        break;
      case 9 :
        sprintf(setting_value, "%li", hw_serial_baud);
        break;
      case 10 :
        sprintf(setting_value, "%li", bt_baud);
        break;
      case 11 :
        sprintf(setting_value, "%li", rfid_baud);
        break;
      case 12 :
        sprintf(setting_value, "%i", tone_frequency);
        break;   
      case 13 :
        sprintf(setting_value, "%i", admin_startup_timeout);
        break;  
      case 14 :
        sprintf(setting_value, "%i", log_to_bt);
        //sprintf(setting_value, log_to_bt ? "true" : "false");
        break;
                                
      default:
        break;
    } // switch
    
    ST_PRINT(", "); ST_PRINTLN(setting_value);
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
    ST_PRINT(F("Settings::displaySetting() gathered: "));
    ST_PRINT(setting_name); ST_PRINT(", "); ST_PRINTLN(setting_value);
    ST_PRINT(F("Settings::displaySetting() returning: "));
    ST_PRINTLN(output);
  }

  void Settings::printSettings(Stream *sp) {
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = "";
      S.displaySetting(n, output);
      sp->println(output);
    }
  }

  int Settings::debugMode() {
    if (digitalRead(DEBUG_PIN) == LOW || TempDebug == true || enable_debug == 1) {
      return 1;
    } else {
      return 0;
    }
  }

  int Settings::save() {
    //Serial.println(F("Settings::save() BEGIN"));
    //int result = Storage::save(SETTINGS_EEPROM_ADDRESS);
    int result = Storage::save();
    LOG(F("Settings::save() result: ")); LOG(result, true);
    //Serial.println(F("Settings::save() END"))
    return result;
  }

  

  /***  Static & Extern  ***/

 
  bool Settings::Failsafe() {
    return digitalRead(FAILSAFE_PIN) == 0;
  }

  // This is a settings-specific wrapper for Storage::Load().
  // It handles settings-specific behavior, like saving default
  // settings if checksum mismatch.
  //
  // See Tags::Load() for potential refactor solution to decouple this function
  // from the static var Settings::Current().
  //
  // Here's the line from Tags:
  // Tags* Tags::Load(Tags* tag_set, int _eeprom_address) {
  //
  //   void Settings::Load() {
  //
  Settings* Settings::Load(Settings *settings_obj, int _eeprom_address) {
    #ifdef ST_DEBUG
      LOG(F("Settings::Load() BEGIN"), true);
    #endif

    //uint16_t calculated_checksum = Current.calculateChecksum();
    //Storage::Load(&Current, _eeprom_address);
    if (Failsafe()) {
      LOG(F("Failsafe enabled, skipping stored settings"), true);
    } else {
      Storage::Load(settings_obj, _eeprom_address);
    }

    // WARN: Can't reliably do ST_PRINT from here, since we don't have a confirmed
    // valid Settings instance yet. Printing settings data before it has been
    // verified can result in UB !!!
    //
    #ifdef ST_DEBUG
      LOG(F("Settings::Load() storage_name '"));
      LOG(settings_obj->storage_name);
      LOG(F("' settings_name '"));
      LOG(settings_obj->settings_name);
      LOG(F("' chksm 0x"));
      LOG(settings_obj->checksum, 16, true);
    #endif

    // Handles checksum mismatch by loading default settings.
    // TODO: √ Don't save default settings, let the user do it.
    //if (GetStoredChecksum() != calculated_checksum) {
    if (!settings_obj->checksumMatch() || Failsafe()) {
      LOG(F("Settings::Load() chksm mismatch"), true);
      settings_obj = new Settings();
      strlcpy(settings_obj->settings_name, "default-settings", SETTINGS_NAME_SIZE);
      settings_obj->eeprom_address = _eeprom_address;
      /* Disabled saving of default settings (let the user decide instead).
         Re-enable this to automatically save default settings to eeprom.
         Currently the user needs to change/save a default setting
         to get the entire defaults set to save to eeprom. This is good. */
      //if (! Failsafe()) settings_obj->save();
      LOG(F("Settings::Load() using default settings '"));
      
    } else {
      LOG(F("Settings::Load() using loaded settings '"));
    }
    
    LOG(settings_obj->settings_name); LOG("'", true);
    #ifdef ST_DEBUG
      LOG(F("Settings::Load() END"), true);
    #endif

    return settings_obj;
  } // Settings::Load()


  // It is necessary to initialize static member vars before using them.
  // Disable when not using static version of log_to_bt,
  // and use regular Settings initializer.
  //int Settings::log_to_bt = 0;
 
  Settings Settings::Current = Settings();

  // a reference (alias?) from S to CurrentSettings
  Settings& S = Settings::Current;

  
