/*
  A global settings file, intended to facilitate storing persistant
  settings in EEPROM.
  
  This also demonstrates how to do globally accessible vars and functions.
  
*/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

  #define VERSION "v0.1.0.pre208"
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

  // NOTE: Remember to iclude null terminator in all char array lengths.
  #define SETTINGS_SIZE 17 // quantity of settings vars
  #define SETTINGS_NAME_SIZE 27 // max length for 'setting_name' and 'settings_name'. Remember null-terminator.
  #define SETTINGS_VALUE_SIZE 16 // max length of a setting var.
  #define ADMIN_PASSWORD_SIZE 11 // max length of admin password.


  /*
    Each setting in the Settings class must have the following:

    * These are what the program uses.
    Declaration/definition (.h)
    Initializer (.cpp)

    * These are for UI only (using Menu system).
    Getter function (declaration & definition) (.h, .cpp)
    Setter function (declaration & definition) (.h, .cpp)
    Entry in SettingsList array of struct (.cpp)

    Make sure Settings' global constraints (like SETTINGS_SIZE, SETTINGS_NAME_SIZE etc.)
    are set correctly. And don't forget about null-terminators on strings.
    
  */


  /*  Defines types for settings getters and setters  */

  class Settings; // forward declaration for the following...
  // This is a cleaner way to do typedefs.

  // typdef for pointer to function (TODO: is this a pionter to member function?)
  using getter_setter_T = void(Settings::*)(char*);

  // typedef for list of settings containing function pointers.
  using settings_list_T = struct {
    const char name[SETTINGS_NAME_SIZE];
    getter_setter_T display_fp;
    getter_setter_T setter_fp;
  };
  

  class Settings : public Storage<Settings> {
  public:


    /*  Constructors  */

    Settings();


    /*  Static  */

    // Declares a static member-array to hold name, display, and setter for each setting.
    //static settings_list_t const SettingsList[] PROGMEM;
    static settings_list_T const SettingsList[] PROGMEM;
    
    static Settings Current;

    static Settings* Load(Settings *settings_obj = &Current, int _eeprom_address = SETTINGS_EEPROM_ADDRESS);
    
    static bool Failsafe();
    

    /*  Instance  */

    char settings_name[SETTINGS_NAME_SIZE];
    
    uint32_t tag_last_read_hard_timeout;
    uint32_t tag_read_sleep_interval; // ms
    uint32_t reader_cycle_low_duration; // ms
    uint32_t reader_cycle_high_max;
    uint32_t tag_last_read_soft_timeout;

    uint32_t admin_timeout;
    int proximity_state_startup;
    uint8_t debug_level;

    uint8_t default_reader;

    long hw_baud;
    long sw_baud;
    long rfid_baud;

    int tone_frequency;
    int admin_startup_timeout;
    bool log_to_bt; // logging to SWserial: 0=No, 1=Yes (but only if bt is connected)
    uint8_t log_level;
    char admin_password[ADMIN_PASSWORD_SIZE];


    /*  Functions  */
    
    bool updateSetting(int, char[]);
    void getSettingByIndex(int, char*, char*);
    void displaySetting(int, char*);
    void printSettings(Stream*);
    bool debugMode();
    //void save(char*); // This is to accommodate putting 'Save' option into settings list.
    int  save(); 


    /*  Getters and Setters for UI (Menu system)  */

    void display_tag_last_read_hard_timeout(char*);
    void set_tag_last_read_hard_timeout(char*);
    
    void display_tag_read_sleep_interval(char*);
    void set_tag_read_sleep_interval(char*);
    
    void display_reader_cycle_low_duration(char*);
    void set_reader_cycle_low_duration(char*);
    
    void display_tag_last_read_soft_timeout(char*);
    void set_tag_last_read_soft_timeout(char*);
    
    void display_admin_timeout(char*);
    void set_admin_timeout(char*);
    
    void display_proximity_state_startup(char*);
    void set_proximity_state_startup(char*);
    
    void display_debug_level(char*);
    void set_debug_level(char*);
    
    void display_default_reader(char*);
    void set_default_reader(char*);
    
    void display_hw_baud(char*);
    void set_hw_baud(char*);
    
    void display_sw_baud(char*);
    void set_sw_baud(char*);
    
    void display_rfid_baud(char*);
    void set_rfid_baud(char*);
    
    void display_tone_frequency(char*);
    void set_tone_frequency(char*);
    
    void display_admin_startup_timeout(char*);
    void set_admin_startup_timeout(char*);
    
    void display_log_to_bt(char*);
    void set_log_to_bt(char*);
    
    void display_log_level(char*);
    void set_log_level(char*);

    void display_admin_password(char*);
    void set_admin_password(char*);

    void display_reader_cycle_high_max(char*);
    void set_reader_cycle_high_max(char*);

  };  // Settings class


  /*  Global / External Vars & Funcs  */

  // This declares an extern 'S' of type Settings.
  // This will be a ref to Settings::Current
  // and is intended as a convenience shortcut, since
  // Settings::Current is referred to so many times in the project.
  extern Settings& S;
    
#endif  // end __SETTINGS_H__

  
