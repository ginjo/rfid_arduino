// Serial Menu Class

#include "serial_menu.h"

	//SerialMenu::SerialMenu(int rx, int tx, unsigned long baud) :
  //SerialMenu::SerialMenu(Stream *stream_ref, unsigned long baud) :
  SerialMenu::SerialMenu(Stream *stream_ref) :
    serial_port(stream_ref),
		baud_rate(9600),
    bt_state('0'),
    buff_index(0),
    tag_index(3),
    tags {
      {'1','2','3','4','a','b','c','d'},
      {'z','x','c','v','9','8','7','6'},
      {'A','1','B','2','C','3','D','4'}
    }
	{
    //serial_port(receive_pin, transmit_pin);
    //serial_port = ss;
		// Don't call .begin here, since this is too close to hardware init.
		// For example, "serial_port()" above triggers actual hardware init.
		// So the hardware might not be initialized yet, at this point.
    // Call .begin from setup() function instead.
	}

	void SerialMenu::begin() {
		SerialMenu::begin(baud_rate);
	}
	
	void SerialMenu::begin(unsigned long baud) {
    //showInfo();
    serial_port->println("1 SerialMenu::setup calling serial_port->println()");
    delay(500);
    serial_port->println("2 SerialMenu::setup calling serial_port->println()");
    delay(500);
    serial_port->println("3 SerialMenu::setup calling serial_port->println()");
    delay(500);
    serial_port->println("4 SerialMenu::setup calling serial_port->println()");
    delay(500);
    serial_port->println("5 SerialMenu::setup calling serial_port->println()");
    showInfo();
	}

  void SerialMenu::loop() {
    //serial_port->println("SerialMenu::loop() calling serial_port->println()");
    //delay(500);
    handleSerialPort();
  }

  void SerialMenu::showInfo() {
    //serial_port->println("serial_port is active!");
    Serial.print("SerialMenu::setup baud_rate: ");
    Serial.println(baud_rate);
    Serial.print("SerialMenu::setup bt_state: ");
    Serial.println(bt_state);
    Serial.print("SerialMenu::setup buff_index: ");
    Serial.println(buff_index);
    Serial.print("SerialMenu::setup tags[2][2]: ");
    Serial.println(tags[2][2]);
    Serial.print("SerialMenu::setup tag_index: ");
    Serial.println(tag_index);
        
    //Serial.println("SerialMenu::setup printing tags to serial_port");
    //listTags();
  }

	// Handle serial_port
	void SerialMenu::handleSerialPort() {
    //Serial.println("About to call serial_port->...()");
    //delay(500);
    //serial_port->println("SerialMenu::handleSerialPort() calling serial_port->println()");
    //delay(500);
    //return 0;
	  if (serial_port->available()) {
      Serial.println("serial_port->available() is TRUE");
	    uint8_t byt = serial_port->read();
    
      //// debugging
      //serial_port->println("");
      //serial_port->print("BYTE ");
      //serial_port->println(byt);
      //serial_port->print("BYTE_CHAR ");
      //serial_port->println(char(byt));
      //serial_port->print("STATE_CHAR ");
      //serial_port->println(bt_state);
    
	    if (bt_state == '0') {
	      // Draws or selects menu
	      switch (char(byt)) {
	        case '1':
	          menuListTags();
	          bt_state = '0';
	          break;
	        case '2':
            menuAddTag();
	          bt_state = '2';
	          break;
	        case '3':
            menuDeleteTag();
	          bt_state = '0';
	          break;
	        default:
            menuMain();
            bt_state = '0';
            break;
	      }
	    } else if (bt_state == '2') {
        receiveTagInput(byt);
	    // Are either of these last two conditions used?
	    } else if (int(byt) == 13) {
	      // User hit Return
	      serial_port->println("");
	      bt_state = '0';
	    } else {
	      serial_port->write(byt);
	    }
    
	  } // done with available serial_port input
	}

  void SerialMenu::menuMain() {
    serial_port->println("Menu");
    serial_port->println("1. List tags");
    serial_port->println("2. Add tag");
    serial_port->println("3. Delete tag");
    serial_port->println("");
  }

	void SerialMenu::menuListTags() {
    serial_port->println("Menu > Tags");
	  //serial_port->println((char*)tags);
	  for (int i = 0; i < TAG_LIST_SIZE; i ++) {
	    if (! char(tags[i][0])) {
	      return;
	    }
	    serial_port->print(i);
	    serial_port->print(". ");
	    for (int j = 0; j < TAG_LENGTH; j ++) {
	      serial_port->print(char(tags[i][j]));
	    }
	    serial_port->println("");
	  }
    serial_port->println("");
	}

  void SerialMenu::menuAddTag() {
    serial_port->println("Menu > Add tag");
    serial_port->print("Enter a tag number to store: ");
  }

  void SerialMenu::menuDeleteTag() {
    serial_port->println("Menu > Delete tag");
    serial_port->println("");
  }

  void SerialMenu::receiveTagInput(uint8_t byt) {
    buff[buff_index] = byt;
    buff_index ++;
    serial_port->write(byt);
  
    if (int(byt) == 13 || buff_index >= TAG_LENGTH) {
      buff_index = 0;
      serial_port->println("");
  
      // Need to discard bogus tags... this kinda works
      //if (sizeof(buff)/sizeof(*buff) != 8 || buff[0] == 13) {
      if (buff_index < TAG_LENGTH || buff[0] == 13) {
        bt_state = '0';
        serial_port->println("");
        return;
      }
    
      //serial_port->print("Tag entered: ");
      //serial_port->println((char*)buff);
      //for (int i = 0; i < 8; i ++) {
      //  serial_port->write(buff[i]);
      //}
      serial_port->println("");
  
      for (int i = 0; i < TAG_LENGTH; i ++) {
        tags[tag_index][i] = buff[i];
      }
      tag_index ++;
    
      bt_state = '0';
      menuListTags();
      serial_port->println("");
    }
  }
