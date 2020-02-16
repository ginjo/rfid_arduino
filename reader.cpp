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
    //cycle_high_finish_ms(0UL),
    tag_last_read_id(0UL),
    serial_port(NULL) // experimental, so that serial_port is initialized
        
  {
    LOG(4, F("Loading reader: "));
    LOG(4, name(), true);
  }



  /*  Instance functions  */

  char *Reader::name() {return (char *)Name;}

  uint32_t Reader::processTagData(uint8_t[]) {
    LOG(2, F("Base Reader called processTagData"), true); // This should never happen, if everything running correctly.
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

    cycle_low_finish_ms = (uint32_t)(last_power_cycle_ms + S.reader_cycle_low_duration);
    
    /*
      TODO: Should this be put into a function? Yes, I think so!
    */
    RD_LOG(6, F("tag_last_read_ms "), false); RD_LOG(6, tag_last_read_ms, true);
    RD_LOG(6, F("msSinceLastTagRead "), false); RD_LOG(6, msSinceLastTagRead(), true);
    RD_LOG(6, F("last_power_cycle_ms "), false); RD_LOG(6, last_power_cycle_ms, true);
    RD_LOG(6, F("msSinceLastPowerCycle "), false); RD_LOG(6, msSinceLastPowerCycle(), true);
    RD_LOG(6, F("powerCycleHighDuration "), false); RD_LOG(6, powerCycleHighDuration(), true);
    RD_LOG(6, F("power_cycle_high_duration_override "), false); RD_LOG(6, power_cycle_high_duration_override, true);
    RD_LOG(6, F("cycle_low_finish_ms "), false); RD_LOG(6, cycle_low_finish_ms, true);
    RD_LOG(6, F("cycleHighFinishMs "), false); RD_LOG(6, cycleHighFinishMs(), true);

    // Limits reader serial port polling to once per tag_read_sleep_interval (1000 ms).
    if (msSinceLastTagRead() > S.tag_read_sleep_interval) {
      // Checks the rfid reader for new data.
      pollReader();
    }

    RD_LOG(6, "tag_last_read_id ", false); RD_LOG(6, tag_last_read_id, true);
    RD_LOG(6, F("READER LOOP END "), false); RD_LOG(6, millis(), true);
  }
  

  uint32_t Reader::msSinceLastTagRead() {
    return (uint32_t)(current_ms - tag_last_read_ms);
  }

  uint32_t Reader::msSinceLastPowerCycle() {
    return (uint32_t)(current_ms - last_power_cycle_ms);
  }

  // Sets power_cycle_high_duration_override with the lowest of three possibilities.
  // Pass in a multiplier (uint8_t) to increase power_cycle_high_duration_override
  // within the confines of the rules.
  //
  uint32_t Reader::powerCycleHighDuration(uint8_t multiplier) {
    if (power_cycle_high_duration_override > 0UL) {
      uint32_t o = power_cycle_high_duration_override * multiplier;
      uint32_t &s = S.tag_last_read_soft_timeout;
      uint32_t &m = S.reader_cycle_high_max;

      if (o <= s-1 && o <= m) power_cycle_high_duration_override = o;
      else if (s-1 <= o && s-1 <= m) power_cycle_high_duration_override = s-1;
      else power_cycle_high_duration_override = m;

      return power_cycle_high_duration_override;
    } else {
      return S.tag_last_read_soft_timeout;
    }
  }

  uint32_t Reader::tagLastReadHardTimeoutX1000() {
    return (uint32_t)(S.tag_last_read_hard_timeout*1000UL);
  }

  uint32_t Reader::cycleHighFinishMs() {
    return cycle_low_finish_ms + powerCycleHighDuration()*1000;
  }

  // Polls reader serial port and processes incoming tag data.
  void Reader::pollReader() {
    // If data available on Controller serial port, do something.
    RD_LOG(6, F("Reader.pollReader name, len: "), false);
    RD_LOG(6, name(), false);    
    RD_LOG(6, F(", "), false);
    RD_LOG(6, raw_tag_length, true);

    // TODO (from Controller class): Consider moving this if/then condition to Controller::loop() function.
    // 2020-02-15 ... I don't think so. This seems to be a Reader thing, not a controller thing.
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

    } else if (
    // If no data on Controller serial port and sufficient conditions exist,
    // then call cycle-reader-power.
    //
    // NOTE: The progressive cycle decay messes with the logic here,
    // and causes the loop to skip this after cycle-low, when it shouldn't.
    // Therefore, trying to divide by 2 to 'loosen' this up a bit.
    // Update: The divide-by-2 helped a bit, but the 3rd condition solved the issue.
    //
    // I'm not sure if this 'else if' is really needed. Could it just be 'else' ?
    // The conditions might just be to reduce the number of unnecessary calls to cycleReaderPower().
    // Extra calls to cycleReaderPower() shouldn't hurt anything... right??

      msSinceLastTagRead() > (powerCycleHighDuration() * 1000UL)/2 ||
      tag_last_read_ms == 0UL ||
      current_ms >= cycle_low_finish_ms ) // to make sure cycleReader is called when it needs to go HIGH from LOW.
    {
      // This log line puts out a LOT of data but can be useful.
      //RD_LOG(6, F("poll-calling-cycl "), false); RD_LOG(6, msSinceLastTagRead(), false); RD_LOG(6, F(" "), false); RD_LOG(6, powerCycleHighDuration() * 1000UL, true); 
      
      cycleReaderPower();
    }
  }

  // TODO (Reader): I think part of this function should stay in Controller,
  // the part that decides what to do with a successfull tag read/authentication.
  // The big question is: Where/what do we do with the successfully read tag? Where do we store it?
  //
  // Processes a received array of tag bytes.
  // NOTE (from Controller): array args in func definitions can only use absolute constants, not vars.
  //
  void Reader::processTag(uint8_t _tag[]) {
    LOG(5, name(), false);
    LOG(5, F(" processTag rcvd buffer: 0x "), false);
    if (LogLevel() >=5) {
      for (int n=0; n<raw_tag_length; n++) {
        LOG(5, _tag[n], 16, false);
        LOG(5, F(" "), false);
      }
      LOG(5, "", true);
    }

    // NOTE: (from Controller): Use this to ensure that virtual functions are working in derived classes.
    //  RD_PRINT(F("Reader::processTag() calling echo(): "));
    //  int tst = echo(24);
    //  RD_PRINTLN(tst);

    // Calls the reader-specific process code.
    //RD_LOG(F("Reader::processTag() calling processTagData()"), true);
    uint32_t tag_id = processTagData(_tag);
    
    //  RD_LOG(5, F("Rslt from processTagData "), false);
    //  RD_LOG(5, tag_id, true);

    if (tag_id) {
      RGB[0]->go_low();
      RGB[2]->once();
    }

    if (tag_id > 0UL && ::Tags::TagSet.getTagIndex(tag_id) >= 0) { // 0 is a valid index.  
    // If tag is valid, immediatly update proximity-state.
    // Actually, in the refactored Reader implementation, just update tag_last_read_ms,
    // and maybe store the last-read-tag in a var.
    // Proximity state should be entirely handled by Controller (Gate/State) class.
      tag_last_read_ms = current_ms;

      //LOG(4, F("Authorized tag: "));
      LOG(4, F("Authorized tag: "));

    } else {
    // Otherwise, don't do anything (not necessarily a failed proximity-state yet).
    
      //LOG(4, F("Unauthorized or invalid tag: "));
      LOG(4, F("Tag not valid/authorized: "));
    }

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
    if (current_ms >= cycleHighFinishMs() || last_power_cycle_ms == 0UL) {
      
      LOG(5, F("cycleReaderPower tag read "));
      if (tag_last_read_ms > 0UL) {
        LOG(5, msSinceLastTagRead()/1000UL);
        LOG(5, F("s ago"));
      } else {
        LOG(5, F("never"));
      }

      LOG(5, F(", cycled "));
      if (last_power_cycle_ms > 0UL) {
        LOG(5, msSinceLastPowerCycle()/1000UL);
        LOG(5, F("s ago"));
      } else {
        LOG(5, F("never"));
      }
      
      LOG(5, F(", interval "));
      LOG(5, powerCycleHighDuration(), true);
      

      RD_LOG(6, F("cycleReaderPower LOW"), true);

      // Exponentially increases time between power cycles (until Controller state changes).
      if (power_cycle_high_duration_override) {
        //  power_cycle_high_duration_override = ( // the following is a ternary experession... a big one!
        //    power_cycle_high_duration_override > S.tag_last_read_soft_timeout ?
        //    S.tag_last_read_soft_timeout - 1 :
        //    (power_cycle_high_duration_override * 2)
        //  );
        
        powerCycleHighDuration(2);
      }
            
      digitalWrite(READER_POWER_CONTROL_PIN, power_control_logic ? LOW : HIGH);
      last_power_cycle_ms = current_ms;
      
    } else if (current_ms >= cycle_low_finish_ms) {
      // This log line will produce a LOT of output but can be useful for troubleshooting.
      //RD_LOG(6, F("cycleReaderPower setting HIGH, high-dur "), false); RD_LOG(6, powerCycleHighDuration, true);
      
      digitalWrite(READER_POWER_CONTROL_PIN, power_control_logic ? HIGH : LOW);
    }
  }

  
