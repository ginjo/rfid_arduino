#include "rfid.h"

  // constructor
  RFID::RFID(Stream *_serial_port, Led *_blinker, Reader *_reader) :
  //RFID::RFID(Stream *_serial_port, Led *_blinker) :
    buff({}),
    buff_index(0),
    current_ms(millis()),
    last_tag_read_ms(0),
    last_reader_power_cycle_ms(0),
    reader_power_cycle_high_duration(0),
    serial_port(_serial_port),
    blinker(_blinker),
    reader(_reader),
    proximity_state(0)
  { ; }

  void RFID::begin() {
    // Starts up with whatever state we left off in.
    // Protects against thief using 'admin' to move
    // in fits and starts, since a failed proximity
    // timeout will set S.proximity_state to 0.
    // This also tells the proximityStateController
    // where we left off at power-down (or reset).
    proximity_state = S.proximity_state;

    //reader = new WL125;

    Serial.print(F("Starting RFID reader with proximity state: "));
    Serial.println(proximity_state);
    
    digitalWrite(13, proximity_state);
    
    // Initializes the reader power/reset control.
    //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
    digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? HIGH : LOW);
  }

  void RFID::loop() {
    current_ms = millis();

    /***  Displays most if not all local vars.      ***/
    /***  TODO: This should be put into a function  ***/
    /***                                            ***/
    /***  Copied from cycleReaderPower()            ***/
    DPRINT(F("*** RFID::loop() current_MS: "));
      DPRINTLN(current_ms);
    DPRINT("last_tag_read: ");
      DPRINTLN(last_tag_read_ms);
    DPRINT("last_p_cycle: ");
      DPRINTLN(last_reader_power_cycle_ms);
    DPRINT("cycleLowFinishMs(): ");
      DPRINTLN(cycleLowFinishMs()); // Requires that this var has been created.
    DPRINT("readerPowerCycleHighDuration()")
      DPRINTLN(readerPowerCycleHighDuration());
    DPRINT("RPCHD * 1000: ");
      DPRINTLN(readerPowerCycleHighDuration() * 1000);
    DPRINT("cycleHighFinishMs(): ");
      DPRINTLN(cycleHighFinishMs()); // Requires that this var has been created.
    /***  Copied from proximityStateController()  ***/
    DPRINT(F("msSinceLastTagRead(): "));
      DPRINTLN(msSinceLastTagRead());
    DPRINT("msSinceLastReaderPowerCycle(): ");
      DPRINTLN(msSinceLastReaderPowerCycle());
    DPRINT("last_tag_read_ms: ");
      DPRINTLN(last_tag_read_ms);
    DPRINT("last_reader_power_cycle_ms: ");
      DPRINTLN(last_reader_power_cycle_ms);
    DPRINT("msReaderCycleTotal(): ");
      DPRINTLN(msReaderCycleTotal());
    DPRINT("S.TAG_LAST_READ_TIMEOUT*1000: ");
      DPRINTLN(S.TAG_LAST_READ_TIMEOUT*1000);
    DPRINTLN("***");
    /*** *** ***/
    
    
    if (msSinceLastTagRead() > S.TAG_READ_SLEEP_INTERVAL) {
      //  Serial.print(F("LAST TAG READ: "));
      //  Serial.println(msSinceLastTagRead()/1000);

      // Checks the rfid reader for new data.
      pollReader();

      // Check fuel pump timeout on every loop.
      // TODO: Is this appropriate here? It was outside (below) the sleep block before.
      proximityStateController();
    }
  }

  uint32_t RFID::msSinceLastTagRead() {
    uint32_t result = current_ms - last_tag_read_ms;

    //  DPRINT(F("RFID::msSinceLastTagRead(): last-read-ms-ago, curr-ms, last-ms: "));
    //  DPRINT(result);
    //  DPRINT(", ");
    //  DPRINT(current_ms);
    //  DPRINT(", ");
    //  DPRINTLN(last_tag_read_ms);
    
    //  DPRINT(F("RFID::msSinceLastTagRead(): "));
    //  DPRINTLN(result);

    return result;
  }

  uint32_t RFID::msSinceLastReaderPowerCycle() {
    return current_ms - last_reader_power_cycle_ms;
  }

  uint32_t RFID::msReaderCycleTotal() {
    //return S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + readerPowerCycleHighDuration() * 1000;
    return S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION*1000;
  }

  uint32_t RFID::readerPowerCycleHighDuration() {
    if (reader_power_cycle_high_duration > 0) {
      return reader_power_cycle_high_duration;
    } else {
      return S.READER_CYCLE_HIGH_DURATION;
    }
  }

  uint32_t RFID::cycleLowFinishMs() {
    return last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION;
  }

  uint32_t RFID::cycleHighFinishMs() {
    return cycleLowFinishMs() + readerPowerCycleHighDuration()*1000;
  }

  

  // Polls reader serial port and processes incoming tag data.
  void RFID::pollReader() {
    // If data available on RFID serial port, do something.
    DPRINT("RFID::pollReader() reader->reader_name: ");
    DPRINTLN(reader->reader_name);    
    DPRINT("RFID::pollReader() reader->raw_tag_length: ");
    DPRINTLN(reader->raw_tag_length);
    
    if (serial_port->available()) {
      while (serial_port->available()) {
        buff[buff_index] = serial_port->read();

        DPRINT("("); DPRINT(buff_index); DPRINT(")");
        if (
          (buff[buff_index] >= 48 && buff[buff_index] <= 57) ||
          (buff[buff_index] >= 65 && buff[buff_index] <= 70) ||
          (buff[buff_index] >= 97 && buff[buff_index] <= 102)
        ) { DPRINT((char)buff[buff_index]); }
        
        DPRINT(":");
        DPRINT(buff[buff_index], HEX); DPRINT(":");
        DPRINT(buff[buff_index], DEC);

        // TODO: Move this to main RFID::loop() function?
        //uint8_t final_index = S.RAW_TAG_LENGTH - 1;
        uint8_t final_index = reader->raw_tag_length - 1;
  
        if (buff_index == 0 && buff[0] != 2) { // reset bogus read
          resetBuffer();
        } else if (buff_index == final_index && buff[final_index] != 3) { // reset bogus read
          resetBuffer();
          DPRINTLN("");
        } else if (buff_index > final_index) { // reset bogus read
          resetBuffer();
          DPRINTLN("");
        } else if (buff_index < final_index) { // good read, add comma to log and keep reading
          buff_index++;
          DPRINT(",");
        } else { // tag complete, now process it
          DPRINTLN("");
          processTagData(buff);
          //last_tag_read_ms = current_ms;
          resetBuffer();
          return;
        }
      }

    // If no data on RFID serial port and sufficient conditions exist,
    // then call cycle-reader-power.
    } else if (
      msSinceLastTagRead() > readerPowerCycleHighDuration() * 1000 ||
      last_tag_read_ms == 0
    ) {
      cycleReaderPower();
    }    
  }

  // Processes a received array of tag bytes.
  // NOTE: array args in func definitions can only use absolute constants, not vars.
  // TODO: create macro definition for max-tag-length that can be used in func definition array args.
  //void RFID::processTagData(uint8_t _tag[S.RAW_TAG_LENGTH]) {
  void RFID::processTagData(uint8_t _tag[]) {
    DPRINT(F("RFID::processTagData() received buffer, using reader: "));
    DPRINTLN(reader->reader_name);

    // DEV: Use this to ensure that virtual functions are working in derived classes.
    //  DPRINT(F("RFID::processTagData() calling reader->echo(): "));
    //  int tst = reader->echo(24);
    //  DPRINTLN(tst);

    DPRINTLN(F("RFID::processTagData() calling reader->processTagData(_tag)"));
    uint32_t tag_id = reader->processTagData(_tag);
    
    DPRINT(F("Tag result: "));
    DPRINTLN(tag_id);

    // assuming successful tag at this point?

    // If tag is valid, immediatly update proximity-state.
    if (tag_id > 0) {
      S.updateProximityState(1);
      last_tag_read_ms = current_ms;
      Serial.print(F("Received valid tag: "));

    // Otherwise, don't do anything (not necessarily a failed proximity-state yet).
    } else {
      Serial.print(F("Tag not valid: "));
    }
    
    Serial.println(tag_id);
    
  }

  void RFID::resetBuffer() {
    buff_index = 0;
    //strncpy(buff, NULL, S.RAW_TAG_LENGTH);
    strncpy(buff, NULL, reader->raw_tag_length);
  }

  void RFID::cycleReaderPower() {
    
    // Moved to their own functions.
    //uint32_t cycleLowFinishMs() = last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION;
    //uint32_t cycleHighFinishMs() = cycleLowFinishMs() + readerPowerCycleHighDuration() * 1000;

    //  // TODO: Put this at the main RFID loop level, and include all relevant vars for RFID class.
    //  DPRINT(F("cycleReaderPower() current_MS, last_tag_read, last_p_cycle, cycleLowFinishMs(), readerPowerCycleHighDuration(), RCHD * 1000, cycleHighFinishMs(): "));
    //  DPRINT(current_ms); DPRINT(",");
    //  DPRINT(last_tag_read_ms); DPRINT(",");
    //  DPRINT(last_reader_power_cycle_ms); DPRINT(",");
    //  DPRINT(cycleLowFinishMs()); DPRINT(",");
    //  DPRINT(readerPowerCycleHighDuration()); DPRINT(",");
    //  DPRINT(readerPowerCycleHighDuration() * 1000); DPRINT(",");
    //  DPRINTLN(cycleHighFinishMs());
    
    if (current_ms >= cycleHighFinishMs() || last_reader_power_cycle_ms == 0) {
      DPRINTLN(F("cycleReaderPower() setting reader power LOW"));
      
      Serial.print(F("cycleReaderPower() tag read: "));
      if (last_tag_read_ms > 0) {
        Serial.print((msSinceLastTagRead())/1000);
        Serial.print(F(" seconds ago"));
      } else {
        Serial.print(F("never"));
      }

      Serial.print(F(", reader cycled: "));
      if (last_reader_power_cycle_ms > 0) {
        Serial.print((msSinceLastReaderPowerCycle())/1000);
        Serial.println(F(" seconds ago"));
      } else {
        Serial.println(F("never"));
      }

      
      last_reader_power_cycle_ms = current_ms;
      //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? LOW : HIGH);
      digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? LOW : HIGH);
      
    } else if (current_ms >= cycleLowFinishMs()) {
      DPRINTLN(F("cycleReaderPower() setting reader power HIGH"));
      //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
      digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? HIGH : LOW);
    }
  }

  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void RFID::proximityStateController() {
    DPRINTLN("### PROXIMITY ###");
    DPRINT("last_tag_read_ms: "); DPRINTLN(last_tag_read_ms);
    DPRINT("msSinceLastTagRead(): "); DPRINTLN(msSinceLastTagRead());
    DPRINT("msReaderCycleTotal(): "); DPRINTLN(msReaderCycleTotal());
    DPRINT("S.TAG_LAST_READ_TIMEOUT*1000: "); DPRINTLN(S.TAG_LAST_READ_TIMEOUT*1000);
    DPRINTLN("###  ###");
    
    
    // If NO TAG READ YET and reader has recently power cycled
    // This should probably calculate or use global setting for appropriate time-to-wait since last power cycle.
    if (
      last_tag_read_ms == 0 &&
      last_reader_power_cycle_ms > 0 &&
      msSinceLastReaderPowerCycle() > 2000
      ){
      
      DPRINTLN(F("proximityStateController() startup grace period timeout, no tag found"));
      blinker->SlowBlink();
      setProximityState(0);
    
    // If last read was TOO LONG AGO, and we've cycled reader at least once in that interval.
    } else if (
      msSinceLastTagRead() > S.TAG_LAST_READ_TIMEOUT*1000 &&
      last_reader_power_cycle_ms > 0 &&
      msSinceLastTagRead() > msSinceLastReaderPowerCycle() &&
      msSinceLastReaderPowerCycle() > 2000
      ){
      
      DPRINTLN(F("proximityStateController() timeout"));
      blinker->SlowBlink();
      setProximityState(0);

    // If last read is greater than reader-power-cycle-total AND
    // less than final timeout total, we're in the AGING zone.
    } else if (
      (last_tag_read_ms > 0) &&
      (msSinceLastTagRead() > msReaderCycleTotal()) &&
      (msSinceLastTagRead() <= (S.TAG_LAST_READ_TIMEOUT*1000))
      ){

      //  DPRINTLN("### AGING ###");
      //  DPRINT("last_tag_read_ms: "); DPRINTLN(last_tag_read_ms);
      //  DPRINT("msSinceLastTagRead(): "); DPRINTLN(msSinceLastTagRead());
      //  DPRINT("msReaderCycleTotal(): "); DPRINTLN(msReaderCycleTotal());
      //  DPRINT("S.TAG_LAST_READ_TIMEOUT*1000: "); DPRINTLN(S.TAG_LAST_READ_TIMEOUT*1000);
      //  DPRINTLN("###  ###");

      DPRINTLN(F("proximityStateController() aging"));
      blinker->FastBlink();
      setProximityState(1);

    // If we're STILL YOUNG.
    } else if (last_tag_read_ms > 0 && msSinceLastTagRead() <= msReaderCycleTotal()) {
        DPRINTLN(F("proximityStateController() still young"));
        blinker->Steady();
        setProximityState(1);

    // No expected condition was met (not sure what to do here yet).
    } else {
      DPRINTLN(F("proximityStateController() no condition was met (not necessarily a problem)"));
    }

    // TODO: Replace 13 with a S.<setting>
    digitalWrite(13, proximity_state);
  }

  void RFID::setProximityState(int _state) {
    proximity_state = _state;
    
    if (proximity_state == 0) {
      reader_power_cycle_high_duration = 3;
    } else {
      reader_power_cycle_high_duration = 0;
    }
    
    S.updateProximityState(_state);
  }

  
