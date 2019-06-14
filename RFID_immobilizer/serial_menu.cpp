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

  // Instanciate the built-in reset function
  void(* resetFunc) (void) = 0;


  /*** Constructor and Setup ***/

  SerialMenu::SerialMenu(Stream *stream_ref, Led * _blinker) :
    serial_port(stream_ref),
    buff({}),
    buff_index(0),
    current_function(""),
    input_mode("menu"),
    blinker(_blinker),
    // See setAdminWithTimeout()
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
    //serial_port->println(F("SerialMenu Admin Console\r\n"));
    
    //showInfo();
    Serial.println(F("Starting SerialMenu"));

    serial_port->print(F("RFID Immobilizer admin console, "));
    serial_port->print(VERSION);
    serial_port->print(", ");
    serial_port->println(TIMESTAMP);
    serial_port->println("");
    
    setAdminWithTimeout(2);
    resetInputBuffer();
    
    setInputMode("char");
    setCallbackFunction("menuSelectedMainItem");
    
    //menuListTags();
	}


  /*** Looping Functions ***/

  void SerialMenu::loop() {
    //serial_port->println("SerialMenu::loop() calling serial_port->println()");

    // Disables switch output if active admin mode (assummed if admin_timeout equals the main setting).
    // TODO: Create a Switch object that handles all switch actions (start, stop, initial, cleanup, etc).
    if (run_mode == 1 && admin_timeout == S.admin_timeout) { digitalWrite(13, 0); }
    
    adminTimeout();
    checkSerialPort();
    runCallbacks();
  }

	// check serial_port every cycle
  //
  void SerialMenu::checkSerialPort() {
    if (serial_port->available()) {
      setAdminWithTimeout(); //(S.admin_timeout) // See header for default function values.
      
      //Serial.println(F("checkSerialPort() serial_port->available() is TRUE"));
      uint8_t byt = serial_port->read();
      
      //  Serial.print("checkSerialPort received byte: ");
      //  Serial.println(char(byt));
      //  Serial.print("checkSerialPort input_mode is: ");
      //  Serial.println(input_mode);
      
      if (matchInputMode("char")) {
        Serial.println(F("checkSerialPort() matchInputMode('char') is TRUE"));
        buff[0] = byt;

      } else if (matchInputMode("line")) {
        Serial.println(F("checkSerialPort() matchInputMode('line') is TRUE"));
        getLine(byt);
        
      } else {
        // last-resort default just writes byt to serial_port
        // this should not happen under normal circumstances
        Serial.println(F("checkSerialPort() user input with no matching input mode: "));
        serial_port->write(byt); serial_port->println("");
      }
     
    } // done with available serial_port input
  }

  // check for callbacks every cycle
  // TODO: I think enum's can be used here instead of matching strings.
  // Could then use 'switch' statement.
  void SerialMenu::runCallbacks() {
    // If completed input, ready for processing, is available.
    // Either a string (with a LF ending), of a char.
    if (inputAvailable()) {
      //Serial.print(F("runCallbacks() inputAvailableFor(): "));
      //Serial.println(inputAvailableFor());

      if (inputAvailable("menuAddTag")) {
        Serial.println(F("runCallbacks() inputAvailable for addTagString"));
        if (addTagString(buff)) { 
          menuListTags();
        } else {
          Serial.println(F("runCallbacks() call to addTagString(buff) failed"));
          menuMain();
        }

      } else if (inputAvailable("menuSelectedMainItem")) {
        Serial.println(F("runCallbacks() inputAvailable for menuSelectedMainItem"));
        menuSelectedMainItem(buff[0]);

      } else if (inputAvailable("menuSelectedSetting")) {
        Serial.println(F("runCallbacks() inputAvailable for menuSelectedSetting"));
        menuSelectedSetting(buff[0]);

      } else if (inputAvailable("updateSetting")) {
        Serial.print(F("runCallbacks() inputAvailable for updateSetting: "));
        Serial.print(selected_menu_item);
        Serial.print(", ");
        Serial.println((char *)buff);
        
        if (S.updateSetting(selected_menu_item, buff)) { 
          menuSettings();
        } else {
          Serial.println(F("runCallbacks() call to S.updateSetting() failed"));
          menuMain();
        }
        selected_menu_item = NULL;
      
      } else {
        Serial.println(F("runCallbacks() no condition was selected"));
        
        //  setInputMode("char");
        //  setCallbackFunction("");
        //  buff_index = 0;
        //  strncpy(buff, "", INPUT_BUFFER_LENGTH);

        setInputMode("char");
        setCallbackFunction("menuSelectedMainItem");
      }

      resetInputBuffer();
    }
  }


  /*** Menu and State Logic ***/

  void SerialMenu::getLine(uint8_t byt) {
    Serial.print(F("getLine() byt, buff: "));
    Serial.print(char(byt));
    Serial.print(", ");
    Serial.println((char *)buff);
    
    buff[buff_index] = byt;
    buff_index ++;
    serial_port->write(byt);
  
    if (int(byt) == 13 || int(byt) == 10) {
      serial_port->println("");
  
      serial_port->print(F("You entered: "));
      serial_port->println((char*)buff);
      serial_port->println("");

      buff_index = 0;
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

  // TODO: Find all of these and make sure they operate correctly,
  // since the recent changes have broken soft-coded links.
  // Update the soft-coded links to fix.
  
  // TODO: Maybe change this to setCallbackFunction()
  void SerialMenu::setCallbackFunction(char func[]) {
    strncpy(current_function, func, CURRENT_FUNCTION_LENGTH);

    Serial.print(F("setCallbackFunction() to: "));
    Serial.println(current_function);
  }

  bool SerialMenu::matchCurrentFunction(char func[]) {
    //  Serial.print(F("matchCurrentFunction() with: "));
    //  Serial.print(func);
    //  Serial.print(", ");
    //  Serial.println(current_function);
    
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

  int SerialMenu::byteToAsciiChrNum(char * byt) {
    char str[3]; // ascii only uses 3 digits in base-10.
    sprintf(str, "%c", byt); // convert the byte to ascii string.
    return strtol(str, NULL, 10); // convert the string of digits to int.
  }

  //bool SerialMenu::addTagString(uint8_t str[TAG_LENGTH]) {
  bool SerialMenu::addTagString(uint8_t str[]) {
    // Need to discard bogus tags... this kinda works,
    // but needs more failure conditions.
    // TODO: Make sure string consists of only numerics, no other characters.
    // OR, if it's a valid hex string, use that.
    //if (sizeof(buff)/sizeof(*buff) != 8 || buff[0] == 13) {
    //if (buff_index < (TAG_LENGTH -1) || buff[0] == 13) {
    if (str[0] == 13 || str[0] == 10) {
      //strncpy(input_mode, "menu", INPUT_MODE_LENGTH);
      //setInputMode("char");
      //setCallbackFunction("menuSelectedMainItem");
      //buff_index = 0;
      //resetInputBuffer();
      //serial_port->println("");
      return false;
    }
  
    serial_port->print(F("Tag entered: "));
    serial_port->println((char*)str);
    serial_port->println("");

    return addTagNum(strtol(str, NULL, 10));
  }
  
  // Adds tag number to tag list.
  // TODO: Make sure number to add is within bounds of 32-bit integer,
  // since that is as high as the tag ids will go: 4294967295 or 'FFFFFFFF'
  bool SerialMenu::addTagNum(unsigned long tag_num) {
    for (int i = 0; i < TAG_LIST_SIZE; i ++) {
      if (! tags[i] > 0) {
        tags[i] = tag_num;
        setCallbackFunction("");
        return true;
      }
    }

    // fails if didn't return from inner block
    return false;
  }

  void SerialMenu::resetInputBuffer() {
    strncpy(buff, "", INPUT_BUFFER_LENGTH);
    buff_index = 0;
  }

  // Starts, restarts, resets admin with timeout.
  void SerialMenu::setAdminWithTimeout(int seconds) {
    Serial.print(F("setAdminWithTimeout() seconds: "));
    Serial.println(seconds);
    
    admin_timeout = seconds;
    run_mode = 1;
    previous_ms = millis();
  }

  // Checks timer for admin timeout and reboots or enters run_mode 0 if true.
  void SerialMenu::adminTimeout() {
    unsigned long current_ms = millis();
    unsigned long elapsed_ms = current_ms - previous_ms;
    
    //  Serial.print("adminTimeout() run_mode, admin_timeout, now, previous_ms: ");
    //  Serial.print(run_mode); Serial.print(" ");
    //  Serial.print(admin_timeout); Serial.print(" ");
    //  Serial.print(current_ms); Serial.print(" ");
    //  Serial.println(previous_ms);

    if (run_mode == 0) { return; }
    if ( elapsed_ms/1000 > admin_timeout ) {
      exitAdmin();
    }
  }

  void SerialMenu::exitAdmin() {
    if (admin_timeout == 2) {
      Serial.println(F("\r\nSerialMenu setting run_mode to 0 'run'"));
      serial_port->println(F("Entering run mode\r\n"));
      blinker->off();
      run_mode = 0;
    } else {
      Serial.println(F("\r\nSerialMenu rebooting arduino"));
      serial_port->println(F("Exiting admin console\r\n"));
      delay(100);
      resetFunc();
    }
  }

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


  /*** Draw Menu Items and Log Messages ***/

  
  void SerialMenu::menuMain() {
    serial_port->println(F("Menu"));
    serial_port->println(F("0. Exit"));
    serial_port->println(F("1. List tags"));
    serial_port->println(F("2. Add tag"));
    serial_port->println(F("3. Delete tag"));
    serial_port->println(F("4. Show free memory"));
    serial_port->println(F("5. Settings"));
    serial_port->println("");

    resetInputBuffer(); // just to be safe, since it's the home position
    setInputMode("char");
    setCallbackFunction("menuSelectedMainItem");
  }

  // Activates an incoming menu selection.
  void SerialMenu::menuSelectedMainItem(uint8_t byt) {
    Serial.print(F("menuSelectedMainItem received byte: "));
    Serial.println(char(byt));

    selected_menu_item = byteToAsciiChrNum(&byt);
    
    switch (selected_menu_item) {
      // warn: a missing 'break' will allow
      // drop-thru to the next case.
      case 0:
        //serial_port->println(F("Exiting admin console\r\n\r\n"));
        setAdminWithTimeout(0);
        break;
      case 1:
        menuListTags();
        break;
      case 2:
        menuAddTag();
        break;
      case 3:
        menuDeleteTag();
        break;
      case 4:
        menuShowFreeMemory();
        break;
      case 5:
        menuSettings();
        break;
      default:
        menuMain();
        break;
    }
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
    
    setInputMode("char");
    setCallbackFunction("menuSelectedMainItem");
  }

  // Asks user for full tag,
  // sets mode to receive-text-line-from-serial,
  // stores received tag (with validation) using RFIDTags class.
  void SerialMenu::menuAddTag() {
    serial_port->println(F("Add Tag"));
    serial_port->print(F("Enter a tag number (unsigned long) to store: "));

    setInputMode("line");
    setCallbackFunction(__FUNCTION__);
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

  // TODO: Should this be moved to settings.cpp?
  void SerialMenu::menuSettings() {
    selected_menu_item = NULL;
    
    serial_port->println(F("Settings"));
    serial_port->println(F("0. Return to main menu: "));
    serial_port->print(F("1. TAG_LAST_READ_TIMEOUT: ")); serial_port->println(S.TAG_LAST_READ_TIMEOUT);
    serial_port->print(F("2. TAG_READ_SLEEP_INTERVAL: ")); serial_port->println(S.TAG_READ_SLEEP_INTERVAL);
    serial_port->print(F("3. READER_CYCLE_LOW_DURATION: ")); serial_port->println(S.READER_CYCLE_LOW_DURATION);
    serial_port->print(F("4. READER_CYCLE_HIGH_DURATION: ")); serial_port->println(S.READER_CYCLE_HIGH_DURATION);
    serial_port->print(F("5. READER_POWER_CONTROL_PIN: ")); serial_port->println(S.READER_POWER_CONTROL_PIN);
    serial_port->print(F("6. proximity_state: ")); serial_port->println(S.proximity_state);
    serial_port->print(F("7. admin_timeout: ")); serial_port->println(S.admin_timeout);
    serial_port->println("");

    setInputMode("char");
    setCallbackFunction("menuSelectedSetting");
  }

  // Handle selected setting.
  void SerialMenu::menuSelectedSetting(uint8_t byt) {
    Serial.print(F("menuSelectedSetting received byte: "));
    Serial.println((char)byt);

    // NOTE: This is an example of converting a byte
    // to the integer represented by the ascii character of the byte.
    // Ex: byte is 55, which is ascii '7', which is integer 7.
    // TODO: Should this be extracted to its own function?
    //char str[3]; // ascii only uses 3 digits in base-10.
    //sprintf(str, "%c", byt); // convert the byte to ascii string.
    //selected_menu_item = strtol(str, NULL, 10); // convert the string of digits to int.
    selected_menu_item = byteToAsciiChrNum(&byt);
    
    Serial.print(F("menuSelectedSetting set selected_menu_item to: "));
    Serial.println(selected_menu_item);

    // TODO: I'm pretty sure switch can only work with integers.
    switch (selected_menu_item) {
      // NOTE: A missing 'break' will allow
      // drop-thru to the next case.
      case 0:
        menuMain();
        //break;
        return;
      case 1:
        serial_port->print(F("TAG_LAST_READ_TIMEOUT: "));
        serial_port->println(S.TAG_LAST_READ_TIMEOUT);
        break;
      case 2:
        serial_port->print(F("TAG_READ_SLEEP_INTERVAL: "));
        serial_port->println(S.TAG_READ_SLEEP_INTERVAL);
        break;
      case 3:
        serial_port->print(F("READER_CYCLE_LOW_DURATION: "));
        serial_port->println(S.READER_CYCLE_LOW_DURATION);
        break;
      case 4:
        serial_port->print(F("READER_CYCLE_HIGH_DURATION: "));
        serial_port->println(S.READER_CYCLE_HIGH_DURATION);
        break;
      case 5:
        serial_port->print(F("READER_POWER_CONTROL_PIN: "));
        serial_port->println(S.READER_POWER_CONTROL_PIN);
        break;
      case 6:
        serial_port->print(F("proximity_state: "));
        serial_port->println(S.proximity_state);
        break;
      case 7:
        serial_port->print(F("admin_timeout: "));
        serial_port->println(S.admin_timeout);
        break;
      default:
        menuSettings();
        //break;
        return;
    }
    
    serial_port->print("Type a new value for setting: ");
    setInputMode("line");
    setCallbackFunction("updateSetting");
  }


  
