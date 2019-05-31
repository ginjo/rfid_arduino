// Serial Menu Class
// Handles human-readable input and output between text-based serial port
// and arduino application (classes, functions, data, et.c.).

#include "serial_menu.h"

  // Gets free-memory, see https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
  // This should really go in a Utility class.
  // It is only here as a quick fix.    
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


  /*** Constructor and Setup ***/

  SerialMenu::SerialMenu(Stream *stream_ref) :
    serial_port(stream_ref),
    buff({}),
    buff_index(0),
    current_function(""),
    input_mode("menu"),
    // See resetAdmin()
    //  run_mode(1), // 0=run, 1=admin
    //  admin_timeout(3), // seconds
    //  previous_ms(millis());
    tags {305441741, 2882343476, 2676915564} // 1234ABCD, ABCD1234, 9F8E7D6C
	{
		// Don't call .begin or Serial functions here, since this is too close to hardware init.
		// The hardware might not be initialized yet, at this point.
    // Call .begin from setup() function instead.
	}
	
	//void SerialMenu::begin(unsigned long baud) {
  void SerialMenu::begin() {
    serial_port->println("SerialMenu Admin Console\r\n");
    showInfo();
    //resetAdmin(5);
    //menuListTags();
	}


  /*** Looping Functions ***/

  void SerialMenu::loop() {
    //serial_port->println("SerialMenu::loop() calling serial_port->println()");
    adminTimeout();
    checkSerialPort();
    runCallbacks();
  }

	// check serial_port every cycle
	void SerialMenu::checkSerialPort() {
	  if (serial_port->available()) {
      resetAdmin(15);
      
      Serial.println("checkSerialPort() serial_port->available() is TRUE");
	    uint8_t byt = serial_port->read();
      
      //  Serial.print("checkSerialPort received byte: ");
      //  Serial.println(char(byt));
      //  Serial.print("checkSerialPort input_mode is: ");
      //  Serial.println(input_mode);
      
	    if (matchInputMode("menu")) {
        // sends incoming byt to menu selector
        Serial.println("checkSerialPort() matchInputMode('menu') is TRUE");
        selectMenuItem(byt);
	    } else if (matchInputMode("line")) {
        // sends incoming byte to getLine()
        Serial.println("checkSerialPort() matchInputMode('line') is TRUE");
        getLine(byt);
	    } else {
        // last-resort default just writes byt to serial_port
        // this should not happen under normal circumstances
        Serial.println("checkSerialPort() no matching input mode, using default");
	      serial_port->write(byt);
	    }
     
	  } // done with available serial_port input
	}

  // check for callbacks every cycle
  void SerialMenu::runCallbacks() {
    if (inputAvailable()) {
      Serial.print("runCallbacks() inputAvailableFor(): ");
      Serial.println(inputAvailableFor());

      if (inputAvailable("menuAddTag")) {
        Serial.println("runCallbacks() selected menuAddTag");
        if (addTagString(buff)) { 
          menuListTags();
        } else {
          Serial.println("runCallbacks() call to addTagString(buff) failed");
        }
        resetInputBuffer();
        setCurrentFunction("");

      // } else if {
      //   ...
      
      } else {
        Serial.println("runCallbacks() inputAvailableFor() 'none/default' ");
        
        setInputMode("menu");
        setCurrentFunction("");
        buff_index = 0;
        strncpy(buff, "", INPUT_BUFFER_LENGTH);
      }
    }
  }


  /*** Menu and State Logic ***/

  // Activates an incoming menu selection.
  void SerialMenu::selectMenuItem(uint8_t byt) {
    Serial.print("selectMenuItem received byte: ");
    Serial.println(char(byt));
    
    switch (char(byt)) {
      // NOTE: A missing 'break' will allow
      // drop-thru to the next case.
      case '1':
        menuListTags();
        break;
      case '2':
        menuAddTag();
        break;
      case '3':
        menuDeleteTag();
        break;
      case '4':
        menuShowFreeMemory();
        break;
      case '5':
        serial_port->println("Exiting admin console\r\n\r\n");
        resetAdmin(0);
        break;
      default:
        menuMain();
        break;
    }
  }

  void SerialMenu::getLine(uint8_t byt) {
    Serial.print("getLine() byt: ");
    Serial.println(char(byt));
    Serial.print("getLine() buff: ");
    Serial.println((char *)buff);
    
    buff[buff_index] = byt;
    buff_index ++;
    serial_port->write(byt);
  
    if (int(byt) == 13) {
      serial_port->println("");
  
      serial_port->print("You entered: ");
      serial_port->println((char*)buff);
      serial_port->println("");

      buff_index = 0;
      setInputMode("menu");
    }
  }

  // TODO: Do parameter var declarations need size, or can they be variable?
  
  void SerialMenu::setInputMode(char str[]) {
    strncpy(input_mode, str, INPUT_MODE_LENGTH);
    
    Serial.print("setInputMode() to: ");
    Serial.println(input_mode);
  }

  bool SerialMenu::matchInputMode(char mode[]) {
    //  Serial.print("matchInputMode() mode, input_mode: ");
    //  Serial.print(mode);
    //  Serial.print(", ");
    //  Serial.println(input_mode);
    
    return strcmp(mode, input_mode) == 0;
  }

  void SerialMenu::setCurrentFunction(char func[]) {
    strncpy(current_function, func, CURRENT_FUNCTION_LENGTH);

    Serial.print("setCurrentFunction() to: ");
    Serial.println(current_function);
  }

  bool SerialMenu::matchCurrentFunction(char func[]) {
    Serial.print("matchCurrentFunction() with: ");
    Serial.print(func);
    Serial.print(", ");
    Serial.println(current_function);
    
    return strcmp(current_function, func) == 0;
  }

  bool SerialMenu::inputAvailable() {
    return buff_index == 0 && buff[0] > 0;
  }
  
  bool SerialMenu::inputAvailable(char func[]) {
    return inputAvailable() && matchCurrentFunction(func);
  }

  char * SerialMenu::inputAvailableFor() {
    if(inputAvailable()) {
      return current_function;
    } else {
      return "";
    }
  }

  //bool SerialMenu::addTagString(uint8_t str[TAG_LENGTH]) {
  bool SerialMenu::addTagString(uint8_t str[]) {
    // Need to discard bogus tags... this kinda works,
    // but needs more failure conditions.
    // TODO: Make sure string consists of only numerics, no other characters.
    // OR, if it's a valid hex string, use that.
    //if (sizeof(buff)/sizeof(*buff) != 8 || buff[0] == 13) {
    //if (buff_index < (TAG_LENGTH -1) || buff[0] == 13) {
    if (buff[0] == 13) {
      strncpy(input_mode, "menu", INPUT_MODE_LENGTH);
      buff_index = 0;
      serial_port->println("");
      return false;
    }
  
    serial_port->print("Tag entered: ");
    serial_port->println((char*)buff);
    serial_port->println("");

    return addTagNum(strtol(buff, NULL, 10));
  }
  
  // Adds tag number to tag list.
  // TODO: Make sure number to add is within bounds of 32-bit integer,
  // since that is as high as the tag ids will go: 4294967295 or 'FFFFFFFF'
  bool SerialMenu::addTagNum(unsigned long tag_num) {
    for (int i = 0; i < TAG_LIST_SIZE; i ++) {
      if (! tags[i] > 0) {
        tags[i] = tag_num;
        setCurrentFunction("");
        return true;
      }
    }

    // fails if didn't return from inner block
    return false;
  }

  void SerialMenu::resetInputBuffer() {
    strncpy(buff, "", INPUT_BUFFER_LENGTH);
    buff_index = 0;
    setInputMode("menu");
  }

  void SerialMenu::adminTimeout() {
    unsigned long current_ms = millis();
    
    //  Serial.print("adminTimeout() run_mode, admin_timeout, now, previous_ms: ");
    //  Serial.print(run_mode); Serial.print(" ");
    //  Serial.print(admin_timeout); Serial.print(" ");
    //  Serial.print(current_ms); Serial.print(" ");
    //  Serial.println(previous_ms);
    
    if (run_mode == 0) { return; }
    if ( (current_ms - previous_ms)/1000 > admin_timeout ) {
      Serial.println("adminTimeout() setting run_mode to 0");
      run_mode = 0;
    }
  }

  void SerialMenu::resetAdmin(int seconds) {
    Serial.print("resetAdmin() seconds: ");
    Serial.println(seconds);
    
    admin_timeout = seconds;
    run_mode = 1;
    previous_ms = millis();
  }


  /*** Draw Menu Items and Log Messages ***/

  void SerialMenu::showInfo() {
    //serial_port->println("serial_port is active!");
    Serial.print("SerialMenu::setup input_mode: ");
    Serial.println(input_mode);
    Serial.print("SerialMenu::setup buff_index: ");
    Serial.println(buff_index);
    Serial.print("SerialMenu::setup tags[2]: ");
    Serial.println(tags[2]);
  }

  void SerialMenu::menuMain() {
    serial_port->println("Menu");
    serial_port->println("1. List tags");
    serial_port->println("2. Add tag");
    serial_port->println("3. Delete tag");
    serial_port->println("4. Show free memory");
    serial_port->println("5. Exit");
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
    setInputMode("line");
    setCurrentFunction(__FUNCTION__);
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
