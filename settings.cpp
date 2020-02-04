  #include "settings.h"
  #include "logger.h"
  #include "reader.h"
  
  // This is moved here by suggestion to stop EEPROM warnings.
  #include <EEPROM.h>

  
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
    proximity_state_startup(2), // 0 = off, 1 = on, 2 = auto (uses last saved state)

    // NOT: enables debugging (separate from using DEBUG macro).
    // Changed to: Sets the debug level if debug-pin is held low.
    enable_debug(4),

    // sets default reader index
    default_reader(3),

    hw_serial_baud(57600),
    bt_baud(57600),
    rfid_baud(9600),
    tone_frequency(2800), /* 2800, 2093, 1259, 1201 */
    admin_startup_timeout(7),
    log_to_bt(false),
    log_level(4)
  {     
    //strlcpy(settings_name, "default-settings", sizeof(settings_name));
    strlcpy(settings_name, "default-settings", SETTINGS_NAME_SIZE);
    //strlcpy(default_reader, "WL-125", sizeof(default_reader));
  }


  // List of all setting names, getters, and setters.
  /*
    This list defines the order (and thus the index) that settings are displayed in.
    See notes in settings.h for further info on Settings structure
  */
  settings_list_T const Settings::SettingsList[SETTINGS_SIZE] PROGMEM = {
    { "admin_startup_timeout", &Settings::display_admin_startup_timeout, &Settings::set_admin_startup_timeout},
    { "admin_timeout", &Settings::display_admin_timeout, &Settings::set_admin_timeout},
    { "proximity_state_startup", &Settings::display_proximity_state_startup, &Settings::set_proximity_state_startup},

    { "log_level", &Settings::display_log_level, &Settings::set_log_level},
    { "enable_debug", &Settings::display_enable_debug, &Settings::set_enable_debug},
    { "log_to_bt", &Settings::display_log_to_bt, &Settings::set_log_to_bt},
    
    { "tag_last_read_timeout", &Settings::display_tag_last_read_timeout, &Settings::set_tag_last_read_timeout},
    { "reader_cycle_high_duration", &Settings::display_reader_cycle_high_duration, &Settings::set_reader_cycle_high_duration},
    { "reader_cycle_low_duration", &Settings::display_reader_cycle_low_duration, &Settings::set_reader_cycle_low_duration},
    { "tag_read_sleep_interval", &Settings::display_tag_read_sleep_interval, &Settings::set_tag_read_sleep_interval},

    { "default_reader", &Settings::display_default_reader, &Settings::set_default_reader},
    { "hw_serial_baud", &Settings::display_hw_serial_baud, &Settings::set_hw_serial_baud},
    { "bt_baud", &Settings::display_bt_baud, &Settings::set_bt_baud},
    { "rfid_baud", &Settings::display_rfid_baud, &Settings::set_rfid_baud},
    { "tone_frequency", &Settings::display_tone_frequency, &Settings::set_tone_frequency},
  };  
  

  // Updates a setting given setting index with data.
  //
  // The passed-in index is 1-based.
  //
  bool Settings::updateSetting(int _index, char _data[]) {

    ST_LOG(5, F("S.updateSetting() "), false);
    ST_LOG(5, _index, false); ST_LOG(5, ", ", false);

    char setting_name[SETTINGS_NAME_SIZE]; // A single setting name.
    // TODO: Is this safe? Is there a strlcpy_P that we can use?
    //strcpy_P(setting_name, (char *)pgm_read_word(&(SETTING_NAMES[_index-1])));

    ST_LOG(5, setting_name, false); ST_LOG(5, ", ", false);
    ST_LOG(5, _data, true);

    if (_index <= SETTINGS_SIZE) {  
          
      // Loads setting object from PROGMEM into ram, where it can be accessed normally.
      settings_list_T setting = {};
      memcpy_P(&setting, &SettingsList[_index-1], sizeof(setting));

      // MAGIC: Sets up and calls pointer-to-member-function on 'this',
      // passing in data to store in field.
      if (setting.setter_fp) {
        ST_LOG(6, F("Calling setting setter_fp"), true);
        getter_setter_T fp = setting.setter_fp;
        (this->*fp)(_data);
      }
      
    }

    // Auto-store Settings object to EEPROM.
    // This was moved to save().
    //strlcpy(settings_name, "custom-settings", SETTINGS_NAME_SIZE); // Name of full Settings object.
    //save();
    
    return true;
  }


  // Populates the passed-in *setting_name and *setting_value with data, per the given index.
  // Note that this does not return any values.
  // 
  // The passed-in index is 1-based.
  //
  // TODO: This is only used in displaySetting(), so maybe it should be rolled into that?
  // 
  void Settings::getSettingByIndex (int index, char *setting_name, char *setting_value) {
    // TODO: Is this safe? Is there a strlcpy_P that we can use?
    //strcpy_P(setting_name, (char *)pgm_read_word(&(SETTING_NAMES[index-1])));

    settings_list_T setting = {};
    memcpy_P(&setting, &SettingsList[index-1], sizeof(setting));
    
    //  strcpy_P(setting_name, (char*)pgm_read_word(&SettingsList[index-1].name));
    //  ST_LOG(5, F("Settings::getSettingByIndex: "), false); ST_LOG(5, index, false); ST_LOG(5, ", ", false); ST_LOG(5, setting_name, false);
    //  Serial.println((char*)pgm_read_word(&SettingsList[index-1].name));

    sprintf(setting_name, "%s", (char*)setting.name); //, sizeof(SETTINGS_NAME_SIZE));
    ST_LOG(5, F("Settings::getSettingByIndex: "), false); ST_LOG(5, index, false); ST_LOG(5, ", ", false); ST_LOG(5, setting_name, false);
    
    if (index <= SETTINGS_SIZE && setting.display_fp) {
      ST_LOG(6, F(", Calling setting display_fp"), false);
      getter_setter_T fp = setting.display_fp;
      (this->*fp)(setting_value);
    }
    
    ST_LOG(5, ", ", false); ST_LOG(5, setting_value, true);
  } // function

  // Prints out one line of settings. Can use variable int '*' in format string here,
  // since it works in onlinegdb.com. See my getSettingByIndex.cpp example.
  // Iterate over number-of-settings with this to get entire settings list.
  // Pass this an initialized output string to return via:
  //   char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
  void Settings::displaySetting(int index, char *output) {
    char setting_name[SETTINGS_NAME_SIZE] = {};
    char setting_value[SETTINGS_VALUE_SIZE] = {};
    
    getSettingByIndex(index, setting_name, setting_value);
    // TODO: Find a way to insert SETTINGS_NAME_SIZE into format string here.
    sprintf(output, "%2i  %-28s %s", index, setting_name, setting_value);
    
    ST_LOG(5, F("Settings::displaySetting() gathered: "), false);
    ST_LOG(5, setting_name, false); ST_LOG(5, ", ", false); ST_LOG(5, setting_value, true);
    
    ST_LOG(5, F("Settings::displaySetting() returning: "), false);
    ST_LOG(5, output, true);
  }

  void Settings::printSettings(Stream *sp) {
    sp->print(F("Settings, chksm 0x"));
    sp->print(S.calculateChecksum(), 16);
    sp->print(F(", size "));
    sp->println(sizeof(S));
    
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE + 8] = {};
      S.displaySetting(n, output);
      sp->println(output);
    }
  }

  // enbable_debug is no longer a boolean.
  // It is the log-level when debug-pin is activated.
  bool Settings::debugMode() {
    //if (digitalRead(DEBUG_PIN) == LOW || TempDebug == true || enable_debug == 1) {
    if (digitalRead(DEBUG_PIN) == LOW || TempDebug == true) {
      return true;
    } else {
      return false;
    }
  }

  //  // This save is to accommodate settings list triggering of save().
  //  // The char arg is only for passing in a flag: '0' bypasses saving, '1' performs save.
  //  void Settings::save(char *dat) {
  //    LOG(5, F("Settings.save called from list with "), false);
  //    LOG(5, (char*)dat, true);
  //    if (dat[0] == '0') return;
  //    LOG(5, "Saving settings from list", true);
  //    save();
  //  }

  // This is the main (and original) save() function.
  int Settings::save() {
    int result = 0;

    // This sets the name of entire Settings object.
    // This used to be in updateSetting().
    strlcpy(settings_name, "custom-settings", SETTINGS_NAME_SIZE);
    
    //Serial.println(F("Settings::save() BEGIN"));
    //int result = Storage::save(SETTINGS_EEPROM_ADDRESS);
    result = Storage::save();
    LOG(5, F("Settings::save() result: ")); LOG(5, result, true);
    //Serial.println(F("Settings::save() END"))
    return result;
  }

  

  /***  Static & Extern  ***/

 
  bool Settings::Failsafe() {
    //return digitalRead(FAILSAFE_PIN) == LOW;
    //Let's try using just a single user-space troubleshooting pin.
    return digitalRead(DEBUG_PIN) == LOW;
  }

  // This is a settings-specific wrapper for Storage::Load().
  // It handles settings-specific behavior, like saving default
  // settings if checksum mismatch.
  //
  // See Tags::Load() for potential refactor solution to decouple this function
  // from the static var Settings::Current().
  //
  // Here's the line from Tags:
  // Tags* Tags::Load(Tags* _tag_set, int _eeprom_address) {
  //
  //   void Settings::Load() {
  //
  Settings* Settings::Load(Settings *settings_obj, int _eeprom_address) {
    LOG(5, F("Settings::Load() BEGIN"), true);

    //uint16_t calculated_checksum = Current.calculateChecksum();
    //Storage::Load(&Current, _eeprom_address);
    if (Failsafe()) {
      LOG(4, F("Failsafe enabled, skipping stored settings"), true);
    } else {
      Storage::Load(settings_obj, _eeprom_address);

      #ifdef ST_DEBUG
        LOG(5, F("Settings::Load() storage_name '"));
        LOG(5, settings_obj->storage_name);
        LOG(5, F("' settings_name '"));
        LOG(5, settings_obj->settings_name);
        LOG(5, F("' chksm 0x"));
        LOG(5, settings_obj->checksum, 16, true);
      #endif
  
      if (!settings_obj->checksumMatch()) {LOG(3, F("Settings::Load() chksm mismatch"), true);}
    }

    // WARN: Can't reliably do ST_PRINT from here, since we don't have a confirmed
    // valid Settings instance yet. Printing settings data before it has been
    // verified can result in UB !!!
    //


    // Handles checksum mismatch by loading default settings.
    // TODO: √ Don't save default settings, let the user do it.
    //if (GetStoredChecksum() != calculated_checksum) {
    if (!settings_obj->checksumMatch() || Failsafe()) {
      LOG(3, F("Settings::Load() chksm mismatch or failsafe"), true);
      /* Two problems here, I think.
       * 1. The new Settings objected pointed to here will go out of scope after this function ends.
       * 2. The settings_obj points to an address of value in Current,
       *    but the address pointed to by settings_obj gets changed to the new Settings value.
       *    
       * So to get the new Settings to stick in the static var Current, what do we do?
       * I think we have to either make Current a pointer, or take settings_obj arg as a reference type var.
       * 
       * Or even better, we just de-reference the pointers and set the original Current var with new value.
       * Old: settings_obj = new Settings();  // doesn't work, repoints settings_obj to new address.
       */
       
      Settings *ss = new Settings();  // Gets new object pointer.
      *settings_obj = *ss;            // Copies new object to settings_obj (same as Current).
      delete ss;                      // Deletes abandoned new object ss.
      
      strlcpy(settings_obj->settings_name, "default-settings", SETTINGS_NAME_SIZE);
      settings_obj->eeprom_address = _eeprom_address;
      /* Disabled saving of default settings (let the user decide instead).
         Re-enable this to automatically save default settings to eeprom.
         Currently the user needs to change/save a default setting
         to get the entire defaults set to save to eeprom. This is good. */
      //if (! Failsafe()) settings_obj->save();
      LOG(4, F("Settings::Load() using default settings '"));
      
    } else {
      LOG(4, F("Settings::Load() using stored settings '"));
    }
    
    LOG(4, settings_obj->settings_name); LOG(4, "'", true);
    #ifdef ST_DEBUG
      LOG(6, F("Settings::Load() END"), true);
    #endif

    return settings_obj;
  } // Settings::Load()


  /*  Getters and Setters  */

  void Settings::display_tag_last_read_timeout(char *out) {sprintf(out, "%lu", tag_last_read_timeout);}
  void Settings::set_tag_last_read_timeout(char *data) {tag_last_read_timeout = (uint32_t)strtol(data, NULL, 10);}

  void Settings::display_tag_read_sleep_interval(char *out) {sprintf(out, "%lu", tag_read_sleep_interval);}
  void Settings::set_tag_read_sleep_interval(char *data) {tag_read_sleep_interval = (uint32_t)strtol(data, NULL, 10);}

  void Settings::display_reader_cycle_low_duration(char *out) {sprintf(out, "%lu", reader_cycle_low_duration);}
  void Settings::set_reader_cycle_low_duration(char *data) {reader_cycle_low_duration = (uint32_t)strtol(data, NULL, 10);}

  void Settings::display_reader_cycle_high_duration(char *out) {sprintf(out, "%lu", reader_cycle_high_duration);}
  void Settings::set_reader_cycle_high_duration(char *data) {reader_cycle_high_duration = (uint32_t)strtol(data, NULL, 10);}

  void Settings::display_admin_timeout(char *out) {sprintf(out, "%lu", admin_timeout);}
  void Settings::set_admin_timeout(char *data) { admin_timeout = (uint32_t)strtol(data, NULL, 10); if (admin_timeout < 10) {admin_timeout = 10;} }

  void Settings::display_proximity_state_startup(char *out) {sprintf(out, "%i", proximity_state_startup);}
  void Settings::set_proximity_state_startup(char *data) {proximity_state_startup = (int)strtol(data, NULL, 10);}

  void Settings::display_enable_debug(char *out) {sprintf(out, "%i", enable_debug);}
  void Settings::set_enable_debug(char *data) {enable_debug = (int)strtol(data, NULL, 10);}

  void Settings::display_default_reader(char *out) {sprintf(out, "%i (%s)", default_reader, Reader::NameFromIndex((int)default_reader));}
  void Settings::set_default_reader(char *data) {default_reader = (uint8_t)strtol(data, NULL, 10);}

  void Settings::display_hw_serial_baud(char *out) {sprintf(out, "%li", hw_serial_baud);}
  void Settings::set_hw_serial_baud(char *data) {hw_serial_baud = (long)strtol(data, NULL, 10);}

  void Settings::display_bt_baud(char *out) {sprintf(out, "%li", bt_baud);}
  void Settings::set_bt_baud(char *data) {bt_baud = (long)strtol(data, NULL, 10);}

  void Settings::display_rfid_baud(char *out) {sprintf(out, "%li", rfid_baud);}
  void Settings::set_rfid_baud(char *data) {rfid_baud = (long)strtol(data, NULL, 10);}

  void Settings::display_tone_frequency(char *out) {sprintf(out, "%i", tone_frequency);}
  void Settings::set_tone_frequency(char *data) {tone_frequency = (int)strtol(data, NULL, 10);}

  void Settings::display_admin_startup_timeout(char *out) {sprintf(out, "%i", admin_startup_timeout);}
  void Settings::set_admin_startup_timeout(char *data) {admin_startup_timeout = (int)strtol(data, NULL, 10);}

  void Settings::display_log_to_bt(char *out) {sprintf(out, "%i", log_to_bt);}
  void Settings::set_log_to_bt(char *data) {log_to_bt = (bool)strtol(data, NULL, 10);}

  void Settings::display_log_level(char *out) {sprintf(out, "%hhu", log_level);}
  void Settings::set_log_level(char *data) {log_level = (uint8_t)strtol(data, NULL, 10);}



  /*  Global & Special Initializers  */

  // Initializes a default settings object as soon as this file loads.
  Settings Settings::Current = Settings();

  // a reference (alias?) from S to CurrentSettings
  Settings& S = Settings::Current;

  
