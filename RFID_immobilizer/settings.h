/*
  A global settings file, intended to facilitate storing persistant
  settings in EEPROM.
  
  This also demonstrates how to do globally accessible vars and functions.
  
  TODO: Maybe make a Settings class to contain all of this?
*/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__
  
  #include <string.h>

  // Examples of how to do global vars & functions.
  // See .cpp file, as it appears to be necessary as well.

  // global constant variables
  extern const char SettingNames[16][24];
  extern const int MySettings[16];

  // global constant functions
  extern const int my_setting(int);
  extern const int index_of(char[], char[16][24]);
  extern const int value_of(char[], char[16][24], int[]);

#endif
