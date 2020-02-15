  #include "reader.h"
  #include "logger.h"


  /*  Static vars & functions  */

  const char Reader::Name[READER_NAME_LENGTH] = "base";

  void Reader::PrintReaders(Stream *sp) {
    sp->println(F("Readers"));
    for (int n=1; n <= READER_COUNT; n++) {
      int output_length = READER_NAME_LENGTH + 9;
      char output[output_length] = "";
      snprintf_P(output, output_length, PSTR("%2i  %s  %s"), n, (S.default_reader == (int8_t)n ? "*" : " "), NameFromIndex(n));
      sp->println(output);
    }
  }
  
  // Defines Reader Constructor
  Reader::Reader(uint8_t _raw_tag_length, uint8_t _id_begin, uint8_t _id_end, bool _control_logic) :
    raw_tag_length(_raw_tag_length),
    id_begin(_id_begin),
    id_end(_id_end),
    power_control_logic(_control_logic),

    /* From Controller */
    buff {},
    buff_index(0UL),
    current_ms(millis()),
    tag_last_read_ms(0UL),
    last_power_cycle_ms(0UL),
    power_cycle_high_duration_override(0UL),
    cycle_low_finish_ms(0UL),
    cycle_high_finish_ms(0UL),
    tag_last_read_id(0UL),
    serial_port(NULL) // experimental, so that serial_port is initialized
        
  {
    LOG(4, F("Loading reader: "));
    LOG(4, name(), true);
  }



  /*  Instance functions  */

  char *Reader::name() {return (char *)Name;}

  uint32_t Reader::processTagData(uint8_t[]) {
    RD_LOG(2, F("Called processTagData() on base Reader class"), true);
    return 0UL;
  }

  /***  From Controller  ***/

  void Reader::loop() {

    if (! serial_port->isListening()) {
      serial_port->listen();
      LOG(6, F("Reader listen"), true);
      delay(100); // Gives better chances of tag being read when adding tags by scanning.
    }
    while (! serial_port->isListening()) {
      ; //delay(2);
    }

    RD_LOG(6, F("READER LOOP BEGIN "), false);
    
    current_ms = millis();
    RD_LOG(6, current_ms, true);

    // TODO: 1. Make sure upstream caller can handle powerCycleHighDuration().
    //       2. Check if these vars have a function() equivalent. Sort them out if so.
    cycle_low_finish_ms = (uint32_t)(last_power_cycle_ms + S.reader_cycle_low_duration);
    cycle_high_finish_ms = (uint32_t)(cycle_low_finish_ms + powerCycleHighDuration()*1000UL);
    
    //RD_PRINT(F("cycle_low_finish_ms: ")); RD_PRINTLN(cycle_low_finish_ms);
    //RD_PRINT(F("cycle_high_finish_ms: ")); RD_PRINTLN(cycle_high_finish_ms);
    
    
    /*
      Displays most if not all local vars.
      TODO: Should this be put into a function? Yes, I think so!
    */
    RD_LOG(6, F("tag_last_read_ms "), false);
      RD_LOG(6, tag_last_read_ms, true);
    RD_LOG(6, F("msSinceLastTagRead() "), false);
      RD_LOG(6, msSinceLastTagRead(), true);
    RD_LOG(6, F("last_power_cycle_ms "), false);
      RD_LOG(6, last_power_cycle_ms, true);
    RD_LOG(6, F("msSinceLastPowerCycle() "), false);
      RD_LOG(6, msSinceLastPowerCycle(), true);

          
    if (msSinceLastTagRead() > S.tag_read_sleep_interval) {
      // Checks the rfid reader for new data.
      pollReader();
    }

    //if (msSinceLastTagRead() > ms_reader_cycle_total) tag_last_read_id = 0;

    // Hmm... adding this log line, and the bogus value appears to have vanished.
    //  RD_LOG(6, F("tag_last_read_ms again "), false);
    //    RD_LOG(6, tag_last_read_ms, true);

    RD_LOG(6, "tag_last_read_id: ", false); RD_LOG(6, tag_last_read_id, true);
    RD_LOG(6, F("READER LOOP END "), false); RD_LOG(6, millis(), true);
  }
  

  uint32_t Reader::msSinceLastTagRead() {
    return (uint32_t)(current_ms - tag_last_read_ms);
  }

  uint32_t Reader::msSinceLastPowerCycle() {
    return (uint32_t)(current_ms - last_power_cycle_ms);
  }

  // TODO: this is currently not used (2020-02-14), but should be to decouple reader-cycle-high-duration from soft-timeout.
  //uint32_t Reader::msReaderCycleTotal() {
	// uint32_t Reader::cycleTotalMs() {
	// 	//return (uint32_t)(S.tag_read_sleep_interval + S.reader_cycle_low_duration + powerCycleHighDuration()*1000UL);
	// 	return (uint32_t)(S.tag_read_sleep_interval + S.reader_cycle_low_duration + S.tag_last_read_soft_timeout * 1000UL);
	// }

  uint32_t Reader::powerCycleHighDuration() {
    if (power_cycle_high_duration_override > 0UL) {
      // This is an instance variable. If you set it, make sure to
      // return it to 0UL at some point.
      //if (power_cycle_high_duration_override > 15) {
      //  power_cycle_high_duration_override = 0;
      //  return 15
      //} else {
        return power_cycle_high_duration_override;
      //}

    } else if (S.tag_last_read_soft_timeout < 15) {
      return S.tag_last_read_soft_timeout;
      
    } else {
      return 15;
    }
  }

  uint32_t Reader::tagLastReadHardTimeoutX1000() {
    return (uint32_t)(S.tag_last_read_hard_timeout*1000UL);
  }

  //  uint32_t Reader::cycleLowFinishMs() {
  //    return last_power_cycle_ms + S.reader_cycle_low_duration;
  //  }
  //
  //  uint32_t Reader::cycleHighFinishMs() {
  //    return cycleLowFinishMs() + powerCycleHighDuration()*1000;
  //  }


  // Polls reader serial port and processes incoming tag data.
  void Reader::pollReader() {
    // If data available on Controller serial port, do something.
    RD_LOG(6, F("Reader.pollReader() name, len: "), false);
    RD_LOG(6, name(), false);    
    RD_LOG(6, F(", "), false);
    RD_LOG(6, raw_tag_length, true);

    // TODO (from Controller class): Consider moving this if/then condition to Controller::loop() function.
    if (serial_port->available()) {
      while (serial_port->available()) {
        if (buff_index >= MAX_TAG_LENGTH || buff_index >= raw_tag_length) {
          resetBuffer();
          continue;
        }
        
        buff[buff_index] = serial_port->read();

        // Logs details of each byte of tag received.
        
        #ifdef RD_DEBUG
          int lv = 6; // this is also used below...
          
          LOG(lv, F("(")); LOG(lv, buff_index); LOG(lv, F(")"));
          if (
            (buff[buff_index] >= 48U && buff[buff_index] <= 57U) ||
            (buff[buff_index] >= 65U && buff[buff_index] <= 70U) ||
            (buff[buff_index] >= 97U && buff[buff_index] <= 102U)
          ) { LOG(lv, (char)buff[buff_index]); }
          
          LOG(lv, ":");
          LOG(lv, buff[buff_index], HEX); LOG(lv, ":");
          LOG(lv, buff[buff_index], DEC);
        #endif

        // TODO: Move this to main Controller::loop() function?
        //uint8_t final_index = S.RAW_TAG_LENGTH - 1;
        uint8_t final_index = raw_tag_length - 1U;
  
        if (buff_index == 0U && buff[0U] != 2U) { // reset bogus read
          resetBuffer();
        } else if (buff_index == final_index && buff[final_index] != 3U) { // reset bogus read
          resetBuffer();
          RD_LOG(lv, "", true);
        } else if (buff_index > final_index || buff_index >= MAX_TAG_LENGTH) { // reset bogus read
          resetBuffer();
          RD_LOG(lv, "", true);
        } else if (buff_index < final_index) { // good read, add comma to log and keep reading
          buff_index++;
          RD_LOG(lv, ",", false);
        } else { // tag complete, now process it
          RD_LOG(lv, "", true);
          processTag(buff);
          //tag_last_read_ms = current_ms;
          resetBuffer();
          return;
        }
      }

    // If no data on Controller serial port and sufficient conditions exist,
    // then call cycle-reader-power.
    } else if (
      msSinceLastTagRead() > powerCycleHighDuration() * 1000UL ||
      tag_last_read_ms == 0UL
    ) {
      cycleReaderPower();
    }    
  }

  // TODO (Reader): I think part of this function should stay in Controller,
  // the part that decides what to do with a successfull tag read/authentication.
  // The big question is: Where/what do we do with the successfully read tag? Where do we store it?
  //
  // Processes a received array of tag bytes.
  // NOTE (from Controller): array args in func definitions can only use absolute constants, not vars.
  // TODO (from Controller): create macro definition for max-tag-length that can be used in func definition array args.
  void Reader::processTag(uint8_t _tag[]) {
    LOG(5, name(), false);
    LOG(5, F(" processTag() received buffer: 0x "), false);
    if (LogLevel() >=5) {
      for (int n=0; n<raw_tag_length; n++) {
        LOG(5, _tag[n], 16, false);
        LOG(5, F(" "), false);
      }
      LOG(5, "", true);
    }

    // DEV (from Controller): Use this to ensure that virtual functions are working in derived classes.
    //  RD_PRINT(F("Reader::processTag() calling echo(): "));
    //  int tst = echo(24);
    //  RD_PRINTLN(tst);

    // This calls the reader-specific process code.
    //RD_PRINTLN(F("Reader::processTag() calling processTagData()"));
    uint32_t tag_id = processTagData(_tag);
    
    RD_LOG(5, F("Tag rslt from processTagData(): "), false);
    RD_LOG(5, tag_id, true);

    if (tag_id) {
      RGB[0]->go_low();
      RGB[2]->once();
    }

    // If tag is valid, immediatly update proximity-state.
    // Actually, in the refactored Reader implementation, just update tag_last_read_ms,
    // and maybe store the last-read-tag in a var.
    // Proximity state should be entirely handled by Controller (Gate/State) class.
    if (tag_id > 0UL && ::Tags::TagSet.getTagIndex(tag_id) >= 0) { // 0 is valid index.  
      tag_last_read_ms = current_ms;

      //LOG(4, F("Authorized tag: "));
      LOG(4, F("Authorized tag: "));

    // Otherwise, don't do anything (not necessarily a failed proximity-state yet).
    } else {
      //LOG(4, F("Unauthorized or invalid tag: "));
      LOG(4, F("Tag not valid/authorized: "));
    }

    //Serial.println(tag_id);
    LOG(4, tag_id, true);
    
    tag_last_read_id = tag_id;
    
  } // processTag()

  void Reader::resetBuffer() {
    // RD_LOG(6, "Reader::resetBuffer", true); // This makes a mess of other log items.
    buff_index = 0U;
    //strncpy(buff, NULL, reader->raw_tag_length);
    //strncpy(buff, NULL, MAX_TAG_LENGTH);
    //memcpy(buff, 0, MAX_TAG_LENGTH);
    memset(buff, 0U, MAX_TAG_LENGTH);
  }

  void Reader::cycleReaderPower() {
    if (current_ms >= cycle_high_finish_ms || last_power_cycle_ms == 0UL) {
      
      LOG(5, F("cycleReaderPower() tag read "));
      if (tag_last_read_ms > 0UL) {
        //LOG(4, (ms_since_last_tag_read)/1000UL);
        LOG(5, msSinceLastTagRead()/1000UL);
        LOG(5, F("s ago"));
      } else {
        LOG(5, F("never"));
      }

      LOG(5, F(", cycled "));
      if (last_power_cycle_ms > 0UL) {
        //LOG(4, (ms_since_last_reader_power_cycle)/1000UL);
        LOG(5, msSinceLastPowerCycle()/1000UL);
        LOG(5, F("s ago"));
      } else {
        LOG(5, F("never"));
      }
      LOG(5, "", true);

      RD_LOG(6, F("cycleReaderPower() setting LOW"), true);
            
      //digitalWrite(READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? LOW : HIGH);
      digitalWrite(READER_POWER_CONTROL_PIN, power_control_logic ? LOW : HIGH);
      last_power_cycle_ms = current_ms;
      
    } else if (current_ms >= cycle_low_finish_ms) {
      RD_LOG(6, F("cycleReaderPower() setting HIGH"), true);
      //digitalWrite(READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
      digitalWrite(READER_POWER_CONTROL_PIN, power_control_logic ? HIGH : LOW);
    }
  }

  


  
