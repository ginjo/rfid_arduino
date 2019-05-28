// Serial Menu Class

#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__
  #include <Arduino.h>
  #include <SoftwareSerial.h>
  
  class SerialMenu {
  public:
  
  	Stream *serial_port;
    int receive_pin;
    int transmit_pin;
  	unsigned long baud_rate;
  	char bt_state;
  
  	// Byte buffer for incoming BTserial strings
  	uint8_t buff[8];
  	int buff_index;
  
  	char tags[8][8];
  	int tag_index;
    

    // constructor
    //SerialMenu(int, int, unsigned long);
    //SerialMenu(Stream, unsigned long);
    SerialMenu(Stream *stream_ref);
    
    void setup();
    void setup(unsigned long);
    void showInfo();
    void handleSerialPort();
    void menuMain();
    void menuListTags();
    void menuAddTag();
    void menuDeleteTag();
    void receiveTagInput(uint8_t);
    void loop();
  
  };

#endif
