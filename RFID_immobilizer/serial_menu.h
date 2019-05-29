// Serial Menu Class

#include <Arduino.h>
#include <Stream.h>
#include <stdlib.h>

#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__

#define TAG_LENGTH 8
#define TAG_LIST_SIZE 8

  class SerialMenu {
  private:
    Stream *serial_port;
    
  public:
    //int receive_pin;
    //int transmit_pin;
  	//unsigned long baud_rate;
  	char menu_state[16];
  
  	// Byte buffer for incoming serial strings
  	uint8_t buff[TAG_LENGTH];
  	int buff_index;
  
  	unsigned long tags[TAG_LIST_SIZE];
  	//int tag_index; // TODO: no longer needed.

    // constructor receives a serial port instance
    // from Serial (HardwareSerial) or SoftwareSerial.
    SerialMenu(Stream *stream_ref);
    
    void begin();
    void showInfo();
    void checkSerialPort();
    void selectMenuItem(char);
    void menuMain();
    void menuListTags();
    void menuAddTag();
    void menuDeleteTag();
    void menuShowFreeMemory();
    void receiveTagInput(uint8_t);
    void addTagNum(unsigned long);
    void loop();
  
  };

#endif
