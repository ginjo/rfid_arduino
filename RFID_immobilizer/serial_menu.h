// Serial Menu Class

#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__

  #include <Arduino.h>
  //#include <Stream.h>
  #include <SoftwareSerial.h>
  #include <string.h>

  #include "settings.h"
  #include "led_blinker.h"
  
  #define INPUT_BUFFER_LENGTH 16
  #define INPUT_MODE_LENGTH 16
  #define CURRENT_FUNCTION_LENGTH 24
  #define TAG_LIST_SIZE 5

  class SerialMenu {
  private:
    Stream *serial_port;
    
  public:
    int run_mode; // 0=run, 1=admin
    unsigned long previous_ms;
    int admin_timeout; // seconds
  	
  	char input_mode[INPUT_MODE_LENGTH];
  
  	// Byte buffer for incoming serial strings
  	uint8_t buff[INPUT_BUFFER_LENGTH];
  	int buff_index;
    char current_function[CURRENT_FUNCTION_LENGTH];
  	unsigned long tags[TAG_LIST_SIZE];
    int selected_menu_item;

    Led * blinker;

    // constructor receives a serial port instance
    // from Serial (HardwareSerial) or SoftwareSerial.
    // TODO: Is the parameter name required here? What about the pointer?
    //SerialMenu(Stream *stream_ref);
    SerialMenu(Stream*, Led*);

    // TODO: Do we need to specify the array lengths here?
    
    void begin();
    void showInfo();
    void checkSerialPort();
    void menuSelectedMainItem(uint8_t);
    void menuMain();
    void menuListTags();
    void menuAddTag();
    void menuDeleteTag();
    void menuShowFreeMemory();
    void menuSettings();
    void menuSelectedSetting(uint8_t);
    int  byteToAsciiChrNum(char*);
    void getLine(uint8_t);
    void receiveTagInput(uint8_t);
    void setInputMode(char[]);
    bool matchInputMode(char[]);
    void setCallbackFunction(char[]);
    bool matchCurrentFunction(char[]);
    bool inputAvailable();
    bool inputAvailable(char[]);
    char * inputAvailableFor(); // returns char array
    void runCallbacks();
    bool addTagString(uint8_t[]);
    bool addTagNum(unsigned long);
    void resetInputBuffer();
    void adminTimeout();
    void setAdminWithTimeout(int = S.admin_timeout); // seconds
    void exitAdmin();
    void loop();
  
  };

#endif
