// Serial Menu Class

#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__

  #include <Arduino.h>
  //#include <Stream.h>
  #include <SoftwareSerial.h>
  #include <string.h>
  // Not sure why we needed this, but seems to work without.
  //#include <stdio.h>


  #include "settings.h"
  #include "reader.h"
  #include "led_blinker.h"
  #include "stack.h"
  //#include "rfid.h" // This would cause circular include. See .cpp file for better solution.
  
  #define INPUT_BUFFER_LENGTH 24
  #define INPUT_MODE_LENGTH 16
  #define CURRENT_FUNCTION_LENGTH 24


  class SerialMenu : public Stack<SerialMenu> {
  public:
    Stream *serial_port;
    Reader *reader;
    // TODO: I think run_mode should be a global.
    //int run_mode; // 0=run, 1=admin // This was moved to static member.
    // NOTE: instance_name is a two-character string with a terminating null.
    char instance_name[3];
    uint32_t previous_ms;
    uint32_t admin_timeout; // seconds
  	
  	char input_mode[INPUT_MODE_LENGTH];
  
  	// Byte buffer for incoming serial strings
  	char buff[INPUT_BUFFER_LENGTH];
  	int buff_index;
    char current_function[CURRENT_FUNCTION_LENGTH];
    int selected_menu_item;
    int get_tag_from_scanner;

    Led * blinker;

    // Constructor receives a serial port instance
    // from Serial (HardwareSerial) or SoftwareSerial.
    // TODO: Is the parameter name required here? What about the pointer?
    //SerialMenu(Stream *stream_ref);
    SerialMenu(Stream*, Reader*, Led*, const char* = "");
    
    void begin();
    void showInfo();
    void checkSerialPort();
    void menuSelectedMainItem(void* = NULL);
    void menuMain(void* = NULL);
    void menuListTags(void* = NULL);
    void menuAddTag(void* = NULL);
    void menuDeleteTag(void* = NULL);
    void menuDeleteAllTags(void* = NULL);
    void menuShowFreeMemory();
    void menuSettings(void* = NULL);
    void menuSelectedSetting(void* = NULL); // (char[]);
    int  byteToAsciiChrNum(char);
    void getLine(char);
    void readLineWithCallback(CB, bool=false);
    void readLine(void* = NULL);
    void receiveTagInput(char);
    //  void setInputMode(const char[]);
    //  bool matchInputMode(const char[]);
    //  void setCallbackFunction(const char[]);
    //  bool matchCurrentFunction(const char[]);
    //  bool inputAvailable();
    //  bool inputAvailable(const char[]);
    //  const char * inputAvailableFor();
    //  void runCallbacks();
    void addTagString(void*);
    void deleteTag(void*);
    void updateSetting(void*);
    void resetInputBuffer();
    void adminTimeout();
    // argument defaults must be declared here, not defined in implementation.
    void updateAdminTimeout(uint32_t = S.admin_timeout); // seconds
    void exitAdmin();
    void prompt(const char[] = "", CB = NULL);
    void getTagFromScanner();
    void loop();


    /*  Static Vars & Functions  */

    static int run_mode; // 0=run, 1=admin 

    static SerialMenu * Current;
    static SerialMenu * HW;
    static SerialMenu * SW;
    
    static void Begin();
    static void Loop();
    
  }; // SerialMenu

#endif
