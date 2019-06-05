/*
  A global settings file, intended to facilitate storing persistant
  settings in EEPROM.
  
  This also demonstrates how to do globally accessible vars and functions.
  
  TODO: Maybe make a Settings class to contain all of this?
  TODO: SEE https://community.particle.io/t/eeprom-put-using-a-multi-variable-struct/30561/7
        for an easier way to use EEPROM: put()'ing a struct object.
*/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

  #include <Arduino.h>
  #include <string.h>
  #include <EEPROM.h>

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

    // for class RFID
    //
    //int RAW_TAG_LENGTH 14  // RDM63000
    //int RAW_TAG_LENGTH;  // 7941E
    
    // Mostly time data
    int TAG_LAST_READ_TIMEOUT;
    int TAG_READ_INTERVAL;
    int READER_CYCLE_LOW_DURATION;
    int READER_CYCLE_HIGH_DURATION;
    int READER_POWER_CONTROL_PIN;
    
    // TNew various settings
    int proximity_state;
    int admin_timeout;


    // for class Led
    //
    //  int INTERVALS_LENGTH;

    // for class SerlialMenu
    //
    //  int INPUT_BUFFER_LENGTH;
    //  int INPUT_MODE_LENGTH;
    //  int CURRENT_FUNCTION_LENGTH;
    //  int TAG_LIST_SIZE;

    int updateProximityState(int);

    Storage::Storage();
  };  

  extern Storage Settings;
  extern Storage& S;
  
  //  extern const int TAG_LAST_READ_TIMEOUT;
  //  extern const int TAG_READ_INTERVAL;
  //  extern const int READER_CYCLE_LOW_DURATION;
  //  extern const int READER_CYCLE_HIGH_DURATION;
  //  extern const int READER_POWER_CONTROL_PIN;
  

#endif
