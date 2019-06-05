// Serial Menu Class
// Handles human-readable input and output between text-based serial port
// and arduino application (classes, functions, data, et.c.).

// TODO: Create a function for menu option "Read Tag",
// that temporarily switches run_mode to 0, gathers a single tag,
// then returns run_mode to 1 so BTmenu can resume where it left off.
// This could allow a menu option for "Add tag from scanner",
// vs the current add-tag-from-keyboard menu option.

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

  SerialMenu::SerialMenu(Stream *stream_ref, Led * _blinker) :
    serial_port(stream_ref),
    buff({}),
    buff_index(0),
    current_function(""),
    input_mode("menu"),
    blinker(_blinker),
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
    serial_port->println(F("SerialMenu Admin Console\r\n"));
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
      
      Serial.println(F("checkSerialPort() serial_port->available() is TRUE"));
	    uint8_t byt = serial_port->read();
      
      //  Serial.print("checkSerialPort received byte: ");
      //  Serial.println(char(byt));
      //  Serial.print("checkSerialPort input_mode is: ");
      //  Serial.println(input_mode);
      
	    if (matchInputMode("menu")) {
        // sends incoming byt to menu selector
        Serial.println(F("checkSerialPort() matchInputMode('menu') is TRUE"));
        selectMenuItem(byt);
	    } else if (matchInputMode("menu_settings")) {
        // sends incoming byt to settings selector
        Serial.println(F("checkSerialPort() matchInputMode('menu_settings') is TRUE"));
        menuSelectedSetting(byt);
	    } else if (matchInputMode("line")) {
        // sends incoming byte to getLine()
        Serial.println(F("checkSerialPort() matchInputMode('line') is TRUE"));
        getLine(byt);
	    } else {
        // last-resort default just writes byt to serial_port
        // this should not happen under normal circumstances
        Serial.println(F("checkSerialPort() no matching input mode, using default"));
	      serial_port->write(byt);
	    }
     
	  } // done with available serial_port input
	}

  // check for callbacks every cycle
  void SerialMenu::runCallbacks() {
    if (inputAvailable()) {
      Serial.print(F("runCallbacks() inputAvailableFor(): "));
      Serial.println(inputAvailableFor());

      if (inputAvailable("menuAddTag")) {
        Serial.println(F("runCallbacks() selected menuAddTag"));
        if (addTagString(buff)) { 
          menuListTags();
        } else {
          Serial.println(F("runCallbacks() call to addTagString(buff) failed"));
        }
        resetInputBuffer();
        setCurrentFunction("");

      // } else if {
      //   ...
      
      } else {
        Serial.println(F("runCallbacks() inputAvailableFor() 'none/default' "));
        
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
    Serial.print(F("selectMenuItem received byte: "));
    Serial.println(char(byt));
    
    switch (char(byt)) {
      // NOTE: A missing 'break' will allow
      // drop-thru to the next case.
      case '0':
        serial_port->println(F("Exiting admin console\r\n\r\n"));
        resetAdmin(0);     
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
        menuSettings();
        break;
      default:
        menuMain();
        break;
    }
  }

  void SerialMenu::getLine(uint8_t byt) {
    Serial.print(F("getLine() byt: "));
    Serial.println(char(byt));
    Serial.print("getLine() buff: ");
    Serial.println((char *)buff);
    
    buff[buff_index] = byt;
    buff_index ++;
    serial_port->write(byt);
  
    if (int(byt) == 13) {
      serial_port->println("");
  
      serial_port->print(F("You entered: "));
      serial_port->println((char*)buff);
      serial_port->println("");

      buff_index = 0;
      setInputMode("menu");
    }
  }

  // TODO: Do parameter var declarations need size, or can they be variable?
  
  void SerialMenu::setInputMode(char str[]) {
    strncpy(input_mode, str, INPUT_MODE_LENGTH);
    
    Serial.print(F("setInputMode() to: "));
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

    Serial.print(F("setCurrentFunction() to: "));
    Serial.println(current_function);
  }

  bool SerialMenu::matchCurrentFunction(char func[]) {
    Serial.print(F("matchCurrentFunction() with: "));
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
  
    serial_port->print(F("Tag entered: "));
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
      Serial.println(F("adminTimeout() setting run_mode to 0 'run'"));
      blinker->off();
      run_mode = 0;
    }

    // Sets proximity_state to 0 (false or shutdown) if active admin mode.
    if (run_mode == 1) { digitalWrite(13, 0); }
  }

  void SerialMenu::resetAdmin(int seconds) {
    Serial.print(F("resetAdmin() seconds: "));
    Serial.println(seconds);
    
    admin_timeout = seconds;
    run_mode = 1;
    previous_ms = millis();
  }


  /*** Draw Menu Items and Log Messages ***/

  // This is just for logging.
  void SerialMenu::showInfo() {
    //serial_port->println("serial_port is active!");
    Serial.print(F("SerialMenu::setup input_mode: "));
    Serial.println(input_mode);
    Serial.print(F("SerialMenu::setup buff_index: "));
    Serial.println(buff_index);
    Serial.print(F("SerialMenu::setup tags[2]: "));
    Serial.println(tags[2]);
  }

  // The rest of these are menu functions.
  
  void SerialMenu::menuMain() {
    serial_port->println(F("Menu"));
    serial_port->println(F("0. Exit"));
    serial_port->println(F("1. List tags"));
    serial_port->println(F("2. Add tag"));
    serial_port->println(F("3. Delete tag"));
    serial_port->println(F("4. Show free memory"));
    serial_port->println(F("5. Settings"));
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
    serial_port->println(F("Add Tag"));
    serial_port->print(F("Enter a tag number (unsigned long) to store: "));
  }

  // Asks user for index of tag to delete from EEPROM.
  void SerialMenu::menuDeleteTag() {
    serial_port->println(F("Delete Tag"));
    serial_port->println("");
  }

  void SerialMenu::menuShowFreeMemory() {
    serial_port->print(F("Free Memory: "));
    serial_port->println(freeMemory());
    serial_port->println();
  }

  void SerialMenu::menuSettings() {
    setInputMode("menu_settings");
    setCurrentFunction(__FUNCTION__);
    serial_port->println(F("Settings"));
    serial_port->println(F("0. Return to main menu"));
    serial_port->println(F("1. TAG_LAST_READ_TIMEOUT"));
    serial_port->println(F("2. TAG_READ_INTERVAL"));
    serial_port->println(F("3. READER_CYCLE_LOW_DURATION"));
    serial_port->println(F("4. READER_CYCLE_HIGH_DURATION"));
    serial_port->println(F("5. READER_POWER_CONTROL_PIN"));
    serial_port->println(F("6. proximity_state"));
    serial_port->println("");
  }

  // Handle selected setting.
  void SerialMenu::menuSelectedSetting(uint8_t byt) {
    Serial.print(F("menuSelectedSetting received byte: "));
    Serial.println(char(byt));
    
    switch (char(byt)) {
      // NOTE: A missing 'break' will allow
      // drop-thru to the next case.
      case '0':
        menuMain();
        break;
      case '1':
        serial_port->print(F("TAG_LAST_READ_TIMEOUT: "));
        serial_port->println(S.TAG_LAST_READ_TIMEOUT);
        break;
      case '2':
        serial_port->print(F("TAG_READ_INTERVAL: "));
        serial_port->println(S.TAG_READ_INTERVAL);
        break;
      case '3':
        serial_port->print(F("READER_CYCLE_LOW_DURATION: "));
        serial_port->println(S.READER_CYCLE_LOW_DURATION);
        break;
      case '4':
        serial_port->print(F("READER_CYCLE_HIGH_DURATION: "));
        serial_port->println(S.READER_CYCLE_HIGH_DURATION);
        break;
      case '5':
        serial_port->print(F("READER_POWER_CONTROL_PIN: "));
        serial_port->println(S.READER_POWER_CONTROL_PIN);
        break;
      case '6':
        serial_port->print(F("proximity_state: "));
        serial_port->println(S.proximity_state);
        break;
      default:
        menuMain();
        break;
    }
  }
