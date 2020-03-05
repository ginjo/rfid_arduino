// Serial Menu Class

#ifndef __MENU_H__
#define __MENU_H__

  #include <Arduino.h>
  //#include <Stream.h>
  #include <SoftwareSerial.h>
  #include <string.h>
  // Is this needed here but is being included by another file?
  //#include <stdio.h>


  #include "settings.h"
  #include "reader.h"
  #include "stack.h"

  // See logger.h for master debug controls.
  #ifdef MU_DEBUG
    #define MU_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define MU_LOG(...)
  #endif

  #define INPUT_BUFFER_LENGTH 24
  #define MENU_ITEMS_SIZE 11
  #define MENU_ITEM_NAME_SIZE 16

  using CB = Stack<Menu>::CB;
  using menu_item_T = struct { // This is like typdef buf better syntax.
    const char name[MENU_ITEM_NAME_SIZE];
    CB fp;
    void *data;
    //CB callback;
  };


  class Menu : public Stack<Menu> {
  public:


    /*  Static Vars & Functions  */
        
    static int RunMode; // 0=run, 1=admin

    static Menu * Current;
    static Menu * M1;
    static Menu * M2;

    static menu_item_T const MenuItems[] PROGMEM;
    
    static void Begin();
    static void Loop();


    /* Instance vars */
    
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


    /*  Constructor
        Receives a serial port instance from HardwareSerial or SoftwareSerial.
    */

    Menu(Stream*, Reader*, const char* = "");


    /* Control */

    void begin();
    void loop();
    void adminTimeout();
    //void updateAdminTimeout(uint32_t = S.admin_timeout); // seconds
    void updateAdminTimeout(void* = (void*)S.admin_timeout); // seconds
    void exitAdmin();


    /* Input */

    void checkSerialPort();
    void clearSerialPort();
    void resetInputBuffer();
    bool bufferReady();
    //void prompt(const __FlashStringHelper*, CB = nullptr, bool = false);
    void prompt_P(const char*, CB = nullptr, bool = false);
    void prompt(const char* = "", CB = nullptr, bool=false);
    //void getLine(char);
    void readLineWithCallback(CB, bool=false);
    void readLine(void* = nullptr);
    void getTagFromScanner();


    /* Data
       Note that some of these will complain at compile time about unused parameter... it's OK
    */
    int  byteToAsciiChrNum(char);
    void addTagString(void*);
    void deleteTag(void*);
    void deleteAllTags(void*);
    void updateSetting(void*);


    /* Commands */

    void menuSelectedMainItem(void* = nullptr);
    void menuMain(void* = nullptr);
    void menuMainPrompt(const char[] = "Select a menu item");
    void menuListTags(void* = nullptr); // if you pass 0 or 1 arg
    void menuListTags(void*, CB);       // if you pass 2 args
    void menuAddTag(void* = nullptr);
    void menuDeleteTag(void* = nullptr);
    void menuDeleteAllTags(void* = nullptr);
    void menuShowFreeMemory(void* = nullptr);
    void menuReboot(void* = nullptr);
    void menuSettings(void* = nullptr);
    void menuSelectedSetting(void* = nullptr); // (char[]);
    void menuSaveSettings(void* = nullptr);
    void menuHandleSaveSettings(void* = nullptr);
    void menuListReaders(void* = nullptr);
    void menuSelectedReader(void* = nullptr);
    void menuManageBT(void* = nullptr);
    void menuSendAtCommand(void* = nullptr);
    void menuPrintATResponse(void* = nullptr);
    void menuLogin(void* = nullptr);
    void menuProcessLogin(void* = nullptr);
    
  }; // Menu

#endif

  
