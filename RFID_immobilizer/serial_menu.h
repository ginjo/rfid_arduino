// Serial Menu Class

#include <Arduino.h>
#include <Stream.h>
#include <string.h>

#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__

#define INPUT_BUFFER_LENGTH 32
#define INPUT_MODE_LENGTH 16
#define TAG_LENGTH 8 // this may no longer be necessary
#define TAG_LIST_SIZE 8

  class SerialMenu {
  private:
    Stream *serial_port;
    
  public:
    //int receive_pin;
    //int transmit_pin;
  	//unsigned long baud_rate;
  	char input_mode[16];
  
  	// Byte buffer for incoming serial strings
  	uint8_t buff[INPUT_BUFFER_LENGTH];
  	int buff_index;
    char current_function[32];
  	unsigned long tags[TAG_LIST_SIZE];

    // constructor receives a serial port instance
    // from Serial (HardwareSerial) or SoftwareSerial.
    SerialMenu(Stream *stream_ref);
    
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
    void setInputMode(char[INPUT_MODE_LENGTH]);
    bool matchInputMode(char[INPUT_MODE_LENGTH]);
    void setCurrentFunction(char[32]);
    bool matchCurrentFunction(char[32]);
    bool inputAvailable();
    bool inputAvailable(char[32]);
    char * inputAvailableFor();
    void runCallbacks();
    bool addTagString(uint8_t[]);
    bool addTagNum(unsigned long);
    void resetInputBuffer();
    void loop();
  
  };

#endif
