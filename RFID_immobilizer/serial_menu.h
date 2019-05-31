// Serial Menu Class

#include <Arduino.h>
//#include <Stream.h>
#include <SoftwareSerial.h>
#include <string.h>

#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__

#define INPUT_BUFFER_LENGTH 32
#define INPUT_MODE_LENGTH 16
#define CURRENT_FUNCTION_LENGTH 32
//#define TAG_LENGTH 8 // this may no longer be necessary
#define TAG_LIST_SIZE 8

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

    // constructor receives a serial port instance
    // from Serial (HardwareSerial) or SoftwareSerial.
    // TODO: Is the parameter name required here? What about the pointer?
    SerialMenu(Stream *stream_ref);

    // TODO: Do we need to specify the array lengths here?
    
    void begin();
    void showInfo();
    void checkSerialPort();
    void selectMenuItem(uint8_t);
    void menuMain();
    void menuListTags();
    void menuAddTag();
    void menuDeleteTag();
    void menuShowFreeMemory();
    void getLine(uint8_t);
    void receiveTagInput(uint8_t);
    void setInputMode(char[]);
    bool matchInputMode(char[]);
    void setCurrentFunction(char[]);
    bool matchCurrentFunction(char[]);
    bool inputAvailable();
    bool inputAvailable(char[]);
    char * inputAvailableFor(); // returns char array
    void runCallbacks();
    bool addTagString(uint8_t[]);
    bool addTagNum(unsigned long);
    void resetInputBuffer();
    void adminTimeout();
    void resetAdmin(int); // seconds
    void loop();
  
  };

#endif
