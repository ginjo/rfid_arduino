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

  #define VERSION "0.1.0.pre62"
  #define TIMESTAMP __DATE__ ", " __TIME__
  
  #define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
  #ifdef DEBUG    //Macros are usually in all capital letters.
    #define DPRINT(...)    if(Settings.enable_debug){Serial.print(__VA_ARGS__);}    //DPRINT is a macro, debug print
    #define DPRINTLN(...)  if(Settings.enable_debug){Serial.println(__VA_ARGS__);}  //DPRINTLN is a macro, debug print with new line
  #else
    #define DPRINT(...)     //now defines a blank line
    #define DPRINTLN(...)   //now defines a blank line
  #endif

  #include <Arduino.h>
  #include <string.h>
  #include <EEPROM.h>
  #include <stdarg.h>

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
    
    /*  Variables  */

    // Temporary EEPROM alternative.
    int state_dev_tmp; // Temporary alternative to setting physical EEPROM, for dev & testing.

    
    // Mostly time data
    uint32_t TAG_LAST_READ_TIMEOUT;
    uint32_t TAG_READ_SLEEP_INTERVAL;
    uint32_t READER_CYCLE_LOW_DURATION;
    uint32_t READER_CYCLE_HIGH_DURATION;
    uint8_t READER_POWER_CONTROL_PIN;
    
    int proximity_state;
    uint32_t admin_timeout;
    int enable_debug;

    //uint8_t RAW_TAG_LENGTH;
    char* DEFAULT_READER;

    // TODO: This should probably be stored with reader specification (in derived Reader).
    //bool READER_POWER_CONTROL_POLARITY;


    /*  Functions  */
    
    int updateProximityState(int);
    bool updateSetting(int, char[]);

    Storage::Storage();
  };  

  extern Storage Settings;
  extern Storage& S;
  
#endif
