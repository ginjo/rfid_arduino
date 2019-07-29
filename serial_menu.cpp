  // Serial Menu Class
  // Handles human-readable input and output between text-based serial port
  // and arduino application (classes, functions, data, et.c.).
  
  
  /*
  
  * Refactored SerialMenu with these features:
  
  * SerialMenu::run_mode is a shared static now.
  
  * Two more statics hold SW and HW serial-menu instances.
  
  * One static, Current, holds the ''chosen-one'', or defaults to HW.
  
  * SerialMenu::Begin() and SerialMenu::Loop() make sub-calls to
  SW and HW instances (both) until once is ''chosen'' (when user
  input triggers admin-mode 'run_mode == 1').
  
  * At that point, the static for Current is filled with the chosen instance.
  Other classes can access the SerialMenu::Current or SerialMenu::run_mode
  if necessary for admin-mode operations.
  
  */
  
  
  // TODO: Create a function for menu option "Read Tag",
  // that temporarily switches run_mode to 0, gathers a single tag,
  // then returns run_mode to 1 so BTmenu can resume where it left off.
  // This could allow a menu option for "Add tag from scanner",
  // vs the current add-tag-from-keyboard menu option.
  
  #include "serial_menu.h"
  // This is here because it seems to avoid circular include,
  // which would happen if this was in serial_menu.h.
  // This is apparently a legitimate C/C++ technique.
  #include "rfid.h"


  // Instanciates the built-in reset function.
  // WARN: This does not work if placed in settings.h (where you would think it should be).
  void(* resetFunc) (void) = 0;



  /***  Static Vars & Funtions  ***/

  int SerialMenu::run_mode = 0;
  
  SerialMenu * SerialMenu::Current;
  SerialMenu * SerialMenu::HW;
  SerialMenu * SerialMenu::SW;

  void SerialMenu::Begin() {
    Serial.println(F("SerialMenu::begin()"));
    HW->begin();
    SW->begin();

    // NOTE: SerialMenu::Current is set in checkSerialPort() or in exitAdmin()
  }
  
  void SerialMenu::Loop() {

    // Runs hardware-serial loop().
    if (!Current || Current == HW) {
      // TODO: Consider moving this delay to the beginning of loop(),
      // since it's the same as in the SW version below.
      delay(HW->get_tag_from_scanner ? 25 : 1);
      DPRINTLN("Menu::HW->loop()");
      HW->loop();
    }

    // Runs software-serial loop().
    if (!Current || Current == SW) {
      SoftwareSerial * sp = (SoftwareSerial*)SW->serial_port;
      sp->listen();
      while (! sp->isListening()) delay(15);
      delay(SW->get_tag_from_scanner ? 25 : 1);
      DPRINTLN("Menu::SW->loop()");
      SW->loop();
    }
    
  } // main Loop()


  /*** Constructors and Setup ***/

  SerialMenu::SerialMenu(Stream *stream_ref, Reader *_reader, Led * _blinker, const char _instance_name[]) :
    serial_port(stream_ref),
    reader(_reader),
    
    // TODO: Initialize the rest of this class's vars. See .h file.
    //run_mode(0), // moved to static member.
    previous_ms(0),
    admin_timeout(0),
    //input_mode("menu"),
    buff {},
    buff_index(0),
    //current_function(""),
    selected_menu_item(-1),
    get_tag_from_scanner(0),
    blinker(_blinker)
	{
		// Don't call .begin or Serial functions here, since this is too close to hardware init.
		// The hardware might not be initialized yet, at this point.
    // Call .begin from setup() function instead.
    //strlcpy(input_mode, "menu", sizeof(input_mode));
    strlcpy(instance_name, _instance_name, sizeof(instance_name));
	}
	
  void SerialMenu::begin() {    
    updateAdminTimeout(2);
    resetInputBuffer();
    //resetStack(&SerialMenu::menuMain);
    
    // If this is hardware instance, don't print info.
    if (strcmp(instance_name, "HW") != 0) {
      serial_port->print(F("RFID Immobilizer admin console, "));
      serial_port->print(VERSION);
      serial_port->print(", ");
      serial_port->println(TIMESTAMP);
      serial_port->println();
      // passing an empty string will hide the default menuMain prompt string,
      // but will still create the ">" prompt and push the necessary functions
      // to the stack.
      menuMainPrompt("");
    } else {
      readLineWithCallback(&SerialMenu::menuSelectedMainItem);
    }

 	}



  /***  Control  ***/

  void SerialMenu::loop() {
    //DPRINTLN(F("/*** MENU LOOP BEGIN ***/"));
    //serial_port->println(F("SerialMenu::loop() calling serial_port->println()"));

    // TODO: Move this into serial_menu, controled by callback stack.
    // TODO: Disable this here as soon as doing so won't affect any other calls.
    //getTagFromScanner();

    // Disables switch output if active admin mode (assummed if admin_timeout equals the main setting).
    // TODO: Create a Switch object that handles all switch actions (start, stop, initial, cleanup, etc).
    if (run_mode == 1 && admin_timeout == S.admin_timeout) { digitalWrite(S.OUTPUT_SWITCH_PIN, 0); }
    
    adminTimeout();

    // TODO: Re-enable this after decoupling from readLine (wich should only care about completed buff).
    //checkSerialPort();
    
    call();
  }

  // Checks timer for admin timeout and reboots or enters run_mode 0 if true.
  void SerialMenu::adminTimeout() {
    unsigned long current_ms = millis();
    unsigned long elapsed_ms = current_ms - previous_ms;
    
    //  Serial.print(F("adminTimeout() run_mode, admin_timeout, now, previous_ms: "));
    //  Serial.print(run_mode); Serial.print(" ");
    //  Serial.print(admin_timeout); Serial.print(" ");
    //  Serial.print(current_ms); Serial.print(" ");
    //  Serial.println(previous_ms);

    //if (run_mode == 0) { return; }
    if ( elapsed_ms/1000 > admin_timeout || run_mode == 0 ) {
      exitAdmin();
    }
  }
  
  // Starts, restarts, resets admin with timeout.
  void SerialMenu::updateAdminTimeout(unsigned long seconds) {
    if (admin_timeout != seconds) {
      Serial.print(F("updateAdminTimeout(): "));
      Serial.println(seconds);
    }
    
    admin_timeout = seconds;
    run_mode = 1;
    previous_ms = millis();
  }

  // Exits admin and starts main RFID/proximity loop.
  // Alternatively, reboots the arduino.
  //
  void SerialMenu::exitAdmin() {
    DPRINTLN("Menu::exitAdmin()");
    if (true || admin_timeout == 2) {
      Serial.println(F("\r\nMenu setting run_mode to 0 'run'"));
      serial_port->println(F("Entering run mode\r\n"));
      blinker->Off();
      run_mode = 0;
      resetInputBuffer();
      resetStack();
      FreeRam("exitAdmin()");
    } // else {
      //    Serial.println(F("\r\nMenu rebooting arduino"));
      //    serial_port->println(F("Exiting admin console\r\n"));
      //    delay(100);
      //    resetFunc();
    //  }
  }




  /***  Input  ***/

  // Checks & reads data from serial port, until EOL is detected.
  //
  // Stop reading serial input after CR/LF
  // is received. Only start reading in data again, after the buff
  // is reset (to null, with buff_index = 0).
  //
  void SerialMenu::checkSerialPort() {
    if (serial_port->available()) {

      // If someone typed anything into this serial port,
      // make this instance the Current one.
      Current = this;

      // Always update the admin timeout when user inputs anything.
      updateAdminTimeout();
      
      while (serial_port->available() && !bufferReady()) {
        char byt = serial_port->read();
        serial_port->write(byt);
        
        DPRINT(F("checkSerialPort() rcvd byte: "));
        DPRINTLN((int)byt);
        
        buff[buff_index] = byt;
        buff_index += 1;

        if ((int)byt == 13 || (int)byt == 10 || (int)byt == 0) {
          DPRINT("readLine EOL indx: "); DPRINTLN(buff_index-1);
          //serial_port->println(F("\r\n"));
          serial_port->println((char)10);
    
          // Adds string terminator to end of buff.
          buff[buff_index] = 0;
          
        } // end if
        
      } // end while
    } // end if
  }

  // Clears any data waiting on serial port, if any.
  void SerialMenu::clearSerialPort() {
    DPRINTLN("Menu::clearSerialPort()");
    while (serial_port->available()) serial_port->read();    
  }

  void SerialMenu::resetInputBuffer() {
    DPRINTLN("Menu::resetInputBuffer()");
    memset(buff, 0, INPUT_BUFFER_LENGTH);
    buff_index = 0;
    get_tag_from_scanner = 0;
  }
  
  bool SerialMenu::bufferReady() {
    bool rslt = (
      buff_index > 0 &&
      buff[0] != 0 && (
        buff[buff_index-1] == 13 ||
        buff[buff_index-1] == 10 ||
        buff[buff_index-1] == 0  ||
        buff[buff_index]   == 13 ||
        buff[buff_index]   == 10 ||
        buff[buff_index]   == 0
      )
    );

    if (rslt) { DPRINTLN("Menu::bufferReady(): "); DPRINTLN(buff); }
    
    return rslt;
  }

  void SerialMenu::prompt(const char _message[], CB _cback) {
    
    if (_message[0]) {
      serial_port->print(_message);
      serial_port->print(" ");
    }

    // This can only go BEFORE the ">" IF it doesn't call anything (just does set-up).
    // Otherwise it should go after the ">" (but then any errors will show up after the prompt).
    readLineWithCallback(_cback);
    
    serial_port->print("> ");
  }

  void SerialMenu::readLineWithCallback(CB cback, bool _read_tag) {
    DPRINTLN("Menu::readLineWithCallback()");
    push(cback);
    push(&SerialMenu::readLine);
    clearSerialPort();
    resetInputBuffer();
  }

  // Checks for bufferReady() and reacts by calling stack-callback.
  // Removes readLine() and callback from stack.
  //
  void SerialMenu::readLine(void *dat) {
    getTagFromScanner();
    checkSerialPort();
    
    if (bufferReady()) {

      char input[INPUT_BUFFER_LENGTH];
      strlcpy(input, buff, sizeof(input));

      resetInputBuffer();
      
      // Removes readLine() from stack.
      pop();

      serial_port->println();
  
      // Calls callback, passing in buff (true removes callback from stack).
      call(input, true);

    } // end if
  } // readLine()

  // This should loop as long as get_tag_from_scanner == 1,
  // until reader has current_tag_id, then this will set buff
  // with reader's current_tag_id and set get_tag_from_scanner to 0.
  //
  // The tag reader needs to be checked and handled regardless
  // of whether or not typed input is available on the UI serial-port.
  //
  void SerialMenu::getTagFromScanner() {
    if (get_tag_from_scanner) {
      reader->loop();
      if (reader->current_tag_id) {
        DPRINT("Menu::getTagFromScanner() found tag "); DPRINTLN(reader->current_tag_id);
        char str[9];
        sprintf(str, "%lu", reader->current_tag_id);
        strlcpy(buff, str, sizeof(buff));
        for (uint8_t i=0; i < sizeof(buff); i++) {
          if (buff[i] == 0) {
            buff_index = i;
            break;
          }
        }
        reader->current_tag_id = 0;
        reader->resetBuffer();
        get_tag_from_scanner = 0;
      }
    }
  }



  /***  Data  ***/
  
  // Converts byte (some kind of integer) to the integer represented
  // by the ascii character of byte. This only works for ascii 48-57.
  int SerialMenu::byteToAsciiChrNum(const char byt) {
    DPRINT(F("Menu::byteToAsciiChrNum rcvd byte: "));
    DPRINTLN((int)byt);
    //  DPRINT(" (");
    //  DPRINT(byt);
    //  DPRINTLN(")");

    // Returns -1 if ascii chr of byte is not a numeric.
    if (byt < 48 || byt > 57) {
      //return '0';
      return -1;
    }
    //char str[2]; // need room for null-terminator?
    //sprintf(str, "%c", byt); // convert the byte to ascii string.
    //char * str = &byt; 
    //return (int)strtol(str, NULL, 10); // convert the string of digits to int.
    // I don't think we need to use the str... it should the exactly the same as byt.
    return (int)strtol(&byt, NULL, 10);
  }

  void SerialMenu::addTagString(void *dat) {
    char *str = (char*)dat;
    int result;
    
    if (str[0] == 13 || str[0] == 10 || str[0] == 0) {
      result = 1;
    } else {
      // TODO: Should probably validate the entire string of digits.
      // But... do it here, or do it in RFID class?
      // (or in Tags class, when we develop that)?
      //result = RFID::AddTag(strtol(str, NULL, 10));
      result = Tags::TagSet.addTag(strtol(str, NULL, 10));
    }

    serial_port->print(F("Tag entered: "));
    serial_port->println((char*)str);
    serial_port->print(F("AddTag() result: "));
    serial_port->print(result); serial_port->print(", ");

    switch (result) {
      case 0:
        serial_port->print(F("success"));
        break;
      case 1:
        serial_port->print(F("aborted, invalid tag-id string"));
        break;
      case 2:
        serial_port->print(F("failed, tag list is full"));
        break;
      case 3:
        serial_port->print(F("failed, tag is duplicate"));
        break;
      case -1:
        serial_port->print(F("failed, unknown error"));
        break;
    }
    
    serial_port->println(); serial_port->println();
    resetInputBuffer();
    //return result;
    menuListTags();
  }

  //int SerialMenu::deleteTag(char str[]) {
  void SerialMenu::deleteTag(void *dat) {
    DPRINTLN("Menu::deleteTag()");
    
    char *str = (char*)dat;
    int tag_index = strtol(str, NULL, 10);
    
    if (str[0] == 13 || str[0] == 10 || tag_index >= TAG_LIST_SIZE) {
      serial_port->println(F("DeleteTag() aborted"));
      serial_port->println();
      //return 1;
    } else {
      //int rslt = RFID::DeleteTagIndex(tag_index);
      int rslt = Tags::TagSet.deleteTagIndex(tag_index);
      serial_port->print(F("DeleteTag() result: "));
      serial_port->println(rslt);
      serial_port->println();
      //return rslt;
    }
    menuListTags();
  }

  void SerialMenu::updateSetting(void *dat) {
    char *str = (char*)dat;
    
    DPRINT(F("Menu::updateSetting(): "));
    DPRINT(selected_menu_item);
    DPRINT(", ");
    //DPRINTLN((char *)buff);
    DPRINTLN(str);
    
    //if (S.updateSetting(selected_menu_item, buff)) {
    if (S.updateSetting(selected_menu_item, str)) {
      // Because we need this after updating any SerialMenu settings
      // and there isn't a better place for this (yet?).
      updateAdminTimeout();          
    } else {
      Serial.println(F("updateSetting() call to S.updateSetting() failed"));
    }

    selected_menu_item = -1;
    menuSettings();
  }




  /***  Menu  ***/
  
  void SerialMenu::menuMain(void *dat) {
    DPRINTLN("Menu::menuMain()");
    serial_port->println(F("Menu"));
    serial_port->println(F("0. Exit"));
    serial_port->println(F("1. List tags"));
    serial_port->println(F("2. Add tag"));
    serial_port->println(F("3. Delete tag"));
    serial_port->println(F("4. Show free memory"));
    serial_port->println(F("5. Settings"));
    serial_port->println(F("6. Delete all tags"));
    
    serial_port->println("");

    menuMainPrompt();
  }

  // Resets stack and gives default main-menu prompt.
  void SerialMenu::menuMainPrompt(const char str[]) { // See .h for default string.
    resetStack();
    prompt(str, &SerialMenu::menuSelectedMainItem);
  }

  // Activates an incoming menu selection.
  // TODO: Figure out when we pop() the stack ?!? This methods should always pop() itself out of the stack.
  void SerialMenu::menuSelectedMainItem(void *bytes) {
    DPRINT(F("menuSelectedMainItem rcvd bytes: "));
    DPRINTLN((char*)bytes);

    // pop(); // I think the prompt() that set up this callback
    // will automatically pop() it, so we shouldn't need this.

    // If first chr is not a numeric, set selected_menu_item to
    // something that will trigger default response.
    // TODO: This could all be encapsulated in byteToAsciiChrNum() function.
    //
    if (byteToAsciiChrNum(((int*)bytes)[0]) == -1) {
      selected_menu_item = -1;
    } else {
      selected_menu_item = strtol((char*)bytes, NULL, 10);
    }

    DPRINT(F("menuSelectedMainItem bytes to num: "));
    DPRINTLN(selected_menu_item);
    
    switch (selected_menu_item) {
      // warn: a missing 'break' will allow
      // drop-thru to the next case.
      case 0:
        //serial_port->println(F("Exiting admin console\r\n\r\n"));
        updateAdminTimeout(0);
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
      case 6:
        menuDeleteAllTags();
        break;
      default:
        menuMain();
        break;
    }
  } // menuSelectedMainItem

  // Lists tags for menu.
  void SerialMenu::menuListTags(void *dat) {
    DPRINTLN("Menu::menuListTags()");
    serial_port->print(F("Tags, chksm 0x"));
    serial_port->print(Tags::TagSet.checksum, 16);
    serial_port->print(F(", size "));
    serial_port->println(sizeof(Tags::TagSet));
    
    for (int i = 0; i < TAG_LIST_SIZE; i ++) {
      //if (RFID::Tags[i] > 0) {
      if (Tags::TagSet.tag_array[i] > 0) {
        serial_port->print(i);
        serial_port->print(F(". "));
        //serial_port->print(RFID::Tags[i]);
        serial_port->print(Tags::TagSet.tag_array[i]);
        serial_port->println("");
      }
    }
    serial_port->println("");

    menuMainPrompt(); 
  }

  // Asks user for full tag,
  // sets mode to receive-text-line-from-serial
  // TODO: Saving this till last: Need to update this to use stack.
  void SerialMenu::menuAddTag(void *dat) {
    DPRINTLN("Menu::menuAddTag()");
    SerialMenu::get_tag_from_scanner = 1;
    prompt("Enter (or scan) a tag number (unsigned long) to store", &SerialMenu::addTagString);
  }

  // Asks user for index of tag to delete from EEPROM.
  void SerialMenu::menuDeleteTag(void *dat) {
    DPRINTLN("Menu::menuDeleteTag()");
    prompt("Enter tag index to delete", &SerialMenu::deleteTag);
  }

  // Deletes all tags from EEPROM.
  void SerialMenu::menuDeleteAllTags(void *dat) {
    DPRINTLN("Menu::menuDeleteAllTags()");
    serial_port->println(F("Delete all Tags"));
    serial_port->println("");

    Tags::TagSet.deleteAllTags();
    menuListTags();
  }

  void SerialMenu::menuShowFreeMemory() {
    DPRINTLN("Menu::menuShowFreeMemory()");
    serial_port->println(F("Free Memory: n/a"));
    //serial_port->println(freeMemory());
    //serial_port->println();
    menuMainPrompt(); 
  }

  void SerialMenu::menuSettings(void *dat) {
    DPRINTLN("Menu::menuSettings()");
    //selected_menu_item = NULL;
    selected_menu_item = -1;
    serial_port->print(F("Settings, chksm 0x"));
    serial_port->print(S.calculateChecksum(), 16);
    serial_port->print(F(", size "));
    serial_port->println(sizeof(S));

    // Prints out all settings in tabular format.
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
      S.displaySetting(n, output);
      serial_port->println(output);
    }

    serial_port->println();

    prompt("Select a setting to edit", &SerialMenu::menuSelectedSetting);
  }

  // Handle selected setting.
  //void SerialMenu::menuSelectedSetting(char bytes[]) {
  void SerialMenu::menuSelectedSetting(void *input) {
    DPRINTLN("Menu::menuSelectedSetting()");
    
    char *bytes = (char*)input;
    
    // DPRINT(F("menuSelectedSetting received bytes: "));
    // DPRINTLN((char *)bytes);

    selected_menu_item = strtol(bytes, NULL, 10);
    
    DPRINT(F("menuSelectedSetting set selected_menu_item to: "));
    DPRINTLN(selected_menu_item);

    // If user selected valid settings item.
    if (selected_menu_item > 0 && selected_menu_item < SETTINGS_SIZE) {
      char setting_name[SETTINGS_NAME_SIZE], setting_value[SETTINGS_VALUE_SIZE];
      S.getSettingByIndex(selected_menu_item, setting_name, setting_value);
      serial_port->print(setting_name); serial_port->print(F(": "));
      serial_port->println(setting_value);
      prompt("Type a new value for setting", &SerialMenu::updateSetting);
      
    // If user selects "0", return, or enter, then go back to main menu.
    } else if (bytes[0] == '0' || bytes[0] == '\r' || bytes[0] == '\n') {
      menuMain();
    } else {
      menuSettings();
    }
  } // menuSelectedSetting()


  
