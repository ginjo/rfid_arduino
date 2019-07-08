// Serial Menu Class
// Handles human-readable input and output between text-based serial port
// and arduino application (classes, functions, data, et.c.).

// TODO: Create a function for menu option "Read Tag",
// that temporarily switches run_mode to 0, gathers a single tag,
// then returns run_mode to 1 so BTmenu can resume where it left off.
// This could allow a menu option for "Add tag from scanner",
// vs the current add-tag-from-keyboard menu option.

#include "serial_menu.h"

  //  // Gets free-memory, see https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
  //  // This should really go in a Utility class.
  //  // It is only here as a quick fix.    
  //  #ifdef __arm__
  //  // should use uinstd.h to define sbrk but Due causes a conflict
  //  extern "C" char* sbrk(int incr);
  //  #else  // __ARM__
  //  extern char *__brkval;
  //  #endif  // __arm__
  //  
  //  static int freeMemory() {
  //    char top;
  //  #ifdef __arm__
  //    return &top - reinterpret_cast<char*>(sbrk(0));
  //  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  //    return &top - __brkval;
  //  #else  // __arm__
  //    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
  //  #endif  // __arm__
  //  }
  

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


  /*** Constructors and Setup ***/

  SerialMenu::SerialMenu(Stream *stream_ref, Led * _blinker) :
    serial_port(stream_ref),

    // TODO: Initialize the rest of this class's vars. See .h file.
    input_mode("menu"),
    buff {},
    buff_index(0),
    current_function(""),
    poll_rfid(0),
    //tags {305441741, 2882343476, 2676915564}, // 1234ABCD, ABCD1234, 9F8E7D6C
    blinker(_blinker)
    
    // See updateAdminTimeout()
    //  run_mode(1), // 0=run, 1=admin
    //  admin_timeout(3), // seconds
    //  previous_ms(millis());
    
	{
		// Don't call .begin or Serial functions here, since this is too close to hardware init.
		// The hardware might not be initialized yet, at this point.
    // Call .begin from setup() function instead.
	}
	
	//void SerialMenu::begin(unsigned long baud) {
  void SerialMenu::begin() {
    //serial_port->println(F("SerialMenu Admin Console\r\n"));
    
    //showInfo();
    Serial.println(F("SerialMenu::begin() running"));

    serial_port->print(F("RFID Immobilizer admin console, "));
    serial_port->print(VERSION);
    serial_port->print(", ");
    serial_port->println(TIMESTAMP);
    serial_port->println();
    
    updateAdminTimeout(2);
    resetInputBuffer();
    
    //  setInputMode("char");
    //  setCallbackFunction("menuSelectedMainItem");
    prompt('l', "", "menuSelectedMainItem");
    
    //menuListTags();
	}


  /*** Looping Functions ***/

  void SerialMenu::loop() {
    //DPRINTLN("/*** MENU LOOP BEGIN ***/");
    //serial_port->println(F("SerialMenu::loop() calling serial_port->println()"));

    // Disables switch output if active admin mode (assummed if admin_timeout equals the main setting).
    // TODO: Create a Switch object that handles all switch actions (start, stop, initial, cleanup, etc).
    if (run_mode == 1 && admin_timeout == S.admin_timeout) { digitalWrite(S.OUTPUT_SWITCH_PIN, 0); }
    
    adminTimeout();
    checkSerialPort();
    runCallbacks();

    // For getting new tag from rfid reader.
    // TODO: Should this be encapsulated in a function?
    if (poll_rfid) {
      Rfid->pollReader();
      if (Rfid->tag_ready) {
        uint32_t tag_id = Rfid->reader->processTagData(Rfid->buff);
        sprintf(buff, "%lu", tag_id);
        Rfid->tag_ready = 0;
        poll_rfid = 0;
      }
    }
  }

	// check serial_port every cycle
  //
  void SerialMenu::checkSerialPort() {
    if (serial_port->available()) {
      while (serial_port->available()) {
        //Serial.println(F("checkSerialPort() serial_port->available() is TRUE"));
        char byt = serial_port->read();
        
        DPRINT(F("checkSerialPort() received byte: "));
        DPRINTLN(char(byt));
        //  DPRINT(F("checkSerialPort input_mode is: "));
        //  DPRINTLN(input_mode);
        
        if (matchInputMode("char")) {
          //DPRINTLN(F("checkSerialPort() matchInputMode('char') is TRUE"));
          buff[0] = byt;
  
        } else if (matchInputMode("line")) {
          //DPRINTLN(F("checkSerialPort() matchInputMode('line') is TRUE"));
          getLine(byt);
          
        } else {
          // last-resort default just writes byt to serial_port
          // this should not happen under normal circumstances
          Serial.println(F("checkSerialPort() user input with no matching input mode: "));
          serial_port->write(byt); serial_port->println("");
        }
      }

      updateAdminTimeout(); //(S.admin_timeout) // See header for default function values.
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
        DPRINTLN(F("runCallbacks() inputAvailable for menuAddTag"));
        addTagString(buff);
        menuListTags();

//      } else if (inputAvailable("menuDeleteAllTags")) {
//        DPRINTLN(F("runCallbacks() inputAvailable for menuDeleteAllTags"));
//        menuDeleteAllTags();
//        RFID::DeleteAllTags();

      } else if (inputAvailable("menuSelectedMainItem")) {
        DPRINTLN(F("runCallbacks() inputAvailable for menuSelectedMainItem"));
        menuSelectedMainItem(buff[0]);

      } else if (inputAvailable("menuSelectedSetting")) {
        DPRINTLN(F("runCallbacks() inputAvailable for menuSelectedSetting"));
        menuSelectedSetting(buff);

      } else if (inputAvailable("updateSetting")) {
        DPRINT(F("runCallbacks() inputAvailable for updateSetting: "));
        DPRINT(selected_menu_item);
        DPRINT(", ");
        DPRINTLN((char *)buff);
        
        if (S.updateSetting(selected_menu_item, buff)) {
          // Because we need this after updating any SerialMenu settings
          // and there isn't a better place for this (yet?).
          updateAdminTimeout();          
        } else {
          Serial.println(F("runCallbacks() call to S.updateSetting() failed"));
        }
        
        //selected_menu_item = NULL;
        selected_menu_item = -1;
        menuSettings();
      
      } else {
        Serial.println(F("runCallbacks() no condition was selected"));
        
        //  setInputMode("char");
        //  setCallbackFunction("");
        //  buff_index = 0;
        //  strncpy(buff, "", INPUT_BUFFER_LENGTH);

        //  setInputMode("char");
        //  setCallbackFunction("menuSelectedMainItem");
        prompt('l', "", "menuSelectedMainItem");
      }

      resetInputBuffer();
    }
  }


  /*** Menu and State Logic ***/

  // Builds a line of input in variable 'buff' until CR or LF is detected,
  // then resets buff_index to 0.
  void SerialMenu::getLine(char byt) {
    DPRINT(F("getLine() byt"));
    DPRINTLN(char(byt));
    
    buff[buff_index] = byt;
    buff_index ++;
    serial_port->write(byt);
  
    if (int(byt) == 13 || int(byt) == 10) {
      serial_port->println("\r\n");
  
      //  DPRINT(F("You entered: "));
      //  DPRINT((char*)buff);
      //  DPRINTLN("");

      // Adds string terminator to end of buff.
      buff[buff_index + 1] = 0;

      // Resets buff_index.
      buff_index = 0;
    }
  }

  // Set input mode to character or line.
  // NOTE: Migrating to line-mode for all functions.
  void SerialMenu::setInputMode(const char str[]) {
    // All input uses 'line' now, but option to use 'char' is still here.
    strncpy(input_mode, str, INPUT_MODE_LENGTH);
    
    DPRINT(F("setInputMode(): "));
    DPRINTLN(input_mode);
  }

  bool SerialMenu::matchInputMode(const char mode[]) {
    //  Serial.print(F("matchInputMode() mode, input_mode: "));
    //  Serial.print(mode);
    //  Serial.print(", ");
    //  Serial.println(input_mode);
    
    return strcmp(mode, input_mode) == 0;
  }

  // TODO: Find all of these and make sure they operate correctly,
  // since the recent changes have broken soft-coded links.
  // Update the soft-coded links to fix.
  
  void SerialMenu::setCallbackFunction(const char *func_name) {
    //  DPRINT(F("setCallbackFunction() received func_name: ("));
    //  DPRINTLN(func_name));
    
    if (func_name[0] != 0) {
      DPRINT(F("setCallbackFunction(): "));
      DPRINTLN(func_name);
      strncpy(current_function, func_name, CURRENT_FUNCTION_LENGTH);
    }
  }

  bool SerialMenu::matchCurrentFunction(const char func[]) {
    //  Serial.print(F("matchCurrentFunction() with: "));
    //  Serial.print(func);
    //  Serial.print(", ");
    //  Serial.println(current_function);
    
    return strcmp(current_function, func) == 0;
  }

  bool SerialMenu::inputAvailable() {
    return buff_index == 0 && buff[0] > 0;
  }
  
  bool SerialMenu::inputAvailable(const char func[]) {
    return inputAvailable() && matchCurrentFunction(func);
  }

  const char * SerialMenu::inputAvailableFor() {
    if(inputAvailable()) {
      return current_function;
    } else {
      return "";
    }
  }

  // Converts byte (some kind of integer) to the integer represented
  // by the ascii character of byte. This only works for ascii 48-57.
  int SerialMenu::byteToAsciiChrNum(const char byt) {
    DPRINT(F("SerialMenu::byteToAsciiChrNum received byte: "));
    DPRINT(char(byt));
    DPRINT(" (");
    DPRINT(byt);
    DPRINTLN(")");
    
    if (byt < 48 || byt > 57) {
      return '0';
    }
    //char str[2]; // need room for null-terminator?
    //sprintf(str, "%c", byt); // convert the byte to ascii string.
    //char * str = &byt; 
    //return (int)strtol(str, NULL, 10); // convert the string of digits to int.
    // I don't think we need to use the str... it should the exactly the same as byt.
    return (int)strtol(&byt, NULL, 10);
  }

  //bool SerialMenu::addTagString(char str[TAG_LENGTH]) {
  int SerialMenu::addTagString(char str[]) {
    if (str[0] == 13 || str[0] == 10 || str[0] == 0) {
      serial_port->println(F("addTagString() failed: invalid tag"));
      return 1;
    }

    // TODO: Should probably validate the entire string of digits.
  
    serial_port->print(F("Tag entered: "));
    serial_port->println((char*)str);
    serial_port->println("");
    
    //return addTagNum(strtol(str, NULL, 10));
    int rslt = RFID::AddTag(strtol(str, NULL, 10));
    serial_port->print(F("AddTag() result: "));
    serial_port->println(rslt);
    return rslt;
  }
  
  // Adds tag number to tag list.
  // TODO: Make sure number to add is within bounds of 32-bit integer,
  // since that is as high as the tag ids will go: 4294967295 or 'FFFFFFFF'
  //  bool SerialMenu::addTagNum(unsigned long tag_num) {
  //    for (int i = 0; i < TAG_LIST_SIZE; i ++) {
  //      if ( tags[i] <= 0) {
  //        tags[i] = tag_num;
  //        setCallbackFunction("");
  //        return true;
  //      }
  //    }
  //
  //    // fails if didn't return from inner block
  //    return false;
  //  }

  void SerialMenu::resetInputBuffer() {
    memset(buff, 0, INPUT_BUFFER_LENGTH);
    buff_index = 0;
  }

  // Starts, restarts, resets admin with timeout.
  void SerialMenu::updateAdminTimeout(uint32_t seconds) {
    if (admin_timeout != seconds) {
      Serial.print(F("updateAdminTimeout() seconds: "));
      Serial.println(seconds);
    }
    
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
    if (true || admin_timeout == 2) {
      Serial.println(F("\r\nSerialMenu setting run_mode to 0 'run'"));
      serial_port->println(F("Entering run mode\r\n"));
      blinker->Off();
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
    //serial_port->println(F("serial_port is active!"));
    Serial.print(F("SerialMenu::setup input_mode: "));
    Serial.println(input_mode);
    Serial.print(F("SerialMenu::setup buff_index: "));
    Serial.println(buff_index);
    Serial.print(F("SerialMenu::setup RFID::Tags[2]: "));
    Serial.println(RFID::Tags[2]);
  }

  void SerialMenu::prompt(const char _input_mode, const char _message[], const char _callback_function[]) {
    if (_input_mode == 'l') {
      setInputMode("line");
    } else if (_input_mode == 'c') {
      setInputMode("char");
    }

    if (_callback_function[0] != 0) {
      setCallbackFunction(_callback_function);
    }

    if (_message[0] != 0) {
      DPRINT(F("prompt() printing _message: "));
      DPRINTLN(_message);
      
      serial_port->print(_message);
      serial_port->print(" ");
    }

    //resetInputBuffer(); // WARN: This might break some things, being called here.
    serial_port->print("> ");
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
    serial_port->println(F("6. Delete all tags"));
    
    serial_port->println("");

    //  resetInputBuffer(); // just to be safe, since it's the home position
    //  setInputMode("char");
    //  setCallbackFunction("menuSelectedMainItem");
    prompt('l', "Select a menu option", "menuSelectedMainItem");
  }

  // Activates an incoming menu selection.
  void SerialMenu::menuSelectedMainItem(char byt) {
    DPRINT(F("menuSelectedMainItem received byte: "));
    DPRINT(char(byt));
    DPRINT(" (");
    DPRINT(byt);
    DPRINTLN(")");

    selected_menu_item = byteToAsciiChrNum(byt);
    DPRINT(F("menuSelectedMainItem converted byte: "));
    DPRINTLN(selected_menu_item);

    // Note that we're still using byt and character ascii codes in
    // the switch statement, instead of using selected_menu_item and
    // literal integers. This gives us more range of what we can compare.
    // For example, if the user hits Enter key, we don't have a
    // selected_menu_item index to correspond with that (since the ascii
    // Enter character doesn't represent a specific integer).
    switch (byt) {
      // warn: a missing 'break' will allow
      // drop-thru to the next case.
      case '0':
        //serial_port->println(F("Exiting admin console\r\n\r\n"));
        updateAdminTimeout(0);
        break;
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
      case '6':
        menuDeleteAllTags();
        break;
      default:
        menuMain();
        break;
    }
  } // menuSelectedMainItem

  // Lists tags for menu.
  void SerialMenu::menuListTags() {
    serial_port->println(F("Tags"));
    //serial_port->println((char*)tags);
    // TODO: Move the bulk of this to RFIDTags?
    for (int i = 0; i < TAG_LIST_SIZE; i ++) {
      if (RFID::Tags[i] > 0) {
        serial_port->print(i);
        serial_port->print(". ");
        serial_port->print(RFID::Tags[i]);
        serial_port->println("");
      }
    }
    serial_port->println("");
    
    //  setInputMode("char");
    //  setCallbackFunction("menuSelectedMainItem");
    prompt('\0', "", "menuSelectedMainItem");
  }

  // Asks user for full tag,
  // sets mode to receive-text-line-from-serial,
  // stores received tag (with validation) using RFIDTags class.
  void SerialMenu::menuAddTag() {
    poll_rfid = 1;
    prompt('l', "Enter a tag number (unsigned long) to store", __FUNCTION__);
  }

  // Asks user for index of tag to delete from EEPROM.
  void SerialMenu::menuDeleteTag() {
    serial_port->println(F("Delete Tag"));
    serial_port->println("");
    prompt();
  }

  // Asks user for index of tag to delete from EEPROM.
  void SerialMenu::menuDeleteAllTags() {
    serial_port->println(F("Delete all Tags"));
    serial_port->println("");
    RFID::DeleteAllTags();
    //prompt();
    menuListTags();
  }

  void SerialMenu::menuShowFreeMemory() {
    serial_port->println(F("Free Memory: n/a"));
    //serial_port->println(freeMemory());
    //serial_port->println();
    prompt();
  }

  // TODO: Should this be moved to settings.cpp?
  void SerialMenu::menuSettings() {
    //selected_menu_item = NULL;
    selected_menu_item = -1;
    serial_port->print("Settings, chksm 0x");
    serial_port->print(S.getChecksum(), 16);
    serial_port->print(", size ");
    serial_port->println(sizeof(S));

    // Prints out all settings in tabular format.
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
      S.displaySetting(n, output);
      serial_port->println(output);
    }

    serial_port->println();

    //  setInputMode("line");
    //  setCallbackFunction("menuSelectedSetting");
    prompt('l', "Select a setting to edit", "menuSelectedSetting");
  }

  // Handle selected setting.
  void SerialMenu::menuSelectedSetting(char bytes[]) {
    // DPRINT(F("menuSelectedSetting received bytes: "));
    // DPRINTLN((char *)bytes);

    selected_menu_item = strtol(bytes, NULL, 10);
    
    DPRINT(F("menuSelectedSetting set selected_menu_item to: "));
    DPRINTLN(selected_menu_item);

    if (selected_menu_item > 0 && selected_menu_item < SETTINGS_SIZE) {
      char setting_name[SETTINGS_NAME_SIZE], setting_value[SETTINGS_VALUE_SIZE];
      S.getSettingByIndex(selected_menu_item, setting_name, setting_value);
      serial_port->print(setting_name); serial_port->print(": ");
      serial_port->println(setting_value);
      prompt('l', "Type a new value for setting", "updateSetting");
    } else if (bytes[0] == '0') {
      menuMain();
    } else {
      menuSettings();
    }
  } // menuSelectedSetting()

  
