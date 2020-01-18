  #include "reader.h"

  const char Reader::Name[READER_NAME_LENGTH] = {"base"};
  //const char *Reader::name() {return Name;}
  char *Reader::name() {return (char *)Name;}
  
  // Defines Reader Constructor
  Reader::Reader(uint8_t _raw_tag_length, uint8_t _id_begin, uint8_t _id_end, bool _control_logic) :
    raw_tag_length(_raw_tag_length),
    id_begin(_id_begin),
    id_end(_id_end),
    power_control_logic(_control_logic),

    /* From Controller */
    buff {},
    buff_index(0UL),
    //proximity_state(0),
    current_ms(millis()),
    last_tag_read_ms(0UL),
    last_reader_power_cycle_ms(0UL),
    reader_power_cycle_high_duration(0UL),
    ms_since_last_tag_read(0UL),
    ms_since_last_reader_power_cycle(0UL),
    ms_reader_cycle_total(0UL),
    cycle_low_finish_ms(0UL),
    cycle_high_finish_ms(0UL),
    last_tag_read_id(0UL)
    //tag_last_read_timeout_x_1000(0UL),
    
  {
    //strlcpy(reader_name, _name, sizeof(reader_name));

    //LOG(4, F("Constructing Reader: "));
    //LOG(4, name(), true);
  }

  uint32_t Reader::processTagData(uint8_t[]) {
    RD_PRINTLN(F("Error: Called processTagData() on base Reader class"));
    return 0UL;
  }

  /***  From Controller  ***/

  void Reader::loop() {

    serial_port->listen();
    while (! serial_port->isListening()) delay(2);
    delay(20);

    RD_PRINT(F("READER LOOP BEGIN "));
    current_ms = millis();
    RD_PRINTLN(current_ms);
    
    cycle_low_finish_ms = (uint32_t)(last_reader_power_cycle_ms + S.reader_cycle_low_duration);
    cycle_high_finish_ms = (uint32_t)(cycle_low_finish_ms + readerPowerCycleHighDuration()*1000UL);
    
    //RD_PRINT(F("cycle_low_finish_ms: ")); RD_PRINTLN(cycle_low_finish_ms);
    //RD_PRINT(F("cycle_high_finish_ms: ")); RD_PRINTLN(cycle_high_finish_ms);
    
    ms_reader_cycle_total = (uint32_t)(S.tag_read_sleep_interval + S.reader_cycle_low_duration + S.reader_cycle_high_duration*1000UL);
    //tag_last_read_timeout_x_1000 = (uint32_t)(S.tag_last_read_timeout*1000UL);

    
    /***  Displays most if not all local vars.       ***/
    /***  TODO: Should this be put into a function?  ***/
    /***                                             ***/
    RD_PRINT(F("last_tag_read_ms: "));
      RD_PRINTLN(last_tag_read_ms);
    RD_PRINT(F("last_reader_power_cycle_ms: "));
      RD_PRINTLN(last_reader_power_cycle_ms);
    RD_PRINT(F("msSinceLastTagRead(): "));
      RD_PRINTLN(msSinceLastTagRead());
    RD_PRINT(F("msSinceLastReaderPowerCycle(): "));
      RD_PRINTLN(msSinceLastReaderPowerCycle());
    RD_PRINT(F("ms_reader_cycle_total: "));
      RD_PRINTLN(ms_reader_cycle_total);
    //RD_PRINT(F("tagLastReadTimeoutX1000(): "));
    //  RD_PRINTLN(tagLastReadTimeoutX1000());
    //RD_PRINT(F("readerPowerCycleHighDuration()"))
    //  RD_PRINTLN(readerPowerCycleHighDuration());
    //RD_PRINT(F("RPCHD*1000: "));
    //  RD_PRINTLN(readerPowerCycleHighDuration()*1000UL);
          
    if (msSinceLastTagRead() > S.tag_read_sleep_interval) {
      // Checks the rfid reader for new data.
      pollReader();
    }

    //if (msSinceLastTagRead() > ms_reader_cycle_total) last_tag_read_id = 0;

    RD_PRINT("Reader::last_tag_read_id: "); RD_PRINTLN(last_tag_read_id);
    RD_PRINT(F("READER LOOP END ")); RD_PRINTLN(millis());
  }
  

  uint32_t Reader::msSinceLastTagRead() {
    return (uint32_t)(current_ms - last_tag_read_ms);
  }

  uint32_t Reader::msSinceLastReaderPowerCycle() {
    return (uint32_t)(current_ms - last_reader_power_cycle_ms);
  }

  uint32_t Reader::msReaderCycleTotal() {
    return (uint32_t)(S.tag_read_sleep_interval + S.reader_cycle_low_duration + S.reader_cycle_high_duration*1000UL);
  }

  uint32_t Reader::readerPowerCycleHighDuration() {
    if (reader_power_cycle_high_duration > 0UL) {
      return reader_power_cycle_high_duration;
    } else {
      return S.reader_cycle_high_duration;
    }
  }

  uint32_t Reader::tagLastReadTimeoutX1000() {
    return (uint32_t)(S.tag_last_read_timeout*1000UL);
  }

  //  uint32_t Reader::cycleLowFinishMs() {
  //    return last_reader_power_cycle_ms + S.reader_cycle_low_duration;
  //  }
  //
  //  uint32_t Reader::cycleHighFinishMs() {
  //    return cycleLowFinishMs() + readerPowerCycleHighDuration()*1000;
  //  }


  // Polls reader serial port and processes incoming tag data.
  void Reader::pollReader() {
    // If data available on Controller serial port, do something.
    RD_PRINT(F("Reader::pollReader() name(), raw_tag_length: "));
    RD_PRINT(name());    
    RD_PRINT(F(", "));
    RD_PRINTLN(raw_tag_length);

    // TODO (from Controller class): Consider moving this if/then condition to Controller::loop() function.
    if (serial_port->available()) {
      while (serial_port->available()) {
        if (buff_index >= MAX_TAG_LENGTH || buff_index >= raw_tag_length) {
          resetBuffer();
          continue;
        }
        
        buff[buff_index] = serial_port->read();

        RD_PRINT(F("(")); RD_PRINT(buff_index); RD_PRINT(F(")"));
        if (
          (buff[buff_index] >= 48U && buff[buff_index] <= 57U) ||
          (buff[buff_index] >= 65U && buff[buff_index] <= 70U) ||
          (buff[buff_index] >= 97U && buff[buff_index] <= 102U)
        ) { RD_PRINT((char)buff[buff_index]); }
        
        RD_PRINT(":");
        RD_PRINT(buff[buff_index], HEX); RD_PRINT(":");
        RD_PRINT(buff[buff_index], DEC);

        // TODO: Move this to main Controller::loop() function?
        //uint8_t final_index = S.RAW_TAG_LENGTH - 1;
        uint8_t final_index = raw_tag_length - 1U;
  
        if (buff_index == 0U && buff[0U] != 2U) { // reset bogus read
          resetBuffer();
        } else if (buff_index == final_index && buff[final_index] != 3U) { // reset bogus read
          resetBuffer();
          RD_PRINTLN("");
        } else if (buff_index > final_index || buff_index >= MAX_TAG_LENGTH) { // reset bogus read
          resetBuffer();
          RD_PRINTLN("");
        } else if (buff_index < final_index) { // good read, add comma to log and keep reading
          buff_index++;
          RD_PRINT(",");
        } else { // tag complete, now process it
          RD_PRINTLN("");
          processTag(buff);
          //last_tag_read_ms = current_ms;
          resetBuffer();
          return;
        }
      }

    // If no data on Controller serial port and sufficient conditions exist,
    // then call cycle-reader-power.
    } else if (
      msSinceLastTagRead() > readerPowerCycleHighDuration() * 1000UL ||
      last_tag_read_ms == 0UL
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
    RD_PRINT(F("Reader::processTag() received buffer w/reader: "));
    RD_PRINTLN(name());

    // DEV (from Controller): Use this to ensure that virtual functions are working in derived classes.
    //  RD_PRINT(F("Reader::processTag() calling echo(): "));
    //  int tst = echo(24);
    //  RD_PRINTLN(tst);

    // This calls the reader-specific process code.
    //RD_PRINTLN(F("Reader::processTag() calling processTagData()"));
    uint32_t tag_id = processTagData(_tag);
    
    RD_PRINT(F("Tag result from processTagData(): "));
    RD_PRINTLN(tag_id);

    if (tag_id) {
      RGB[0]->go_low();
      RGB[2]->once();
    }

    // If tag is valid, immediatly update proximity-state.
    // Actually, in the refactored Reader implementation, just update last_tag_read_ms,
    // and maybe store the last-read-tag in a var.
    // Proximity state should be entirely handled by Controller (Gate/State) class.
    if (tag_id > 0UL && ::Tags::TagSet.getTagIndex(tag_id) >= 0) { // 0 is valid index.  
      // This should be handled in the Controller class, not here.
      // TODO: Make sure it's ok to not run this here!
      //setProximityState(1);
      last_tag_read_ms = current_ms;

      //LOG(4, F("Authorized tag: "));
      LOG(4, F("Authorized tag: "));

    // Otherwise, don't do anything (not necessarily a failed proximity-state yet).
    } else {
      //LOG(4, F("Unauthorized or invalid tag: "));
      LOG(4, F("Unauthorized or invalid tag: "));
    }

    //Serial.println(tag_id);
    LOG(4, tag_id, true);
    
    last_tag_read_id = tag_id;
    
  } // processTag()

  void Reader::resetBuffer() {
    RD_PRINTLN("Reader::resetBuffer");
    buff_index = 0U;
    //strncpy(buff, NULL, reader->raw_tag_length);
    //strncpy(buff, NULL, MAX_TAG_LENGTH);
    //memcpy(buff, 0, MAX_TAG_LENGTH);
    memset(buff, 0U, MAX_TAG_LENGTH);
  }

  void Reader::cycleReaderPower() {
    if (current_ms >= cycle_high_finish_ms || last_reader_power_cycle_ms == 0UL) {
      
      LOG(4, F("cycleReaderPower() tag read "));
      if (last_tag_read_ms > 0UL) {
        //LOG(4, (ms_since_last_tag_read)/1000UL);
        LOG(4, msSinceLastTagRead()/1000UL);
        LOG(4, F("s ago"));
      } else {
        LOG(4, F("never"));
      }

      LOG(4, F(", cycled "));
      if (last_reader_power_cycle_ms > 0UL) {
        //LOG(4, (ms_since_last_reader_power_cycle)/1000UL);
        LOG(4, msSinceLastReaderPowerCycle()/1000UL);
        LOG(4, F("s ago"));
      } else {
        LOG(4, F("never"));
      }

      LOG(4, F(", uptime "));
      printUptime(true);

      RD_PRINTLN(F("cycleReaderPower() setting reader power LOW"));
            
      //digitalWrite(READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? LOW : HIGH);
      digitalWrite(READER_POWER_CONTROL_PIN, power_control_logic ? LOW : HIGH);
      last_reader_power_cycle_ms = current_ms;
      
    } else if (current_ms >= cycle_low_finish_ms) {
      RD_PRINTLN(F("cycleReaderPower() setting reader power HIGH"));
      //digitalWrite(READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
      digitalWrite(READER_POWER_CONTROL_PIN, power_control_logic ? HIGH : LOW);
    }
  }


  /* Static vars & functions */

  void Reader::PrintReaders(Stream *sp) {
    sp->print(F("Readers ("));
    sp->print(READER_COUNT);
    sp->println(F(")"));
    for (int n=1; n <= READER_COUNT; n++) {
      char output[24] = "";
      snprintf(output, 24, "%i %s (%s)", n, (S.default_reader == (int8_t)n ? "*" : " "), NameFromIndex(n));
      sp->println(output);
    }
  }


  
