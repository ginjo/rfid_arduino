/*
  A global settings file, intended to facilitate storing persistant
  settings in EEPROM.
  
  This also demonstrates how to do globally accessible vars and functions.
  
*/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

  #define VERSION "v0.1.0.pre195"
  #define TIMESTAMP __DATE__ ", " __TIME__

  #include <Arduino.h>
  #include <SoftwareSerial.h>
  #include <string.h>

  #include "global.h"
  #include "logger.h"

  // See logger.h for master debug controls.
  #ifdef ST_DEBUG
    #define ST_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define ST_LOG(...)
  #endif
  
  #include "storage.h"
    
  #define SETTINGS_SIZE 15 // quantity of settings vars
  #define SETTINGS_NAME_SIZE 32 // max length of a setting var name
  #define SETTINGS_VALUE_SIZE 16 // max length of a setting var
  //#define SETTINGS_EEPROM_ADDRESS 800 // see storage.h, see settings class definition

  // Instanciates the built-in reset function.
  // WARN: This causes multiple-definition errors if run here. See menu.cpp.
  //void(* resetFunc) (void) = 0;


  class Settings : public Storage<Settings> {
  public:

    /***  Static  ***/
    
    static Settings Current;

    static Settings* Load(Settings *settings_obj = &Current, int _eeprom_address = SETTINGS_EEPROM_ADDRESS);
    
    static bool Failsafe();
    

    /***  Instance  ***/

    char settings_name[SETTINGS_NAME_SIZE];
    
    uint32_t tag_last_read_timeout;
    uint32_t tag_read_sleep_interval; // ms
    uint32_t reader_cycle_low_duration; // ms
    uint32_t reader_cycle_high_duration;

    uint32_t admin_timeout;
    int proximity_state_startup;
    int enable_debug;

    //char default_reader[SETTINGS_VALUE_SIZE];
    uint8_t default_reader;

    long hw_serial_baud;
    long bt_baud;
    long rfid_baud;

    int tone_frequency;
    int admin_startup_timeout;
    bool log_to_bt; // logging to BTserial: 0=No, 1=Yes (but only if bt is connected)
    uint8_t log_level;

    /*  Constructors  */

    Settings();


    /*  Functions  */
    
    bool updateSetting(int, char[]);
    void getSettingByIndex(int, char*, char*);
    void displaySetting(int, char*);
    void printSettings(Stream*);
    int  debugMode();
    int  save();
    
  };  // class Settings


  /***  Global / External Vars & Funcs  ***/

  // This declares an extern 'S' of type Settings.
  // This will be a ref to Settings::Current
  // and is intended as a convenience shortcut, since
  // Settings::Current is referred to so many times in the project.
  extern Settings& S;


  // Creates an extern constant 2D char array 'SETTING_NAMES' that holds Settings var names.
  // This is used to get names/values by index and to iterate through Settings
  // for UI purposes (displaying lists, displaying individual settings).
  // Note that this array is 0-based, unlike some other Menu lists, which are 1-based.
  //
  //  There are now 5 places where settings need to be managed:
  //  1. Declaration in settings.h
  //  2. Initialization in settings.cpp
  //  3. Retrieving by index in settings.cpp
  //  4. Setting by index in settings.cpp
  //  5. Names index in settings.h (for storage of strings in PROGMEM).
  //
  // TODO: Should SETTING_NAMES[] be converted to static Settings variable?
  //
  namespace {  // a nameless namespace helps build a 2D array of char strings in PROGMEM.
    // See here for why the 'namespace' makes this work. Otherwise we get compilation errors.
    // https://stackoverflow.com/questions/2727582/multiple-definition-in-header-file
    const static char str_0[] PROGMEM = "tag_last_read_timeout"; // "String 0" etc are strings to store - change to suit.
    const static char str_1[] PROGMEM = "tag_read_sleep_interval";
    const static char str_2[] PROGMEM = "reader_cycle_low_duration";
    const static char str_3[] PROGMEM =  "reader_cycle_high_duration";
    const static char str_4[] PROGMEM =  "admin_timeout";
    const static char str_5[] PROGMEM =  "proximity_state_startup";
    const static char str_6[] PROGMEM =  "enable_debug";
    const static char str_7[] PROGMEM =  "default_reader";
    const static char str_8[] PROGMEM =  "hw_serial_baud";
    const static char str_9[] PROGMEM =  "bt_baud";
    const static char str_10[] PROGMEM =  "rfid_baud";
    const static char str_11[] PROGMEM =  "tone_frequency";
    const static char str_12[] PROGMEM =  "admin_startup_timeout";
    const static char str_13[] PROGMEM =  "log_to_bt";
    const static char str_14[] PROGMEM =  "log_level";
    extern const char *const SETTING_NAMES[] PROGMEM = {
      str_0,
      str_1,
      str_2,
      str_3,
      str_4,
      str_5,
      str_6,
      str_7,
      str_8,
      str_9,
      str_10,
      str_11,
      str_12,
      str_13,
      str_14
    };
  } // end nameless namespace
    
#endif  // end __SETTINGS_H__

  
