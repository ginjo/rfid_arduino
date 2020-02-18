  // Serial Menu Class
  // Handles human-readable input and output between text-based serial port
  // and arduino application (classes, functions, data, et.c.).
  
  
  /*
  
  * Refactored Menu with these features:
  
  * Menu::RunMode is a shared static now.
  
  * Two more statics hold SW and HW serial-menu instances.
  
  * One static, Current, holds the 'chosen-one', or defaults to HW.
  
  * Menu::Begin() and Menu::Loop() make sub-calls to
  SW and HW instances (both) until once is 'chosen' (when user
  input triggers admin-mode 'RunMode == 1').
  
  * At that point, the static for Current is filled with the chosen instance.
  Other classes can access the Menu::Current or Menu::RunMode
  if necessary for admin-mode operations.
  
  */
  
  
  #include "menu.h"
  #include "logger.h"
  #include "global.h"
  // This is here because it seems to avoid circular include,
  // which would happen if this was in menu.h.
  // This is apparently a legitimate C/C++ technique.
  // See https://www.acodersjourney.com/top-10-c-header-file-mistakes-and-how-to-fix-them/
  #include "controller.h"


  // Instantiates the built-in reset function.
  // WARN: This does not work if placed in settings.h (where you would think it should be).
  //       It causes multiple-definition errors if run there.
  // NOTE: This is a function pointer.
  void(* resetFunc) (void) = 0;



  /***  Static Vars & Funtions  ***/

  int Menu::RunMode = 1; // Starts up in mode 1 'Menu'.

  // These provide the "definition" of these static vars. See .h file for declarations.
  Menu * Menu::Current = nullptr;
  Menu * Menu::HW = nullptr;
  Menu * Menu::SW = nullptr;


  menu_item_T const Menu::MenuItems[MENU_ITEMS_SIZE] PROGMEM = { 
    { "Exit", &Menu::updateAdminTimeout, (void*)0},// nullptr },

    { "Add tag", &Menu::menuAddTag, nullptr},  // nullptr },
    { "Delete tag", &Menu::menuDeleteTag, nullptr},  // nullptr },
    { "Delete all tags", &Menu::menuDeleteAllTags, nullptr},  //, nullptr },
    { "List tags", &Menu::menuListTags, nullptr},  // nullptr },
    { "List Readers", &Menu::menuListReaders, nullptr},  // nullptr },
    { "Show free mem", &Menu::menuShowFreeMemory, nullptr},  // nullptr },
    { "BT command", &Menu::menuManageBT, nullptr},  // nullptr },    
    { "Settings", &Menu::menuSettings, nullptr},  // nullptr },
    { "Save settings", &Menu::menuSaveSettings, nullptr},  // nullptr },
    { "Restart", &Menu::menuReboot, nullptr},  // nullptr }
  };


  void Menu::Begin() {
    LOG(4, F("Menu.Begin()"), true);
    HW->begin();
    SW->begin();
    
    // NOTE: Menu::Current is set in checkSerialPort()
  }
  
  void Menu::Loop() {
    // Runs hardware-serial loop().
    if (!Current || Current == HW) {
      HW->loop();
    }

    // Runs software-serial loop.
    //
    // The last condition allows AT commands to be sent from the HW menu to the BT module.
    // TODO: Is there a better way to poll input from SW while admining with HW?
    //
    //if (!Current || Current == SW || (digitalRead(BT_STATUS_PIN) == 1 && RunMode != 0)) {  // works well.
    if (Current != HW || (digitalRead(BT_STATUS_PIN) == 1 && RunMode != 0)) {                // also works well but shorter code.
      SW->loop();
    }
  }


  /*** Constructors ***/

  Menu::Menu(Stream *stream_ref, Reader *_reader, const char _instance_name[]) :
    serial_port(stream_ref),
    reader(_reader),    
    previous_ms(0UL),
    admin_timeout(0UL),
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


  /***  Controll  ***/
  
  void Menu::begin() {    
    updateAdminTimeout((void*)S.admin_startup_timeout);
    resetInputBuffer();
    clearSerialPort(); // recently added, dunno if needed.
    
    //if (strcmp(instance_name, "HW") == 0) {
    if (this == HW) {
      /* If this is hard-serial instance, just listen for input. */
      readLineWithCallback(&Menu::menuSelectedMainItem);
    //} else if (strcmp(instance_name, "SW") == 0) {
    } else if (this == SW) {
      /*
        If this is soft-serial instance, prints info,
        but only if soft-serial is connected (via BT).

        Then sets up the login challenge.
      */

      // NOTE: Don't use Logger here, since this needs
      // to be printed in cases where Logger would skip it.
      // So basically, always print this if BT is connected.
      //
      if (digitalRead(BT_STATUS_PIN) == LOW) {
        serial_port->print(F("RFID admin console, "));
        serial_port->print(VERSION);
        serial_port->print(", ");
        serial_port->println(TIMESTAMP);
        serial_port->println();
      }

      menuLogin();
    }
 	}

  void Menu::loop() {
    MU_LOG(6, F("MENU LOOP BEGIN "), false); MU_LOG(6, instance_name, true);

    // IF Current not assigned yet, checks tag reader periodically, and exits admin if tag read.
    if (millis() % 1000 < 500 && !Current && this == SW) {
      reader->power_cycle_high_duration_override = 1UL; // Resets override to 1 on each pass, so it never decays.
      reader->loop();
      if (reader->tag_last_read_id && !get_tag_from_scanner) {
        exitAdmin();
      }

      return;
    }

    // Disables switch output if active admin mode (assummed if admin_timeout equals the main setting).
    // TODO: This should probably call something like Controller::outputoff().
    if (RunMode == 1 && admin_timeout == S.admin_timeout) digitalWrite(OUTPUT_SWITCH_PIN, LOW);
    
    if (! Current || Current == this) adminTimeout();

    // TODO: Re-enable this after decoupling from readLine (which should only care about completed buff).
    //       Really? Is this still relevant?
    //checkSerialPort();
    
    call();
    MU_LOG(6, F("MENU LOOP END"), true);
  }

  // Checks timer for admin timeout and reboots or enters RunMode 0 if true.
  void Menu::adminTimeout() {
    unsigned long current_ms = millis();
    unsigned long elapsed_ms = current_ms - previous_ms;
    
    MU_LOG(6, F("adminTimeout() mode, adm-tmout, now, prev-ms: "), false);
    MU_LOG(6, RunMode, false); MU_LOG(6, " ", false);
    MU_LOG(6, admin_timeout, false); MU_LOG(6, " ", false);
    MU_LOG(6, current_ms, false); MU_LOG(6, " ", false);
    MU_LOG(6, previous_ms, true);

    if ( elapsed_ms/1000 > admin_timeout || RunMode == 0 ) {
      exitAdmin();
    }
  }
  
  // Starts, restarts, resets admin with timeout.
  //void Menu::updateAdminTimeout(unsigned long seconds) {
  void Menu::updateAdminTimeout(void *intgr) {
    unsigned long seconds = (unsigned long)intgr;
    if (admin_timeout != seconds) {
      MU_LOG(5, F("updateAdminTimeout() "), false);
      MU_LOG(5, seconds, true);
    }
    
    admin_timeout = seconds;
    RunMode = 1;
    previous_ms = millis();
  }

  // Exits admin (and allows main Controller loop to run).
  void Menu::exitAdmin() {
    MU_LOG(5, F("Menu.exitAdmin()"), true);
    resetInputBuffer();
    resetStack();
    if (RunMode != 0) {
      LOG(5, instance_name);
      LOG(5, F(" setting RunMode to 0"), true);
      LOG(4, F("Entering run mode"), true);
      LOG(4, "", true);

      // Divides debug pin recognition into two sessions: Admin and Run.
      // If you startup with TempDebug, it will reset to false upon entering run-mode,
      // unless you are holding the DEBUG_PIN low.
      TempDebug = (digitalRead(DEBUG_PIN) == LOW);

      RunMode = 0;
      
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
    //if (strcmp(instance_name, "SW") == 0) {
    if (this == SW) {
      SoftwareSerial * sp = (SoftwareSerial*)serial_port;

      if (! sp->isListening()) {
        sp->listen();
        LOG(6, F("Menu listen"), true);
        delay(100);
      }
      while (! sp->isListening()) {
        ; //delay(2);
      }
    }
    
    if (serial_port->available()) {

      // If someone typed anything into this serial port,
      // and Current isn't already set,
      // make this instance the Current one.
      if (! Current) {
        Current = this;

        // This is new, an attempt to mimic selecting manage-bt,
        // which cleans up the first beep of a multi-beep signal
        // after tag-scanning during add-tag.
        // This should only run on the SW menu object, if 'this'
        // is the HW object.
        //
        if (this == HW) {
          SW->resetStack();
          SW->clearSerialPort();
          SW->resetInputBuffer();
        }
        
        Beeper->mediumBeep(1);
      }

      // Always update the admin timeout when user inputs anything.
      updateAdminTimeout();

      // As soon as user types something (anything), disable get-tag-from-scanner.
      get_tag_from_scanner = 0;
      
      while (serial_port->available() && !bufferReady()) {
        char byt = serial_port->read();
        
        MU_LOG(6, F("checkSerialPort() rcvd byte: "), false);
        MU_LOG(6, (int)byt, true);

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
          MU_LOG(6, F("checkSerialPort EOL indx: "), false); MU_LOG(6, buff_index-1, true);
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
    MU_LOG(6, F("Menu.clearSerialPort()"), true);
    while (serial_port->available()) serial_port->read();    
  }

  void Menu::resetInputBuffer() {
    MU_LOG(6, F("Menu.resetInputBuffer()"), true);
    memset(buff, 0, INPUT_BUFFER_LENGTH);
    buff_index = 0;
    get_tag_from_scanner = 0;
  }
  
  bool Menu::bufferReady() {
    bool bool_result = false;
    
    MU_LOG(6, "bufferReady? bytes: ", false);
    for (uint8_t i=0; i < sizeof(buff); i++) {
      MU_LOG(6, (int)buff[i], false); MU_LOG(6, ",", false);
      if (buff[i] == 10 || buff[i] == 13) {
        bool_result = true;
        break;
      }
    }
    MU_LOG(6, "", true);

    if (bool_result) { MU_LOG(6, F("Menu.bufferReady(): "), false); MU_LOG(6, buff, true); }
    
    return bool_result;
  }

  
  // So you can store prompt text in progmem: prompt_P(PSTR("whatever"))
  //
  // Properly copies progmem string to local var.
  // See https://forum.arduino.cc/index.php?topic=50197.0
  // See https://forum.arduino.cc/index.php?topic=158375.0
  // See https://forum.arduino.cc/index.php?topic=556489.0
  //
  // Also suggested:
  //  char *buff;
  //  buff=(char *)malloc(strlen_P(str)+1);
  //  strcpy_P(buff, *str);
  //  prompt(buff, _cback, _read_tag);
  //  free(buff);
  //
  void Menu::prompt_P(const char *str, CB _cback, bool _read_tag) {
    char buff[strlen_P(str)+1];
    strcpy_P(buff, str);
    prompt(buff, _cback, _read_tag);
  }

  // Displays a prompt with string, sending eventual user input to callback.
  void Menu::prompt(const char *_message, CB _cback, bool _read_tag) {
    
    if (_message[0] && !(this == SW && digitalRead(BT_STATUS_PIN) == HIGH)) {
      serial_port->print(_message);
      serial_port->print(F(": "));
    }

    // This can only go BEFORE the ">" IF it doesn't call anything (just does set-up).
    // Otherwise it should go after the ">" (but then any errors will show up after the prompt).
    readLineWithCallback(_cback, _read_tag);

    //serial_port->print(F(": "));
  }

  // TODO: Consider renaming this... maybe getInput()?
  void Menu::readLineWithCallback(CB cback, bool _read_tag) {
    MU_LOG(6, F("Menu.readLineWithCallback()"), true);
    push(cback);
    push(&Menu::readLine);
    clearSerialPort();
    resetInputBuffer();
    if (_read_tag) {
      // not sure if the resetBuffer() is necessary.
      reader->resetBuffer();
      get_tag_from_scanner = 1;
    }
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
  // until reader has tag_last_read_id, then this will set buff
  // with reader's tag_last_read_id and set get_tag_from_scanner to 0.
  //
  // The tag reader needs to be checked and handled regardless
  // of whether or not typed input is available on the UI serial-port.
  //
  void Menu::getTagFromScanner() {
    if (get_tag_from_scanner) {
      reader->power_cycle_high_duration_override = 1UL; // Resets override to 1 on each pass, so it never decays.
      reader->loop();
      if (reader->tag_last_read_id) {
        MU_LOG(6, F("Menu.getTagFromScanner() found tag "), false); MU_LOG(6, reader->tag_last_read_id, true);
        char str[10]; // TODO: This was 9, shouldn't it be 12 (10 digits + LF + null)? Why do we need this var?
        sprintf(str, "%lu", reader->tag_last_read_id);
        strlcpy(buff, str, sizeof(buff));
        // Adds missing LF to end of buff (and pushes null to next byte).
        for (uint8_t i=0; i < sizeof(buff); i++) {
          if (buff[i] == 0) {
            buff[i] = 10;
            buff[i+1] = 0;
            buff_index = i+1;
            break;
          }
        }
        reader->tag_last_read_id = 0;
        reader->resetBuffer();
        get_tag_from_scanner = 0;
      }
    }
  }



  /***  Data  ***/
  
  // Converts byte (some kind of integer) to the integer represented
  // by the ascii character of byte. This only works for ascii 48-57.
  int Menu::byteToAsciiChrNum(const char byt) {
    MU_LOG(6, F("Menu.byteToAsciiChrNum rcvd byte: "), false);
    MU_LOG(6, (int)byt, true);
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

    // Warning: a missing 'break' will allow drop-thru to the next case.
    switch (result) {
      case 0:
        serial_port->println(F("success"));
        break;
      case 1:
        serial_port->println(F("invalid tag-id string"));
        break;
      case 2:
        serial_port->println(F("tag list is full"));
        break;
      case 3:
        serial_port->println(F("tag is duplicate"));
        break;
      case -1:
        serial_port->println(F("unknown error"));
        break;
    }

    if (result == 0) {
      Beeper->mediumBeep(1);
    } else if (result > 0) {
      Beeper->shortBeep(result);
    } else if (result < 0) {
      Beeper->shortBeep(5);
    }
    
    serial_port->println("");
    resetInputBuffer();
    //return result;
    menuListTags();
  }

  void Menu::deleteTag(void *dat) {
    MU_LOG(6, F("Menu.deleteTag()"), true);
    
    char *str = (char*)dat;
    int tag_index = strtol(str, NULL, 10);
    
    if (tag_index < 1 || str[0] == 13 || str[0] == 10 || tag_index > TAG_LIST_SIZE) {
      serial_port->println(F("deleteTag() aborted"));
    } else {
      int rslt = Tags::TagSet.deleteTagIndex(tag_index-1);
      serial_port->print(F("deleteTag() result: "));
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
    
    MU_LOG(5, F("Menu.updateSetting(): "), false);
    MU_LOG(5, selected_menu_item, false);
    MU_LOG(5, ", ", false);
    //MU_PRINTLN((char *)buff);
    MU_LOG(5, str, true);

    if (str[0] == 13 || str[0] == 10 || str[0] == 0) {
      LOG(5, F("updateSetting() aborted"), true);
    } else if (S.updateSetting(selected_menu_item, str)) {
      // Because we need this after updating any Menu settings
      // and there isn't a better place for this (yet?).
      updateAdminTimeout();
      //serial_port->println(F("Don't forget to Save Settings!"));
    } else {
      LOG(2, F("Menu call to S.updateSetting() failed"), true);
    }

    selected_menu_item = -1;
    menuSettings();
  }


  /***  Menu  ***/
  
  void Menu::menuMain(void *dat) {
    MU_LOG(6, F("menuMain()"), true);

    // Extracts MenuItems[n] from PROGMEM and prints index and item name.
    for (int n=0; n < MENU_ITEMS_SIZE; n++) {
      menu_item_T item = {};
      memcpy_P(&item, &MenuItems[n], sizeof(menu_item_T));

      char out[MENU_ITEM_NAME_SIZE + 5] = "";
      sprintf_P(out, PSTR("%2i  %s"), n, item.name);
      serial_port->println(out);
    }

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
    MU_LOG(6, F("menuSelectedMainItem rcvd data: "), false);
    MU_LOG(6, (char*)bytes, true);

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

    MU_LOG(6, F("menuSelectedMainItem bytes to num: "), false);
    MU_LOG(6, selected_menu_item, true);

    if (selected_menu_item >= 0 && selected_menu_item < MENU_ITEMS_SIZE) {

      menu_item_T item = {};
      memcpy_P(&item, &MenuItems[selected_menu_item], sizeof(item));
  
      // MAGIC: Sets up and calls pointer-to-member-function on 'this',
      // passing in item.data (which is nullptr in most cases).
      CB fp = item.fp;
      (this->*fp)(item.data);

    } else {
      menuMain();
    }
    
  } // menuSelectedMainItem

  // Lists tags for menu. See .h file for explanation of default args.
  void Menu::menuListTags(void *dat) {menuListTags(dat, nullptr);}
  void Menu::menuListTags(void *dat, CB cback ) {
    MU_LOG(6, F("menuListTags()"), true);

    Tags::TagSet.printTags(serial_port);
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
    MU_LOG(6, F("menuAddTag()"), true);
    //get_tag_from_scanner = 1; // now handled by promt(,,true). See promt().
    prompt_P(PSTR("Enter or scan a tag number to store"), &Menu::addTagString, true);
  }

  // Asks user for index of tag to delete from EEPROM.
  void Menu::menuDeleteTag(void *dat) {
    MU_LOG(6, F("menuDeleteTag()"), true);
    //menuListTags();
    //prompt("Enter tag index to delete", &Menu::deleteTag);
    menuListTags((void*)"Enter tag index to delete", &Menu::deleteTag);
  }

  // Deletes all tags from EEPROM.
  void Menu::menuDeleteAllTags(void *dat) {
    MU_LOG(6, F("menuDeleteAllTags()"), true);
    //prompt("Delete all tags [y/N]?", &Menu::deleteAllTags);
    menuListTags((void*)"Delete all tags [y/N]?", &Menu::deleteAllTags);
  }

  void Menu::menuShowFreeMemory(void *dat) {
    MU_LOG(6, F("menuShowFreeMemory()"), true);
    int ram = FreeRam();
    serial_port->print(F("Free Memory: "));
    serial_port->println(ram);
    serial_port->println();
    menuMainPrompt(); 
  }

  void Menu::menuReboot(void *dat) {
    MU_LOG(6, F("menuReboot()"), true);
    resetFunc();
  }


  void Menu::menuSettings(void *dat) {
    MU_LOG(6, F("menuSettings()"), true);
    //selected_menu_item = NULL;
    selected_menu_item = -1;

    S.printSettings(serial_port);

    serial_port->println("");

    prompt_P(PSTR("Select a setting to edit"), &Menu::menuSelectedSetting);
  }

  // Handle selected setting.
  void Menu::menuSelectedSetting(void *input) {
    MU_LOG(6, F("menuSelectedSetting()"), true);
    
    char *bytes = (char*)input;
    
    // MU_PRINT(F("menuSelectedSetting received bytes: "));
    // MU_PRINTLN((char *)bytes);

    selected_menu_item = strtol(bytes, NULL, 10);
    
    MU_LOG(6, F("menuSelectedSetting set selected_menu_item to: "), false);
    MU_LOG(6, selected_menu_item, true);

    // If user selected valid settings item.
    if (selected_menu_item > 0 && selected_menu_item <= SETTINGS_SIZE) {
      char setting_name[SETTINGS_NAME_SIZE] = {}, setting_value[SETTINGS_VALUE_SIZE] = {};
      S.getSettingByIndex(selected_menu_item, setting_name, setting_value);
      serial_port->print(setting_name); serial_port->print(F(": "));
      serial_port->println(setting_value);
      prompt_P(PSTR("Type a new value for setting"), &Menu::updateSetting);
      
    // If user selects "0", return, or enter, then go back to main menu.
    } else if (bytes[0] == '0' || bytes[0] == '\r' || bytes[0] == '\n') {
      menuMain();
    } else {
      menuSettings();
    }
  } // menuSelectedSetting()

  
  void Menu::menuSaveSettings(void *dat) {
    prompt_P(PSTR("Save settings? [Y/n]"), &Menu::menuHandleSaveSettings);
  }
  
  // Handle save settings.
  void Menu::menuHandleSaveSettings(void *input) {
    MU_LOG(6, F("menuHandleSaveSettings()"), true);
    
    char *text = (char*)input;
    if (text[0] == 'Y' || text[0] == 'y' || text[0] == '1' || text[0] == 13 || text[0] == 10) {
      LOG(5, F("Calling S.save()"), true);
      S.save();
    }
    
    menuMain();
  }


  // Lists readers for menu.
  void Menu::menuListReaders(void *dat) {
    MU_LOG(6, F("menuListReaders()"), true);
    
    Reader::PrintReaders(serial_port);
    serial_port->println("");

    prompt_P(PSTR("Select a reader"), &Menu::menuSelectedReader);
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
   via the HW serial interface menu object. 
  */
  
  void Menu::menuManageBT(void *dat) {
    if (digitalRead(BT_STATUS_PIN) == HIGH) {
      // Cleanup, just to be safe.
      SW->resetStack();
      SW->clearSerialPort();
      SW->resetInputBuffer();
      
      HW->prompt_P(PSTR("Enter AT+ command"), &Menu::menuSendAtCommand);
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
  
  void Menu::menuLogin(void *dat) {
      resetStack();
      clearSerialPort();
      resetInputBuffer();
      
      prompt_P(PSTR("Password"), &Menu::menuProcessLogin);  	
  }
  
  void Menu::menuProcessLogin(void *_input) {
    
    // NOTE: This will chomp the trailing return (good), since strlen doesn't count the null terminator.
    unsigned long len = strlen((char*)_input);
    char cstr[len];
    strlcpy(cstr, (char*)_input, len); 

    #ifdef MU_DEBUG
      int pw_login_log_level = 5;
      
      if (LogLevel() >= pw_login_log_level) {
        LOG(pw_login_log_level, F("PWs"), true);
        
        //LOG(pw_login_log_level, F("pw:")); LOG(pw_login_log_level, len, false); LOG(pw_login_log_level, F(":")); LOG(pw_login_log_level, cstr, true);
        for (int n=0; n < (int)len-1; n++) {
          LOG(pw_login_log_level, (int)(cstr)[n], false); LOG(pw_login_log_level, F("."), false);
        }
        LOG(pw_login_log_level, F(""), true);

        //LOG(pw_login_log_level, F("PW:")); LOG(pw_login_log_level, strlen(S.admin_password), false); LOG(pw_login_log_level, F(":")); LOG(pw_login_log_level, S.admin_password, true);
        for (int n=0; n < (int)strlen(S.admin_password); n++) {
          LOG(pw_login_log_level, (int)(S.admin_password)[n], false); LOG(pw_login_log_level, F("."), false);
        }
        LOG(pw_login_log_level, F(""), true);
      }
    #endif
    
    //char pw[] = "2928";
    
    if (strcmp(cstr, S.admin_password) == 0) {
  		menuMain();
  	} else {
  		menuLogin();
  	}
  }
  
