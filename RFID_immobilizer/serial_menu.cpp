// Serial Menu Class
// Handles human-readable input and output between text-based serial port
// and arduino application (classes, functions, data, et.c.).

#include "serial_menu.h"


  // This should really go in a Utility class.
  // It is only here as a quick fix.    
  // Free memory from https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
  #ifdef __arm__
  // should use uinstd.h to define sbrk but Due causes a conflict
  extern "C" char* sbrk(int incr);
  #else  // __ARM__
  extern char *__brkval;
  #endif  // __arm__
  
  static int freeMemory() {
    char top;
  #ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
  #else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
  #endif  // __arm__
  }
  

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
    menu_state("menu"),
    buff_index(0),
    tags {305441741, 2882343476, 2676915564} // 1234ABCD, ABCD1234, 9F8E7D6C
	{
		// Don't call .begin here, since this is too close to hardware init.
		// The hardware might not be initialized yet, at this point.
    // Call .begin from setup() function instead.
	}
	
	//void SerialMenu::begin(unsigned long baud) {
  void SerialMenu::begin() {
    serial_port->println("SerialMenu is active\r\n");
    showInfo();
    //menuListTags();
	}

  void SerialMenu::loop() {
    //serial_port->println("SerialMenu::loop() calling serial_port->println()");
    //delay(500);
    checkSerialPort();
  }

  void SerialMenu::showInfo() {
    //serial_port->println("serial_port is active!");
    Serial.print("SerialMenu::setup menu_state: ");
    Serial.println(menu_state);
    Serial.print("SerialMenu::setup buff_index: ");
    Serial.println(buff_index);
    Serial.print("SerialMenu::setup tags[2]: ");
    Serial.println(tags[2]);
  }

	// Handle serial_port
	void SerialMenu::checkSerialPort() {
	  if (serial_port->available()) {
      //Serial.println("serial_port->available() is TRUE");
	    uint8_t byt = serial_port->read();
      
      //  Serial.print("checkSerialPort received byte: ");
      //  Serial.println(char(byt));
      //  Serial.print("checkSerialPort menu_state is: ");
      //  Serial.println(menu_state);

      // TODO: should (can?) this use a switch statement?
      
	    if (strcmp(menu_state, "menu") == 0) {
        // sends incoming byt to menu selector
        selectMenuItem(char(byt));
	    } else if (strcmp(menu_state, "add_tag") == 0) {
        // sends incoming byte to add-a-tag handler
        receiveTagInput(byt);
	    } else if (int(byt) == 13) {
	      // user hit return in unhandled state
	      serial_port->println("");
	      strncpy(menu_state, "menu", 16);
	    } else {
        // last-resort default just writes byt to serial_port
	      serial_port->write(byt);
	    }
     
	  } // done with available serial_port input
	}

  // Activates an incoming menu selection.
  void SerialMenu::selectMenuItem(char byt) {
    //  Serial.print("selectMenuItem received byte: ");
    //  Serial.print(byt);
    
    switch (char(byt)) {
      case '1':
        menuListTags();
        strncpy(menu_state, "menu", 16);
        break;
      case '2':
        menuAddTag();
        strncpy(menu_state, "add_tag", 16);
        break;
      case '3':
        menuDeleteTag();
        strncpy(menu_state, "menu", 16);
        break;
      case '4':
        menuShowFreeMemory();
        strncpy(menu_state, "menu", 16);
        break;
      default:
        menuMain();
        strncpy(menu_state, "menu", 16);
        break;
    }
  }

  void SerialMenu::menuMain() {
    serial_port->println("Menu");
    serial_port->println("1. List tags");
    serial_port->println("2. Add tag");
    serial_port->println("3. Delete tag");
    serial_port->println("4. Show free memory");
    serial_port->println("");
  }

  // Lists tags for menu.
  void SerialMenu::menuListTags() {
    serial_port->println("Tags");
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
    serial_port->println("Add Tag");
    serial_port->print("Enter a tag number (unsigned long) to store: ");
  }

  // Asks user for index of tag to delete from EEPROM.
  void SerialMenu::menuDeleteTag() {
    serial_port->println("Delete Tag");
    serial_port->println("");
  }

  void SerialMenu::menuShowFreeMemory() {
    serial_port->print("Free Memory: ");
    serial_port->println(freeMemory());
    serial_port->println();
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
        strncpy(menu_state, "menu", 16);
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
    
      strncpy(menu_state, "menu", 16);
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
