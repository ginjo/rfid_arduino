  // Serial Menu Class
  // Handles human-readable input and output between text-based serial port
  // and arduino application (classes, functions, data, et.c.).
  
  
  /*
  
  * Refactored Menu with these features:
  
  * Menu::run_mode is a shared static now.
  
  * Two more statics hold SW and HW serial-menu instances.
  
  * One static, Current, holds the 'chosen-one', or defaults to HW.
  
  * Menu::Begin() and Menu::Loop() make sub-calls to
  SW and HW instances (both) until once is 'chosen' (when user
  input triggers admin-mode 'run_mode == 1').
  
  * At that point, the static for Current is filled with the chosen instance.
  Other classes can access the Menu::Current or Menu::run_mode
  if necessary for admin-mode operations.
  
  */
  
  
  #include "menu.h"
  // This is here because it seems to avoid circular include,
  // which would happen if this was in menu.h.
  // This is apparently a legitimate C/C++ technique.
  #include "controller.h"


  // Instanciates the built-in reset function.
  // WARN: This does not work if placed in settings.h (where you would think it should be).
  // NOTE: This is a function pointer.
  void(* resetFunc) (void) = 0;



  /***  Static Vars & Funtions  ***/

  int Menu::run_mode = 0;
  
  Menu * Menu::Current;
  Menu * Menu::HW;
  Menu * Menu::SW;

  void Menu::Begin() {
    LOG(4, F("Menu::Begin()"), true);
    HW->begin();
    SW->begin();

    // NOTE: Menu::Current is set in checkSerialPort()
  }
  
  void Menu::Loop() {
    // Runs hardware-serial loop().
    if (!Current || Current == HW) {
      HW->loop();
    }

    // Runs software-serial loop().
    //if (!Current || Current == SW) {
    
    // FIX: This was to allow AT commands to the BT module, but it currently breaks the menus.
    //      Find a better way to poll input from SW while admining with HW.
    if (!Current || Current == SW || (digitalRead(BT_STATUS_PIN) == 1 && run_mode != 0)) {
      SW->loop();
    } 
  }


  /*** Constructors and Setup ***/

  Menu::Menu(Stream *stream_ref, Reader *_reader, const char _instance_name[]) :
    serial_port(stream_ref),
    reader(_reader),    
    previous_ms(0),
    admin_timeout(0),
    buff {},
    buff_index(0),
    selected_menu_item(-1),
    get_tag_from_scanner(0)
	{
		// Don't call .begin or Serial functions here, since this is too close to hardware init.
		// The hardware might not be initialized yet, at this point.
    // Call .begin from setup() function instead.
    //strlcpy(input_mode, "menu", sizeof(input_mode));
    strlcpy(instance_name, _instance_name, sizeof(instance_name));
	}
	
  void Menu::begin() {    
    updateAdminTimeout(S.admin_startup_timeout);
    resetInputBuffer();
    //resetStack(&Menu::menuMain);
    
    if (strcmp(instance_name, "HW") == 0) {
      /* If this is hard-serial instance, just listen for input. */
      readLineWithCallback(&Menu::menuSelectedMainItem);
    } else if (strcmp(instance_name, "SW") == 0) {
      /*
        If this is soft-serial instance, prints info,
        but only if soft-serial is connected (via BT).
      */
      
      if (digitalRead(BT_STATUS_PIN) == LOW) {
        serial_port->print(F("RFID admin console, "));
        serial_port->print(VERSION);
        serial_port->print(", ");
        serial_port->println(TIMESTAMP);
        serial_port->println();        
      }
      /*
       Calls menuMainPromp(""), which sets up a listener for input.
       passing an empty string will hide the default menuMain prompt string,
       but will still create the ":" or ">" prompt and push the necessary functions
       to the stack.

       Actually, we don't want a prompt or any output if this is SW menu starting up.
       But we still need to create the listener/callback, so input will be processed.
      */      
      //menuMainPrompt("");
      readLineWithCallback(&Menu::menuSelectedMainItem);
    }
 	}


  /***  Control  ***/

  void Menu::loop() {
    MU_PRINT(6, F("MENU LOOP BEGIN: "), false); MU_PRINT(6, instance_name, true);

    // Disables switch output if active admin mode (assummed if admin_timeout equals the main setting).
    // TODO: This should probably call something like Controller::outputoff().
    if (run_mode == 1 && admin_timeout == S.admin_timeout) { digitalWrite(OUTPUT_SWITCH_PIN, 0); }
    
    if (! Current || Current == this) adminTimeout();

    // TODO: Re-enable this after decoupling from readLine (which should only care about completed buff).
    //       Really? Is this todo still relevant?
    //checkSerialPort();
    
    call();
    MU_PRINT(6, F("MENU LOOP END"), true);
  }

  // Checks timer for admin timeout and reboots or enters run_mode 0 if true.
  void Menu::adminTimeout() {
    unsigned long current_ms = millis();
    unsigned long elapsed_ms = current_ms - previous_ms;
    
    //  LOG(4, F("adminTimeout() run_mode, admin_timeout, now, previous_ms: "));
    //  LOG(4, run_mode); LOG(4, " ");
    //  LOG(4, admin_timeout); LOG(4, " ");
    //  LOG(4, current_ms); LOG(4, " ");
    //  Serial.println(previous_ms);

    if ( elapsed_ms/1000 > admin_timeout || run_mode == 0 ) {
      exitAdmin();
    }
  }
  
  // Starts, restarts, resets admin with timeout.
  void Menu::updateAdminTimeout(unsigned long seconds) {
    if (admin_timeout != seconds) {
      MU_PRINT(5, F("updateAdminTimeout(): "), false);
      MU_PRINT(5, seconds, true);
    }
    
    admin_timeout = seconds;
    run_mode = 1;
    previous_ms = millis();
  }

  // Exits admin (and allows main Controller loop to run).
  void Menu::exitAdmin() {
    MU_PRINT(5, F("Menu::exitAdmin()"), true);
    resetInputBuffer();
    resetStack();
    if (run_mode != 0) {
      LOG(5, instance_name);
      LOG(5, F(" setting run_mode => 0"), true);
      LOG(4, F("Entering run mode"), true);
      LOG(4, "", true);

      run_mode = 0;
      FREERAM("exitAdmin()");
    }
  }




  /***  Input  ***/

  // Checks & reads data from serial port, until EOL is detected.
  //
  // Stops reading serial input after CR/LF is received.
  // Only starts reading in data again, after the buff
  // is reset (to null, with buff_index = 0).
  //
  // The resulting "line", as bufferReady() reports true,
  // will be any string, with ending \r or \n, terminated by null (0).
  // The final buff_index will point to the terminating null.
  //
  // All upstream/downstream functions should assume the resulting
  // line as described above.
  //
  void Menu::checkSerialPort() {
    if (strcmp(instance_name, "SW") == 0) {
      SoftwareSerial * sp = (SoftwareSerial*)serial_port;
      sp->listen();
      while (! sp->isListening()) delay(2);
      delay(get_tag_from_scanner ? 10 : 1);
    }
    
    if (serial_port->available()) {

      // If someone typed anything into this serial port,
      // and Current isn't already set,
      // make this instance the Current one.
      if (! Current) Current = this;

      // Always update the admin timeout when user inputs anything.
      updateAdminTimeout();

      // As soon as user types something (anything), disable get-tag-from-scanner.
      get_tag_from_scanner = 0;
      
      while (serial_port->available() && !bufferReady()) {
        char byt = serial_port->read();
        
        MU_PRINT(6, F("checkSerialPort() rcvd byte: "), false);
        MU_PRINT(6, (int)byt, true);

        // Escape key resets buffer and mimics Enter key, for data-entry abort.
        if ((int)byt == 27) {
          buff[0] = 10;
          buff[1] = 0;
          buff_index = 1;
          return;
        }
        
        if (Current == this) serial_port->write(byt);
        buff[buff_index] = byt;
        buff_index += 1;

        if ((int)byt == 13 || (int)byt == 10) {   // || (int)byt == 0) {
          MU_PRINT(6, F("checkSerialPort EOL indx: "), false); MU_PRINT(6, buff_index-1, true);
          //serial_port->println(F("\r\n"));
          //serial_port->println((char)10);
    
          // Adds string terminator to end of buff.
          buff[buff_index] = 0;
          
        } // end if
        
      } // end while
    } // end if
  }

  // Clears any data waiting on serial port, if any.
  void Menu::clearSerialPort() {
    MU_PRINT(6, F("Menu::clearSerialPort()"), true);
    while (serial_port->available()) serial_port->read();    
  }

  void Menu::resetInputBuffer() {
    MU_PRINT(6, F("Menu::resetInputBuffer()"), true);
    memset(buff, 0, INPUT_BUFFER_LENGTH);
    buff_index = 0;
    get_tag_from_scanner = 0;
  }
  
  bool Menu::bufferReady() {
    bool bool_result = false;
    
    MU_PRINT(6, "bufferReady? bytes: ", false);
    #ifdef MU_DEBUG
      for (uint8_t i=0; i < sizeof(buff); i++) {
        MU_PRINT(6, (int)buff[i], false); MU_PRINT(6, ",", false);
        if (buff[i] == 10 || buff[i] == 13) {
          bool_result = true;
          break;
        }
      }
      MU_PRINT(6, "", true);
    #endif

    if (bool_result) { MU_PRINT(6, F("Menu::bufferReady(): "), false); MU_PRINT(6, buff, true); }
    
    return bool_result;
  }

  void Menu::prompt(const char _message[], CB _cback, bool _read_tag) {
    
    if (_message[0]) {
      serial_port->print(_message);
      //serial_port->print(F(" "));
    }

    // This can only go BEFORE the ">" IF it doesn't call anything (just does set-up).
    // Otherwise it should go after the ">" (but then any errors will show up after the prompt).
    readLineWithCallback(_cback, _read_tag);

    serial_port->print(F(": "));
  }

  // TODO: Consider renaming this... maybe getInput()?
  void Menu::readLineWithCallback(CB cback, bool _read_tag) {
    MU_PRINT(6, F("Menu::readLineWithCallback()"), true);
    push(cback);
    push(&Menu::readLine);
    clearSerialPort();
    resetInputBuffer();
    if (_read_tag) get_tag_from_scanner = 1;
  }

  // Checks for bufferReady() and reacts by calling stack-callback.
  // Removes readLine() and callback from stack.
  //
  void Menu::readLine(void *dat) {
    getTagFromScanner();
    checkSerialPort();
    
    if (bufferReady()) {

      char input[INPUT_BUFFER_LENGTH];
      strlcpy(input, buff, sizeof(input));

      resetInputBuffer();
      
      // Removes readLine() from stack.
      pop();
      
      serial_port->println((char)10);
  
      // Calls callback, passing in buff (true removes callback from stack).
      call(input, true);

    } // end if
  } // readLine()

  // This should loop as long as get_tag_from_scanner == 1,
  // until reader has last_tag_read_id, then this will set buff
  // with reader's last_tag_read_id and set get_tag_from_scanner to 0.
  //
  // The tag reader needs to be checked and handled regardless
  // of whether or not typed input is available on the UI serial-port.
  //
  void Menu::getTagFromScanner() {
    if (get_tag_from_scanner) {
      reader->loop();
      if (reader->last_tag_read_id) {
        MU_PRINT(6, F("Menu::getTagFromScanner() found tag "), false); MU_PRINT(6, reader->last_tag_read_id, true);
        char str[9];
        sprintf(str, "%lu", reader->last_tag_read_id);
        strlcpy(buff, str, sizeof(buff));
        for (uint8_t i=0; i < sizeof(buff); i++) {
          if (buff[i] == 0) {
            buff[i] = 10;
            buff[i+1] = 0;
            buff_index = i+1;
            break;
          }
        }
        reader->last_tag_read_id = 0;
        reader->resetBuffer();
        get_tag_from_scanner = 0;
      }
    }
  }



  /***  Data  ***/
  
  // Converts byte (some kind of integer) to the integer represented
  // by the ascii character of byte. This only works for ascii 48-57.
  int Menu::byteToAsciiChrNum(const char byt) {
    MU_PRINT(6, F("Menu::byteToAsciiChrNum rcvd byte: "), false);
    MU_PRINT(6, (int)byt, true);
    //  MU_PRINT(" (");
    //  MU_PRINT(byt);
    //  MU_PRINTLN(")");

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

  void Menu::addTagString(void *dat) {
    char *str = (char*)dat;
    int result;
    
    if (str[0] == 13 || str[0] == 10 || str[0] == 0) {
      result = 1;
    } else {
      // TODO: Should probably validate the entire string of digits.
      // But... do it here? or do it in Controller class? or in Tags class?
      //result = Controller::AddTag(strtol(str, NULL, 10));
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

  void Menu::deleteTag(void *dat) {
    MU_PRINT(6, F("Menu::deleteTag()"), true);
    
    char *str = (char*)dat;
    int tag_index = strtol(str, NULL, 10);
    
    if (tag_index < 1 || str[0] == 13 || str[0] == 10 || tag_index >= TAG_LIST_SIZE) {
      serial_port->println(F("deleteTag() aborted"));
    } else {
      int rslt = Tags::TagSet.deleteTagIndex(tag_index-1);
      serial_port->print(F("DeleteTag() result: "));
      serial_port->println(rslt);
    }
    serial_port->println();
    menuListTags();
  }

  void Menu::deleteAllTags(void *dat) {
    char *str = (char*)dat;
    int input = (int)(str[0]);
    if (input == 'y' || input == 'Y') {
      Tags::TagSet.deleteAllTags();
    } else {
      serial_port->println(F("deleteAllTags() aborted"));
    }
    serial_port->println();
    menuListTags();
  }

  void Menu::updateSetting(void *dat) {
    char *str = (char*)dat;
    
    MU_PRINT(5, F("Menu::updateSetting(): "), false);
    MU_PRINT(5, selected_menu_item, false);
    MU_PRINT(5, ", ", false);
    //MU_PRINTLN((char *)buff);
    MU_PRINT(5, str, true);

    if (str[0] == 13 || str[0] == 10 || str[0] == 0) {
      LOG(4, F("updateSetting() aborted"), true);
    } else if (S.updateSetting(selected_menu_item, str)) {
      // Because we need this after updating any Menu settings
      // and there isn't a better place for this (yet?).
      updateAdminTimeout();          
    } else {
      LOG(2, F("updateSetting() call to S.updateSetting() failed"), true);
    }

    selected_menu_item = -1;
    menuSettings();
  }



  /***  Menu  ***/
  
  void Menu::menuMain(void *dat) {
    MU_PRINT(6, F("Menu::menuMain()"), true);
    serial_port->println(F("Menu"));
    serial_port->println(F("0. Exit"));
    serial_port->println(F("1. List tags"));
    serial_port->println(F("2. Add tag"));
    serial_port->println(F("3. Delete tag"));
    serial_port->println(F("4. Delete all tags"));
    serial_port->println(F("5. Settings"));
    serial_port->println(F("6. Show free memory"));
    serial_port->println(F("7. List readers"));
    serial_port->println(F("8. Restart"));
    serial_port->println(F("9. BT command"));
    
    serial_port->println("");

    menuMainPrompt();
  }

  // Resets stack and gives default main-menu prompt.
  void Menu::menuMainPrompt(const char str[]) { // See .h for default string.
    resetStack();
    prompt(str, &Menu::menuSelectedMainItem);
  }

  // Activates an incoming menu selection.
  // TODO: Figure out when we pop() the stack ?!? This methods should always pop() itself out of the stack.
  void Menu::menuSelectedMainItem(void *bytes) {
    MU_PRINT(6, F("menuSelectedMainItem rcvd data: "), false);
    MU_PRINT(6, (char*)bytes, true);

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

    MU_PRINT(6, F("menuSelectedMainItem bytes to num: "), false);
    MU_PRINT(6, selected_menu_item, true);
    
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
        menuDeleteAllTags();
        break;
      case 5:
        menuSettings();
        break;
      case 6:
        menuShowFreeMemory();
        break;
      case 7:
        menuListReaders();
        break;
      case 8:
        menuReboot();
        break;
      case 9:
        menuManageBT();
        break;
      default:
        menuMain();
        break;
    }
  } // menuSelectedMainItem

  // Lists tags for menu.
  void Menu::menuListTags(void *dat, CB cback) {
    MU_PRINT(6, F("Menu::menuListTags()"), true);
    serial_port->print(F("Tags, chksm 0x"));
    serial_port->print(Tags::TagSet.checksum, 16);
    serial_port->print(F(", size "));
    serial_port->println(sizeof(Tags::TagSet));
    
    for (int i = 0; i < TAG_LIST_SIZE; i ++) {
      if (Tags::TagSet.tag_array[i] > 0) {
        serial_port->print(i+1);
        serial_port->print(F(". "));
        serial_port->print(Tags::TagSet.tag_array[i]);
        serial_port->println("");
      }
    }
    serial_port->println("");

    if (cback) {
      char *str = (char*)dat;
      prompt(str, cback);
    } else {
      menuMainPrompt();
    } 
  }

  // Asks user for full tag,
  // sets mode to receive-text-line-from-serial
  void Menu::menuAddTag(void *dat) {
    MU_PRINT(6, F("Menu::menuAddTag()"), true);
    //get_tag_from_scanner = 1; // now handled by promt(,,true). See promt().
    prompt("Enter (or scan) a tag number (unsigned long) to store", &Menu::addTagString, true);
  }

  // Asks user for index of tag to delete from EEPROM.
  void Menu::menuDeleteTag(void *dat) {
    MU_PRINT(6, F("Menu::menuDeleteTag()"), true);
    //menuListTags();
    //prompt("Enter tag index to delete", &Menu::deleteTag);
    menuListTags((void*)"Enter tag index to delete", &Menu::deleteTag);
  }

  // Deletes all tags from EEPROM.
  void Menu::menuDeleteAllTags(void *dat) {
    MU_PRINT(6, F("Menu::menuDeleteAllTags()"), true);
    //prompt("Delete all tags [y/N]?", &Menu::deleteAllTags);
    menuListTags((void*)"Delete all tags [y/N]?", &Menu::deleteAllTags);
  }

  void Menu::menuShowFreeMemory() {
    MU_PRINT(6, F("Menu::menuShowFreeMemory()"), true);
    int ram = FreeRam();
    serial_port->print(F("Free Memory: "));
    serial_port->println(ram);
    serial_port->println();
    menuMainPrompt(); 
  }

  void Menu::menuReboot() {
    MU_PRINT(6, F("Menu::menuReboot()"), true);
    resetFunc();
  }

  void Menu::menuSettings(void *dat) {
    MU_PRINT(6, F("Menu::menuSettings()"), true);
    //selected_menu_item = NULL;
    selected_menu_item = -1;

    S.printSettings(serial_port);

    serial_port->println("");

    prompt("Select a setting to edit", &Menu::menuSelectedSetting);
  }

  // Handle selected setting.
  //void Menu::menuSelectedSetting(char bytes[]) {
  void Menu::menuSelectedSetting(void *input) {
    MU_PRINT(6, F("Menu::menuSelectedSetting()"), true);
    
    char *bytes = (char*)input;
    
    // MU_PRINT(F("menuSelectedSetting received bytes: "));
    // MU_PRINTLN((char *)bytes);

    selected_menu_item = strtol(bytes, NULL, 10);
    
    MU_PRINT(6, F("menuSelectedSetting set selected_menu_item to: "), false);
    MU_PRINT(6, selected_menu_item, true);

    // If user selected valid settings item.
    if (selected_menu_item > 0 && selected_menu_item <= SETTINGS_SIZE) {
      char setting_name[SETTINGS_NAME_SIZE], setting_value[SETTINGS_VALUE_SIZE];
      S.getSettingByIndex(selected_menu_item, setting_name, setting_value);
      serial_port->print(setting_name); serial_port->print(F(": "));
      serial_port->println(setting_value);
      prompt("Type a new value for setting", &Menu::updateSetting);
      
    // If user selects "0", return, or enter, then go back to main menu.
    } else if (bytes[0] == '0' || bytes[0] == '\r' || bytes[0] == '\n') {
      menuMain();
    } else {
      menuSettings();
    }
  } // menuSelectedSetting()

  // Lists readers for menu.
  void Menu::menuListReaders(void *dat) {
    MU_PRINT(6, F("Menu::menuListReaders()"), true);
    
    Reader::PrintReaders(serial_port);
    serial_port->println("");

    prompt("Select a reader", &Menu::menuSelectedReader);
  }

  void Menu::menuSelectedReader(void *input) {
    int selected_reader = (int)strtol((char *)input, NULL, 10);

    //LOG(4, F("input: ")); LOG(4, (char *)input, true);
    //LOG(4, F("selected_reader: ")); LOG(4, selected_reader, true);

    //MU_PRINT(F("menuSelectedReader set selected_reader to: "));
    //MU_PRINTLN(selected_reader);

    // If user selected valid reader.
    if (selected_reader > 0 && selected_reader <= READER_COUNT) {
      serial_port->print(F("Selected: "));
      serial_port->println(Reader::NameFromIndex(selected_reader));
      serial_port->println("");

      S.updateSetting(8, (char*)input);
    }

    serial_port->println("");
    
    if (selected_reader == 0 || (int)input == 13 || (int)input == 10) {
      menuMain();
    } else {
      menuListReaders();
    }
  }



  
  /*
   Experimental read/write AT commands for BT module.
   These look like they could be genericised, but I think
   it would not work, or would at least be very messy...

   These functions are specific to the convoluted back-and-forth
   between the two serial interfaces (HW, SW), and the logic
   is specific to the process of managing the BT (SW) port
   via the HW serial interface menu. 
  */
  
  void Menu::menuManageBT(void*) {
    if (digitalRead(BT_STATUS_PIN) == HIGH) {
      // Cleanup, just to be safe.
      SW->resetStack();
      SW->clearSerialPort();
      SW->resetInputBuffer();
      
      HW->prompt("Enter AT+ command", &Menu::menuSendAtCommand);
    } else {
      serial_port->println(F("N/A"));
      serial_port->println("");
      menuMain();
    }
  }

  void Menu::menuSendAtCommand(void *_input) {
    /* Remember that when you pass a char array to a function,
       you convert it to a pointer. Fortunately, strlen() knows
       how to get the length of the c-string that's pointed to (looks for NULL).
    */

    int len = strlen((char *)_input);
    char output[32] = {""};
    strncpy(output, (char *)_input, len-1);

    #ifdef MU_DEBUG
      HW->serial_port->print(F("Cmd: "));
      HW->serial_port->println(output);
      HW->serial_port->println("");
    #endif

    //SW->serial_port->println("AT+VERSION"); // this works.
    if (output[0] == 'A') {
      SW->serial_port->println(output);
      SW->readLineWithCallback(&Menu::menuPrintATResponse);
      
    } else {
      SW->resetStack();
      SW->clearSerialPort();
      SW->resetInputBuffer();

      HW->menuMain();
    }
  }

  void Menu::menuPrintATResponse(void *_input) {
    char *input = (char *)_input;
    HW->serial_port->println((char *)input);
    HW->serial_port->println("");

    HW->menuManageBT();
  }
  
