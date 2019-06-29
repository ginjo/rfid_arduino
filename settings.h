/*
  A global settings file, intended to facilitate storing persistant
  settings in EEPROM.
  
  This also demonstrates how to do globally accessible vars and functions.
  
  TODO: Consider the Storage class as an abstract base,
        with Settings, Tags, and State as derived classes.
  TODO: SEE https://community.particle.io/t/eeprom-put-using-a-multi-variable-struct/30561/7
        for an easier way to use EEPROM: put()'ing a struct object.
*/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

  #define VERSION "0.1.0.pre84"
  #define TIMESTAMP __DATE__ ", " __TIME__



  // This stuff should probably in a debug.h file.
  
  #define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
  #ifdef DEBUG    //Macros are usually in all capital letters.
    #define DPRINT(...)    if(Settings.enable_debug){Serial.print(__VA_ARGS__);} // BTserial.print(__VA_ARGS__);}    //DPRINT is a macro, debug print
    #define DPRINTLN(...)  if(Settings.enable_debug){Serial.println(__VA_ARGS__);} // BTserial.println(__VA_ARGS__);}  //DPRINTLN is a macro, debug print with new line
  #else
    #define DPRINT(...)     //now defines a blank line
    #define DPRINTLN(...)   //now defines a blank line
  #endif

  #include <Arduino.h>
  #include <SoftwareSerial.h>
  #include <string.h>
  #include <EEPROM.h>
  #include <stdarg.h>

  #define storage_name_size 16
  #define DEFAULT_READER_SIZE 16
  #define SETTINGS_SIZE 16 // quantity of settings vars
  #define SETTINGS_NAME_SIZE 32 // max length of a setting var name
  

  // TODO: Integrate loading of GlobalSettings from EEPROM,
  // maybe putting defaults in the .h struct definition.
  //
  // TODO: You may need to wait until after admin window to load
  // settings from eeprom, giving you a chance to 'resetToDefaults' if
  // something goes wrong with settings.
  //
  // If you do this, you definitely want to store 'proximity_state_timeout'
  // condition as a separate eeprom byte.
  // On second thought... actually, you will need to load settings, 'reasonable-defaults',
  // at the very least, whenever you startup. Even if just using the admin mode.
  //
  // Maybe make it like OpenWRT, with a basic failsafe boot mode, allowing you
  // to do basic things like reset, reboot, configure, etc.
  // How would we do this? Probably need a hardware pin to signal it?

  struct Storage {
    /*  Private implementation details:
     *  storage_name is a string representation
     *  of the subclass name, since we can't
     *  instrospect the name at runtime.
     *  TODO: Should this be a 'const'?
     */
    char storage_name[storage_name_size];

    /*  Global Settings  */
    
    uint32_t TAG_LAST_READ_TIMEOUT;
    uint32_t TAG_READ_SLEEP_INTERVAL;
    uint32_t READER_CYCLE_LOW_DURATION;
    uint32_t READER_CYCLE_HIGH_DURATION;
    uint8_t READER_POWER_CONTROL_PIN;

    uint32_t admin_timeout;
    int proximity_state;
    int enable_debug;

    char DEFAULT_READER[DEFAULT_READER_SIZE];

    // Temporary EEPROM alternative. I don't think this is used.
    // NOTE: This is NOT part of Settings.
    int state_dev_tmp; // Temporary alternative to setting physical EEPROM, for dev & testing.

    int LED_PIN;
    int BT_RXTX[2];
    int RFID_SERIAL_RX;
    long HW_SERIAL_BAUD;
    int DEBUG_PIN;
    long BT_BAUD;
    long RFID_BAUD;
    int OUTPUT_SWITCH_PIN;


    /*  Constructor  */

    Storage();
    

    /*  Functions  */
    
    int  updateProximityState(int);
    bool updateSetting(int, char[]);
    void getSettingByIndex(int, char[2][SETTINGS_NAME_SIZE]);
    void save();
  };  


  /*  Global / External  Vars & Funcs  */

  extern Storage Settings;
  extern Storage& S;
  extern SoftwareSerial BTserial;

  extern Storage loadStorage(const char[]);

  // Creates an extern constant 2D char array 'SETTING_NAMES' that holds Settings var names.
  // This is used to get names/values by index and to iterate through Settings
  // for UI purposes (displaying lists, displaying individual settings).
  // Note that this array is 0-based, unlike some other SerialMenu lists, which are 1-based.
  //
  //  There are now 5 places where settings need to be managed:
  //  1. Declaration in settings.h
  //  2. Initialization in settings.cpp
  //  3. Retrieving by index in settings.cpp
  //  4. Setting by index in settings.cpp
  //  5. Names index in settings.h (for storage of strings in PROGMEM).
  //
  namespace {  // a nameless namespace
    // See here for why the 'namespace' makes this work. Otherwise we get compilation errors.
    // https://stackoverflow.com/questions/2727582/multiple-definition-in-header-file
    const static char str_0[] PROGMEM = "TAG_LAST_READ_TIMEOUT"; // "String 0" etc are strings to store - change to suit.
    const static char str_1[] PROGMEM = "TAG_READ_SLEEP_INTERVAL";
    const static char str_2[] PROGMEM = "READER_CYCLE_LOW_DURATION";
    const static char str_3[] PROGMEM =  "READER_CYCLE_HIGH_DURATION";
    const static char str_4[] PROGMEM =  "READER_POWER_CONTROL_PIN";
    const static char str_5[] PROGMEM =  "admin_timeout";
    const static char str_6[] PROGMEM =  "proximity_state";
    const static char str_7[] PROGMEM =  "enable_debug";
    const static char str_8[] PROGMEM =  "DEFAULT_READER";
    const static char str_9[] PROGMEM =  "LED_PIN";
    const static char str_10[] PROGMEM = "BT_RXTX";
    const static char str_11[] PROGMEM =  "RFID_SERIAL_RX";
    const static char str_12[] PROGMEM =  "HW_SERIAL_BAUD";
    const static char str_13[] PROGMEM =  "DEBUG_PIN";
    const static char str_14[] PROGMEM =  "BT_BAUD";
    const static char str_15[] PROGMEM =  "RFID_BAUD";
    
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
      str_14,
      str_15
    };
  } // end nameless namespace
    
#endif
