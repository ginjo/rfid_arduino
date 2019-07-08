// Serial Menu Class

#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__

  #include <Arduino.h>
  //#include <Stream.h>
  #include <SoftwareSerial.h>
  #include <string.h>
  #include <stdio.h>


  #include "settings.h"
  #include "led_blinker.h"
  #include "rfid.h"
  
  #define INPUT_BUFFER_LENGTH 24
  #define INPUT_MODE_LENGTH 16
  #define CURRENT_FUNCTION_LENGTH 24

  // TODO: Remove
  //#define TAG_LIST_SIZE 10

  class SerialMenu {
  public:
    Stream *serial_port;
    // TODO: I think run_mode should be a global-global.
    int run_mode; // 0=run, 1=admin
    uint32_t previous_ms;
    uint32_t admin_timeout; // seconds
  	
  	char input_mode[INPUT_MODE_LENGTH];
  
  	// Byte buffer for incoming serial strings
  	char buff[INPUT_BUFFER_LENGTH];
  	int buff_index;
    char current_function[CURRENT_FUNCTION_LENGTH];
  	//uint32_t tags[TAG_LIST_SIZE];
    int selected_menu_item;
    int poll_rfid;

    Led * blinker;

    // Constructor receives a serial port instance
    // from Serial (HardwareSerial) or SoftwareSerial.
    // TODO: Is the parameter name required here? What about the pointer?
    //SerialMenu(Stream *stream_ref);
    SerialMenu(Stream*, Led*);

    // TODO: Do we need to specify the array lengths here?
    
    void begin();
    void showInfo();
    void checkSerialPort();
    void menuSelectedMainItem(char);
    void menuMain();
    void menuListTags();
    void menuAddTag();
    void menuDeleteTag();
    void menuDeleteAllTags();
    void menuShowFreeMemory();
    void menuSettings();
    void menuSelectedSetting(char[]);
    int  byteToAsciiChrNum(char);
    void getLine(char);
    void receiveTagInput(char);
    void setInputMode(const char[]);
    bool matchInputMode(const char[]);
    void setCallbackFunction(const char[]);
    bool matchCurrentFunction(const char[]);
    bool inputAvailable();
    bool inputAvailable(const char[]);
    const char * inputAvailableFor();
    void runCallbacks();
    int addTagString(char[]);
    //bool addTagNum(uint32_t);
    void resetInputBuffer();
    void adminTimeout();
    // argument defaults must be declared here, not defined in implementation.
    void updateAdminTimeout(uint32_t = S.admin_timeout); // seconds
    void exitAdmin();
    void prompt(const char = 0, const char* = "", const char* = "");
    void loop();
  };

#endif
