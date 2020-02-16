  #include "settings.h"
  #include "logger.h"
  #include "reader.h"
  
  // This is moved here by suggestion to stop EEPROM warnings.
  #include <EEPROM.h>



  /***  Static & Extern  ***/


  /*
    List of all setting names, getters, and setters.
    
    This list defines the order (and thus the index) that settings are displayed in.
    See notes in settings.h for further info on Settings structure
  */
  settings_list_T const Settings::SettingsList[SETTINGS_SIZE] PROGMEM = {
    { "Startup state", &Settings::display_proximity_state_startup, &Settings::set_proximity_state_startup},
    { "Startup timeout", &Settings::display_admin_startup_timeout, &Settings::set_admin_startup_timeout},
    { "Admin timeout", &Settings::display_admin_timeout, &Settings::set_admin_timeout},
    { "Soft timeout", &Settings::display_tag_last_read_soft_timeout, &Settings::set_tag_last_read_soft_timeout},
    { "Hard timeout", &Settings::display_tag_last_read_hard_timeout, &Settings::set_tag_last_read_hard_timeout},

    { "Reader", &Settings::display_default_reader, &Settings::set_default_reader},
    { "Reader cycle max", &Settings::display_reader_cycle_high_max, &Settings::set_reader_cycle_high_max},
    { "Reader cycle low (ms)", &Settings::display_reader_cycle_low_duration, &Settings::set_reader_cycle_low_duration},
    { "Tag read sleep (ms)", &Settings::display_tag_read_sleep_interval, &Settings::set_tag_read_sleep_interval},

    { "Log level", &Settings::display_log_level, &Settings::set_log_level},
    { "Log to BT", &Settings::display_log_to_bt, &Settings::set_log_to_bt},
    { "Debug log level", &Settings::display_debug_level, &Settings::set_debug_level},

    { "HW serial baud", &Settings::display_hw_serial_baud, &Settings::set_hw_serial_baud},
    { "BT serial baud", &Settings::display_bt_baud, &Settings::set_bt_baud},
    { "Reader serial baud", &Settings::display_rfid_baud, &Settings::set_rfid_baud},
    { "Tone freq", &Settings::display_tone_frequency, &Settings::set_tone_frequency},
    { "Admin password", &Settings::display_admin_password, &Settings::set_admin_password}
  };  

  /*  Constructor  */
  
  Settings::Settings() :
    Storage("settings", SETTINGS_EEPROM_ADDRESS),
  
    // See for explanation: https://stackoverflow.com/questions/7405740/how-can-i-initialize-base-class-member-variables-in-derived-class-constructor
    // I don't think you can initialize char arrays like this. See below for alternative.
    //settings_name("default-settings"),
  
    // ultimate valid-tag timeout
    tag_last_read_hard_timeout(30), // seconds

    // time between attempts to listen to reader
    tag_read_sleep_interval(1000), // millis

    // off duration during reader power cycle
    reader_cycle_low_duration(150), // millis

    // max on duration of reader power cycle
    reader_cycle_high_max(15),

    // on duration before reader next power cycle
    // also is duration before 'aging' stage begins
    tag_last_read_soft_timeout(15), // seconds

    // idle time before admin mode switches to run mode
    // should be greater than tag_last_read_soft_timeout
    admin_timeout(60), // seconds

    // Sets whether output switches off or on at startup.
    proximity_state_startup(2), // 0 = off, 1 = on, 2 = auto (uses last saved state)

    // NOT: enables debugging (separate from using DEBUG macro).
    // Changed to: Overrides log_level if debug-pin is held low or debugMode() is true.
    debug_level(5),

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
    strlcpy(settings_name, "default-settings", SETTINGS_NAME_SIZE);
    strlcpy(admin_password, "12345", ADMIN_PASSWORD_SIZE);
  }

 
  bool Settings::Failsafe() {
    //return digitalRead(FAILSAFE_PIN) == LOW;
    //Let's try using just a single user-space troubleshooting pin.
    return digitalRead(DEBUG_PIN) == LOW;
  }

  // This is a settings-specific wrapper for Storage::Load().
  // It handles settings-specific behavior.
  //
  // See Tags::Load() for potential refactor solution to decouple this function
  // from the static var Settings::Current().
  //
  // Here's the line from Tags:
  // Tags* Tags::Load(Tags* _tag_set, int _eeprom_address) {
  //
  // Is the above still relevant?
  //
  Settings* Settings::Load(Settings *settings_obj, int _eeprom_address) {
    LOG(5, F("Stng Load() BEGIN"), true);
    bool rslt;

    if (Failsafe()) {
      LOG(3, F("Failsafe enabled"), true);
      rslt = 0;
    } else {
      rslt = Storage::Load(settings_obj, _eeprom_address);

      #ifdef ST_DEBUG
        LOG(5, F("Stng Load() storage_name '"));
        LOG(5, settings_obj->storage_name);
        LOG(5, F("' settings_name '"));
        LOG(5, settings_obj->settings_name);
        LOG(5, F("' stored chksm 0x"));
        LOG(5, settings_obj->checksum, 16, true);
      #endif
  
      if (! rslt) {
        LOG(3, F("Stng Load() chksm mismatch"), true);
      }
    }

    if (! rslt || Failsafe()) {
      LOG(4, F("Using default settings '"));
    } else {
      LOG(4, F("Using stored settings '"));
    }
    
    LOG(4, settings_obj->settings_name); LOG(4, "'", true);
    #ifdef ST_DEBUG
      LOG(6, F("Stng Load() END"), true);
    #endif

    return settings_obj;
  } // Settings::Load()



  /***  Instance  ***/

  // Updates a setting given setting index with data.
  //
  // The passed-in index is 1-based.
  //
  bool Settings::updateSetting(int _index, char _data[]) {

    ST_LOG(5, F("S.updateSetting() "), false);
    ST_LOG(5, _index, false); ST_LOG(5, ", ", false);

    //char setting_name[SETTINGS_NAME_SIZE]; // A single setting name.
    // TODO: Is this safe? Is there a strlcpy_P that we can use?
    //strcpy_P(setting_name, (char *)pgm_read_word(&(SETTING_NAMES[_index-1])));
    //ST_LOG(5, setting_name, false); ST_LOG(5, ", ", false);
    //ST_LOG(5, _data, true);

    if (_index <= SETTINGS_SIZE) {  
          
      // Loads setting object from PROGMEM into ram, where it can be accessed normally.
      settings_list_T setting = {};
      memcpy_P(&setting, &SettingsList[_index-1], sizeof(setting));

      char setting_name[SETTINGS_NAME_SIZE] = {}; // A single setting name.
      strcpy(setting_name, setting.name);
      ST_LOG(5, setting_name, false); ST_LOG(5, ", ", false);
      ST_LOG(5, _data, true);
      
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
    ST_LOG(6, F("Stng getSettingByIndex: "), false); ST_LOG(6, index, false); ST_LOG(5, ", ", false); ST_LOG(6, setting_name, false);
    
    if (index <= SETTINGS_SIZE && setting.display_fp) {
      ST_LOG(6, F(", Calling setting display_fp"), false);
      getter_setter_T fp = setting.display_fp;
      (this->*fp)(setting_value);
    }
    
    ST_LOG(6, ", ", false); ST_LOG(6, setting_value, true);
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
    sprintf_P(output, PSTR("%2i  %-28s %s"), index, setting_name, setting_value);
    
    ST_LOG(6, F("Stng displaySetting() gathered: "), false);
    ST_LOG(6, setting_name, false); ST_LOG(6, ", ", false); ST_LOG(6, setting_value, true);
    
    ST_LOG(6, F("Stng displaySetting() returning: "), false);
    ST_LOG(6, output, true);
  }

  void Settings::printSettings(Stream *sp) {
    sp->print(F("Settings, chksm 0x"));
    sp->print(S.calculateChecksum(), 16);
    sp->print(F(", size "));
    sp->println(sizeof(S));
    
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE + 8] = {}; // Don't forget null-terminator.
      S.displaySetting(n, output);
      sp->println(output);
    }
  }

  // enbable_debug is no longer a boolean.
  // It is the log-level when debug-pin is activated.
  // TODO: This should be moved to Logger.
  bool Settings::debugMode() {
    //if (digitalRead(DEBUG_PIN) == LOW || TempDebug == true || enable_debug == 1) {
    if (digitalRead(DEBUG_PIN) == LOW || TempDebug == true) {
      return true;
    } else {
      return false;
    }
  }


  // This is the main (and original) save() function.
  int Settings::save() {
    int result = 0;

    // This sets the name of entire Settings object.
    // This used to be in updateSetting().
    strlcpy(settings_name, "custom-settings", SETTINGS_NAME_SIZE);
    
    //Serial.println(F("Settings::save() BEGIN"));
    //int result = Storage::save(SETTINGS_EEPROM_ADDRESS);
    result = Storage::save();
    LOG(5, F("Stng save() result: ")); LOG(5, result, true);
    //Serial.println(F("Settings::save() END"))
    return result;
  }

  
  /*  Getters and Setters  */

  // TODO: Convert all strings to PSTR() (and sprintf to sprintf_P).

  void Settings::display_tag_last_read_hard_timeout(char *out) {sprintf_P(out, PSTR("%lu"), tag_last_read_hard_timeout);}
  void Settings::set_tag_last_read_hard_timeout(char *data) {tag_last_read_hard_timeout = (uint32_t)strtol(data, NULL, 10);}

  void Settings::display_tag_read_sleep_interval(char *out) {sprintf_P(out, PSTR("%lu"), tag_read_sleep_interval);}
  void Settings::set_tag_read_sleep_interval(char *data) {tag_read_sleep_interval = (uint32_t)strtol(data, NULL, 10);}

  void Settings::display_reader_cycle_low_duration(char *out) {sprintf_P(out, PSTR("%lu"), reader_cycle_low_duration);}
  void Settings::set_reader_cycle_low_duration(char *data) {reader_cycle_low_duration = (uint32_t)strtol(data, NULL, 10);}

  void Settings::display_tag_last_read_soft_timeout(char *out) {sprintf_P(out, PSTR("%lu"), tag_last_read_soft_timeout);}
  void Settings::set_tag_last_read_soft_timeout(char *data) {tag_last_read_soft_timeout = (uint32_t)strtol(data, NULL, 10);}

  void Settings::display_admin_timeout(char *out) {sprintf_P(out, PSTR("%lu"), admin_timeout);}
  void Settings::set_admin_timeout(char *data) { admin_timeout = (uint32_t)strtol(data, NULL, 10); if (admin_timeout < 10) {admin_timeout = 10;} }

  void Settings::display_proximity_state_startup(char *out) {sprintf_P(out, PSTR("%i"), proximity_state_startup);}
  void Settings::set_proximity_state_startup(char *data) {proximity_state_startup = (int)strtol(data, NULL, 10);}

  void Settings::display_debug_level(char *out) {sprintf_P(out, PSTR("%u"), debug_level);}
  void Settings::set_debug_level(char *data) {debug_level = (uint8_t)strtol(data, NULL, 10);}

  void Settings::display_default_reader(char *out) {sprintf_P(out, PSTR("%i (%s)"), default_reader, Reader::NameFromIndex((int)default_reader));}
  void Settings::set_default_reader(char *data) {default_reader = (uint8_t)strtol(data, NULL, 10);}

  void Settings::display_hw_serial_baud(char *out) {sprintf_P(out, PSTR("%li"), hw_serial_baud);}
  void Settings::set_hw_serial_baud(char *data) {hw_serial_baud = (long)strtol(data, NULL, 10);}

  void Settings::display_bt_baud(char *out) {sprintf_P(out, PSTR("%li"), bt_baud);}
  void Settings::set_bt_baud(char *data) {bt_baud = (long)strtol(data, NULL, 10);}

  void Settings::display_rfid_baud(char *out) {sprintf_P(out, PSTR("%li"), rfid_baud);}
  void Settings::set_rfid_baud(char *data) {rfid_baud = (long)strtol(data, NULL, 10);}

  void Settings::display_tone_frequency(char *out) {sprintf_P(out, PSTR("%i"), tone_frequency);}
  void Settings::set_tone_frequency(char *data) {tone_frequency = (int)strtol(data, NULL, 10);}

  void Settings::display_admin_startup_timeout(char *out) {sprintf_P(out, PSTR("%i"), admin_startup_timeout);}
  void Settings::set_admin_startup_timeout(char *data) {admin_startup_timeout = (int)strtol(data, NULL, 10);}

  void Settings::display_log_to_bt(char *out) {sprintf_P(out, PSTR("%i"), log_to_bt);}
  void Settings::set_log_to_bt(char *data) {log_to_bt = (bool)strtol(data, NULL, 10);}

  void Settings::display_log_level(char *out) {sprintf_P(out, PSTR("%hhu"), log_level);}
  void Settings::set_log_level(char *data) {log_level = (uint8_t)strtol(data, NULL, 10);}

  void Settings::display_reader_cycle_high_max(char *out) {sprintf_P(out, PSTR("%lu"), reader_cycle_high_max);}
  void Settings::set_reader_cycle_high_max(char *data) {reader_cycle_high_max = (uint8_t)strtol(data, NULL, 10);}

  void Settings::display_admin_password(char *out) {sprintf_P(out, PSTR("*****"));}
  void Settings::set_admin_password(char *data) {
    // NOTE: This will chomp the trailing return (good), since strlen doesn't count the null terminator.
    size_t len = strlen(data);
    if (len > ADMIN_PASSWORD_SIZE) return; // TODO: Make return boolean here.
    strlcpy(admin_password, data, len);
  }

  /*  Global & Special Initializers  */

  // Initializes a default settings object as soon as this file loads.
  Settings Settings::Current = Settings();

  // a reference (alias?) from S to CurrentSettings
  Settings& S = Settings::Current;

  
