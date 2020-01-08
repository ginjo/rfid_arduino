// Serial Menu Class

#ifndef __MENU_H__
#define __MENU_H__

  #include <Arduino.h>
  //#include <Stream.h>
  #include <SoftwareSerial.h>
  #include <string.h>
  // Not sure why we needed this, but seems to work without.
  //#include <stdio.h>


  #include "settings.h"
  #include "reader.h"
  //#include "led_blinker.h"
  #include "stack.h"
  //#include "controller.h" // This would cause circular include. See .cpp file for better solution.

  // See logger.h for master debug controls.
  #ifdef MU_DEBUG
    #define MU_PRINT(...) DPRINT(__VA_ARGS__)
    #define MU_PRINTLN(...) DPRINTLN(__VA_ARGS__)
  #else
    #define MU_PRINT(...)
    #define MU_PRINTLN(...)
  #endif
  
  #define INPUT_BUFFER_LENGTH 24


  class Menu : public Stack<Menu> {
  public:
    Stream *serial_port;
    Reader *reader;
    // NOTE: instance_name is a two-character string with a terminating null.
    char instance_name[3];
    uint32_t previous_ms;
    uint32_t admin_timeout; // seconds
  	  
  	// Byte buffer for incoming serial strings
  	char buff[INPUT_BUFFER_LENGTH];
  	int buff_index;
    int selected_menu_item;
    int get_tag_from_scanner;

    // Constructor receives a serial port instance
    // from HardwareSerial or SoftwareSerial.
    Menu(Stream*, Reader*, const char* = "");

    /* Control */
    void begin();
    void loop();
    void adminTimeout();
    void updateAdminTimeout(uint32_t = S.admin_timeout); // seconds
    void exitAdmin();

    /* Input */
    void checkSerialPort();
    void clearSerialPort();
    void resetInputBuffer();
    bool bufferReady();
    void prompt(const char[] = "", CB = nullptr, bool=false);
    //void getLine(char);
    void readLineWithCallback(CB, bool=false);
    void readLine(void* = nullptr);
    void getTagFromScanner();

    /* Data */
    int  byteToAsciiChrNum(char);
    void addTagString(void*);
    void deleteTag(void*);
    void deleteAllTags(void*);
    void updateSetting(void*);

    /* Commands */
    void menuSelectedMainItem(void* = nullptr);
    void menuMain(void* = nullptr);
    void menuMainPrompt(const char[] = "Select a menu item");
    void menuListTags(void* = nullptr, CB = nullptr);
    void menuAddTag(void* = nullptr);
    void menuDeleteTag(void* = nullptr);
    void menuDeleteAllTags(void* = nullptr);
    void menuShowFreeMemory();
    void menuSettings(void* = nullptr);
    void menuSelectedSetting(void* = nullptr); // (char[]);    


    /*  Static Vars & Functions  */

    static int run_mode; // 0=run, 1=admin

    static Menu * Current;
    static Menu * HW;
    static Menu * SW;
    
    static void Begin();
    static void Loop();
    
  }; // Menu

#endif
