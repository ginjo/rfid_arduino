// Serial Menu Class
// Handles human-readable input and output between text-based serial port
// and arduino application (classes, functions, data, et.c.).

#include "serial_menu.h"
#include <stdlib.h>

  // NOTE: Here is a simple formula to convert a hex string to dec integer (unsigned long).
  // This works in onlinegbd.com, but may not work for arduino.
  //  #include <stdlib.h>
  //  int main()
  //  {
  //      char str[] = "9F8E7D6C";
  //      unsigned long num = strtol(str, NULL, 16);
  //      printf("%u", num);
  //  }

  SerialMenu::SerialMenu(Stream *stream_ref) :
    serial_port(stream_ref),
		//baud_rate(9600),
    menu_state('0'),
    buff_index(0),
    tags {305441741, 2882343476, 2676915564} // 1234ABCD, ABCD1234, 9F8E7D6C
	{
		// Don't call .begin here, since this is too close to hardware init.
		// For example, "serial_port()" above triggers actual hardware init.
		// So the hardware might not be initialized yet, at this point.
    // Call .begin from setup() function instead.
	}
	
	//void SerialMenu::begin(unsigned long baud) {
  void SerialMenu::begin() {
    serial_port->println("SerialMenu::setup calling showInfo()");
    showInfo();
	}

  void SerialMenu::loop() {
    //serial_port->println("SerialMenu::loop() calling serial_port->println()");
    //delay(500);
    checkSerialPort();
  }

  void SerialMenu::showInfo() {
    //serial_port->println("serial_port is active!");
    //Serial.print("SerialMenu::setup baud_rate: ");
    //Serial.println(baud_rate);
    Serial.print("SerialMenu::setup menu_state: ");
    Serial.println(menu_state);
    Serial.print("SerialMenu::setup buff_index: ");
    Serial.println(buff_index);
    Serial.print("SerialMenu::setup tags[2]: ");
    Serial.println(tags[2]);
    //Serial.print("SerialMenu::setup tag_index: ");
    //Serial.println(tag_index);
        
    //Serial.println("SerialMenu::setup printing tags to serial_port");
    //listTags();
  }

	// Handle serial_port
	void SerialMenu::checkSerialPort() {

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
      //serial_port->println(menu_state);
    
	    if (menu_state == '0') {
	      // Draws or selects menu
	      switch (char(byt)) {
	        case '1':
	          menuListTags();
	          menu_state = '0';
	          break;
	        case '2':
            menuAddTag();
	          menu_state = '2';
	          break;
	        case '3':
            menuDeleteTag();
	          menu_state = '0';
	          break;
	        default:
            menuMain();
            menu_state = '0';
            break;
	      }
	    } else if (menu_state == '2') {
        receiveTagInput(byt);
	    // Are either of these last two conditions used?
	    } else if (int(byt) == 13) {
	      // User hit Return
	      serial_port->println("");
	      menu_state = '0';
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

  // Lists tags for menu.
  void SerialMenu::menuListTags() {
    serial_port->println("Menu > Tags");
    //serial_port->println((char*)tags);
    // TODO: Move the bulk of this to RFIDTags?
    for (int i = 0; i < TAG_LIST_SIZE; i ++) {
      if (tags[i] > 0) {
        serial_port->print(i);
        serial_port->print(". ");
        serial_port->print(tags[i]);
        serial_port->println("");
      }
    }
    serial_port->println("");
  }

  // Asks user for full tag,
  // sets mode to receive-text-line-from-serial,
  // stores received tag (with validation) using RFIDTags class.
  void SerialMenu::menuAddTag() {
    serial_port->println("Menu > Add tag");
    serial_port->print("Enter a tag number (unsigned long) to store: ");
  }

  // Asks user for index of tag to delete from EEPROM.
  void SerialMenu::menuDeleteTag() {
    serial_port->println("Menu > Delete tag");
    serial_port->println("");
  }

  // TODO: Make a generic function for receiving line(s?) of input.
  // Example: ... SerialMenu::receiveInput(int lines, char stop, char var_name[]);
  // The suggestion function would have to put the instance of SerialMenu
  // into a receive bytes mode, capable of spanning multiple loop cycles,
  // like this existing receiveTagInput does.
  // 
  // TODO: Perhaps this class should be called SerialCLI,
  // where the 'menu' is just a mode of interaction with the port,
  // and receiving lines of data would be another mode of interaction.
  // 
  // TODO: I think tag-related functions should be in their own class: Tags.
  // The RFID reader should probably also have its own class, maybe?
  //
  // TODO: Hmmm, we have RFID, serial-cli, tag management, timer-switch management.
  // Do each of those need theier own class, or should some be combined.
  //
  void SerialMenu::receiveTagInput(uint8_t byt) {
    buff[buff_index] = byt;
    buff_index ++;
    serial_port->write(byt);
  
    if (int(byt) == 13 || buff_index >= TAG_LENGTH) {
      serial_port->println("");
  
      // Need to discard bogus tags... this kinda works
      //if (sizeof(buff)/sizeof(*buff) != 8 || buff[0] == 13) {
      if (buff_index < (TAG_LENGTH -1) || buff[0] == 13) {
        menu_state = '0';
        buff_index = 0;
        serial_port->println("");
        return;
      }
    
      serial_port->print("Tag entered: ");
      serial_port->println((char*)buff);
      serial_port->println("");

      addTagNum(strtol(buff, NULL, 10));
      
      // TODO: Is tag_index still necessary? I don't think so.
      //tag_index ++;
    
      menu_state = '0';
      buff_index = 0;
      menuListTags();
    }
  }

  // Adds tag number to tag list.
  void SerialMenu::addTagNum(unsigned long tag_num) {
    for (int i = 0; i < TAG_LIST_SIZE; i ++) {
      if (! tags[i] > 0) {
        tags[i] = tag_num;
        return 0;
      }
    }
  }
