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
  #include "stack.h"

  // See logger.h for master debug controls.
  #ifdef MU_DEBUG
    #define MU_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define MU_LOG(...)
  #endif
  
  #define INPUT_BUFFER_LENGTH 24


  class Menu : public Stack<Menu> {
  public:

    /*  Static Vars & Functions  */
    static int RunMode; // 0=run, 1=admin

    static Menu * Current;
    static Menu * HW;
    static Menu * SW;
    
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

    /* Constructor receives a serial port instance from HardwareSerial or SoftwareSerial. */
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

    /* Data
       Note that some of these will complain at compile time about unused parameter... it's OK */
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
    void menuReboot();
    void menuSettings(void* = nullptr);
    void menuSelectedSetting(void* = nullptr); // (char[]);
    void menuListReaders(void* = nullptr);
    void menuSelectedReader(void* = nullptr);
    void menuManageBT(void* = nullptr);
    void menuSendAtCommand(void* = nullptr);
    void menuPrintATResponse(void* = nullptr);
    
  }; // Menu

#endif

  
