/*
  A global settings file, intended to facilitate storing persistant
  settings in EEPROM.
  
  This also demonstrates how to do globally accessible vars and functions.
  
  TODO: SEE https://community.particle.io/t/eeprom-put-using-a-multi-variable-struct/30561/7
        for an easier way to use EEPROM: put()'ing a struct object.
*/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

  #define VERSION "v0.1.0.pre92"
  #define TIMESTAMP __DATE__ ", " __TIME__

  #include <Arduino.h>
  #include <SoftwareSerial.h>
  #include <string.h>
  // stdarg.h is used to process variadic function args.
  // I don't think we're using it yet.
  //#include <stdarg.h>
  // Moved to .cpp to stop EEPROM warnings.
  //#include <EEPROM.h>

  #include "logger.h"
    
  #define DEFAULT_READER_SIZE 16
  #define SETTINGS_SIZE 17 // quantity of settings vars
  #define SETTINGS_NAME_SIZE 32 // max length of a setting var name
  #define SETTINGS_EEPROM_ADDRESS 100
  

  // TODO: Integrate loading of Global Settings from EEPROM,
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

  class Settings {
  public:
    /*  Implementation details, not settings:
     *  settings_name is a string representation
     *  of the Settings instance Currently usint
     *  it to verify successful loading of instance
     *  from EEPROM (not sure if that works yet).
     */
    char settings_name[SETTINGS_NAME_SIZE];

    /*  Global Settings  */
    
    uint32_t TAG_LAST_READ_TIMEOUT;
    uint32_t TAG_READ_SLEEP_INTERVAL;
    uint32_t READER_CYCLE_LOW_DURATION;
    uint32_t READER_CYCLE_HIGH_DURATION;
    uint8_t READER_POWER_CONTROL_PIN;

    uint32_t admin_timeout;
    int proximity_state;  //  Put this in a state.h file (and class).
    
    int enable_debug;

    char DEFAULT_READER[DEFAULT_READER_SIZE];

    // Temporary EEPROM alternative for dev/testing.
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


    /*  Constructors  */

    Settings();


    /*  Functions  */
    
    int  updateProximityState(int);
    bool updateSetting(int, char[]);
    void getSettingByIndex(int, char[2][SETTINGS_NAME_SIZE]);
    char * displaySetting(int);
    unsigned int getChecksum();

    void save(int address = SETTINGS_EEPROM_ADDRESS); // eeprom address


    /*  Static  */

    static Settings current;

    static Settings * load(int address = SETTINGS_EEPROM_ADDRESS);
  };  


  /*  Global / External  Vars & Funcs  */

  // This declares an extern 'S' of type Settings.
  // This will be a ref (pointer?) to Settings::current
  // and is intended as a convenience shortcut, since
  // Settings::current is referred to so many times in the project.
  extern Settings& S;


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
  // TODO: Should all of this be converted to static Settings var?
  //
  namespace {  // a nameless namespace helps build a 2D array of char strings in PROGMEM.
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
    const static char str_16[] PROGMEM =  "OUTPUT_SWITCH_PIN";
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
      str_15,
      str_16
    };
  } // end nameless namespace
    
#endif  // end __SETTINGS_H__