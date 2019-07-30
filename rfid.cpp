#include "rfid.h"
#include <EEPROM.h>

  // Constructors
  //RFID::RFID(Stream *_serial_port, Led *_blinker, Reader *_reader) :
  //RFID::RFID(Stream *_serial_port, Led *_blinker) :
  RFID::RFID(Reader *_reader, Led *_blinker) :
    //  buff {},
    //  buff_index(0UL),
    proximity_state(0),
    //  current_ms(millis()),
    //  last_tag_read_ms(0UL),
    //  last_reader_power_cycle_ms(0UL),
    //  reader_power_cycle_high_duration(0UL),
    //
    //  ms_since_last_tag_read(0UL),
    //  ms_since_last_reader_power_cycle(0UL),
    //  ms_reader_cycle_total(0UL),
    //  cycle_low_finish_ms(0UL),
    //  cycle_high_finish_ms(0UL),
    //  //tag_last_read_timeout_x_1000(0UL),
    //  
    //  serial_port(_serial_port),

    reader(_reader),
    blinker(_blinker)
    //reader(_reader)
    
  { ; }


  // TODO: Some (all?) of this needs to be moved to Reader setup().
  void RFID::begin() {
    // Moved output switch initialization to its own function.
    // Make sure to run this somewhere, if not here.
    //initializeOutput();
    
    pinMode(S.READER_POWER_CONTROL_PIN, OUTPUT);

    // Sets local 'reader' to instance of Reader.
    //reader = GetReader(S.DEFAULT_READER); // Moving to main .ino, to be passed into RFID::RFID().

    Serial.print(F("Starting RFID reader "));
    Serial.print(reader->reader_name);
    Serial.print(F(", with EEPROM proximity state "));
    Serial.println(proximity_state);
    //  Serial.print(F(", and output switch pin: "));
    //  Serial.println(S.OUTPUT_SWITCH_PIN);
        
    // Initializes the reader power/reset control.
    //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
    // This first one is to clear any built-up charge, as something is holding the reader low at startup.
    digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? LOW : HIGH);
    delay(50);
    digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? HIGH : LOW);
    
  } // setup()

  void RFID::loop() {
    // IMPORTANT: Some numbers use instance (global) vars, and some use functions.
    //   Which uses which might not make sense, but it's what works.
    //   Before finding the "magic" balance, I was getting calculation
    //   errors in the comparisons, maybe from corrupted memory/data,
    //   but I still don't really know what caused it.
    //   Anyway, be very careful if modifying the vars & functions of this class.
    //   UPDATE: This problem might be gone, as it was caused by unrelated UB in the program.

    
    //RF_PRINT(F("*** RFID LOOP BEGIN "));
    //  current_ms = millis();
    //  RF_PRINT(current_ms); RF_PRINTLN(F(" ***"));
    //  
    //  cycle_low_finish_ms = (uint32_t)(last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION);
    //  cycle_high_finish_ms = (uint32_t)(cycle_low_finish_ms + readerPowerCycleHighDuration()*1000UL);
    //  
    //  RF_PRINT(F("cycle_low_finish_ms: ")); RF_PRINTLN(cycle_low_finish_ms);
    //  RF_PRINT(F("cycle_high_finish_ms: ")); RF_PRINTLN(cycle_high_finish_ms);
    //  
    //  //ms_since_last_tag_read = (uint32_t)(current_ms - last_tag_read_ms);
    //  //ms_since_last_reader_power_cycle = (uint32_t)(current_ms - last_reader_power_cycle_ms);
    //  ms_reader_cycle_total = (uint32_t)(S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION*1000UL);
    //  //tag_last_read_timeout_x_1000 = (uint32_t)(S.TAG_LAST_READ_TIMEOUT*1000UL);
  
    
    /***  Displays most if not all local vars.       ***/
    /***  TODO: Should this be put into a function?  ***/
    /***                                             ***/
    //  RF_PRINT(F("last_tag_read_ms: "));
    //    RF_PRINTLN(reader->last_tag_read_ms);
    //  RF_PRINT(F("last_reader_power_cycle_ms: "));
    //    RF_PRINTLN(reader->last_reader_power_cycle_ms);
    //  RF_PRINT(F("msSinceLastTagRead(): "));
    //    RF_PRINTLN(reader->msSinceLastTagRead());
    //  RF_PRINT(F("msSinceLastReaderPowerCycle(): "));
    //    RF_PRINTLN(reader->msSinceLastReaderPowerCycle());
    //  RF_PRINT(F("ms_reader_cycle_total: "));
    //    RF_PRINTLN(reader->ms_reader_cycle_total);
    //  RF_PRINT(F("tagLastReadTimeoutX1000(): "));
    //    RF_PRINTLN(reader->tagLastReadTimeoutX1000());
    //  RF_PRINT(F("readerPowerCycleHighDuration()"))
    //    RF_PRINTLN(reader->readerPowerCycleHighDuration());
    //  RF_PRINT(F("RPCHD*1000: "));
    //    RF_PRINTLN(reader->readerPowerCycleHighDuration()*1000UL);
    //  RF_PRINTLN(F("***"));
    //
    //  RF_PRINT("RFID::loop() about to touch reader: ");
    //  RF_PRINTLN(reader->reader_name);
    
    //if (reader->msSinceLastTagRead() > S.TAG_READ_SLEEP_INTERVAL) {
      // Checks the rfid reader for new data.
      //pollReader();
      //reader->loop();

      // Check output switch timeout.
      proximityStateController();
    //}
  }

  //  uint32_t RFID::msSinceLastTagRead() {
  //    return (uint32_t)(current_ms - last_tag_read_ms);
  //  }
  //
  //  uint32_t RFID::msSinceLastReaderPowerCycle() {
  //    return (uint32_t)(current_ms - last_reader_power_cycle_ms);
  //  }
  //
  //  uint32_t RFID::msReaderCycleTotal() {
  //    //return S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + readerPowerCycleHighDuration() * 1000;
  //    //return S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION*1000;
  //    return (uint32_t)(S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION*1000UL);
  //  }
  //
  //  uint32_t RFID::readerPowerCycleHighDuration() {
  //    if (reader_power_cycle_high_duration > 0UL) {
  //      return reader_power_cycle_high_duration;
  //    } else {
  //      return S.READER_CYCLE_HIGH_DURATION;
  //    }
  //  }
  //
  //  uint32_t RFID::tagLastReadTimeoutX1000() {
  //    return (uint32_t)(S.TAG_LAST_READ_TIMEOUT*1000UL);
  //  }
  //
  //  //  uint32_t RFID::cycleLowFinishMs() {
  //  //    return last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION;
  //  //  }
  //  //
  //  //  uint32_t RFID::cycleHighFinishMs() {
  //  //    return cycleLowFinishMs() + readerPowerCycleHighDuration()*1000;
  //  //  }


  // Initializes output switch.
  void RFID::initializeOutput() {
    pinMode(S.OUTPUT_SWITCH_PIN, OUTPUT);    
    
    // Starts up with whatever state we left off in.
    // Protects against thief using 'admin' to move
    // in fits and starts, since a failed proximity
    // timeout will set S.proximity_state to 0.
    // This also tells the proximityStateController
    // where we left off at power-down (or reset).
    proximity_state = S.proximity_state;

    Serial.print(F("Setting output switch per saved proximity_state: "));
    Serial.println(proximity_state);
    // Switches the main load according to current proximity_state.
    // This turns on the load if saved prox-state was "on".
    // This begins the courtesey grace period until the system can
    // start processing tags (at which time, it will immediately
    // shut down output until a successful tag read).
    // TODO: Protect this action with a gate of some sort.
    //       Use a setting, or  a pin, or a key-press.
    digitalWrite(S.OUTPUT_SWITCH_PIN, proximity_state);
  }

  
  //  // Polls reader serial port and processes incoming tag data.
  //  void RFID::pollReader() {
  //    // If data available on RFID serial port, do something.
  //    RF_PRINT(F("RFID::pollReader() reader->reader_name: "));
  //    RF_PRINTLN(reader->reader_name);    
  //    RF_PRINT(F("RFID::pollReader() reader->raw_tag_length: "));
  //    RF_PRINTLN(reader->raw_tag_length);
  //
  //    // TODO: Consider moving this if/then condition to RFID::loop() function.
  //    if (serial_port->available()) {
  //      while (serial_port->available()) {
  //        if (buff_index >= MAX_TAG_LENGTH || buff_index >= reader->raw_tag_length) {
  //          resetBuffer();
  //          continue;
  //        }
  //        
  //        buff[buff_index] = serial_port->read();
  //
  //        RF_PRINT(F("(")); RF_PRINT(buff_index); RF_PRINT(F(")"));
  //        if (
  //          (buff[buff_index] >= 48U && buff[buff_index] <= 57U) ||
  //          (buff[buff_index] >= 65U && buff[buff_index] <= 70U) ||
  //          (buff[buff_index] >= 97U && buff[buff_index] <= 102U)
  //        ) { RF_PRINT((char)buff[buff_index]); }
  //        
  //        RF_PRINT(":");
  //        RF_PRINT(buff[buff_index], HEX); RF_PRINT(":");
  //        RF_PRINT(buff[buff_index], DEC);
  //
  //        // TODO: Move this to main RFID::loop() function?
  //        //uint8_t final_index = S.RAW_TAG_LENGTH - 1;
  //        uint8_t final_index = reader->raw_tag_length - 1U;
  //  
  //        if (buff_index == 0U && buff[0U] != 2U) { // reset bogus read
  //          resetBuffer();
  //        } else if (buff_index == final_index && buff[final_index] != 3U) { // reset bogus read
  //          resetBuffer();
  //          RF_PRINTLN("");
  //        } else if (buff_index > final_index || buff_index >= MAX_TAG_LENGTH) { // reset bogus read
  //          resetBuffer();
  //          RF_PRINTLN("");
  //        } else if (buff_index < final_index) { // good read, add comma to log and keep reading
  //          buff_index++;
  //          RF_PRINT(",");
  //        } else { // tag complete, now process it
  //          RF_PRINTLN("");
  //          processTagData(buff);
  //          //last_tag_read_ms = current_ms;
  //          resetBuffer();
  //          return;
  //        }
  //      }
  //
  //    // If no data on RFID serial port and sufficient conditions exist,
  //    // then call cycle-reader-power.
  //    } else if (
  //      msSinceLastTagRead() > readerPowerCycleHighDuration() * 1000UL ||
  //      last_tag_read_ms == 0UL
  //    ) {
  //      cycleReaderPower();
  //    }    
  //  }
  //
  //  // Processes a received array of tag bytes.
  //  // NOTE: array args in func definitions can only use absolute constants, not vars.
  //  // TODO: create macro definition for max-tag-length that can be used in func definition array args.
  //  //void RFID::processTagData(uint8_t _tag[S.RAW_TAG_LENGTH]) {
  //  void RFID::processTagData(uint8_t _tag[]) {
  //    RF_PRINT(F("RFID::processTagData() received buffer, using reader: "));
  //    RF_PRINTLN(reader->reader_name);
  //
  //    // DEV: Use this to ensure that virtual functions are working in derived classes.
  //    //  RF_PRINT(F("RFID::processTagData() calling reader->echo(): "));
  //    //  int tst = reader->echo(24);
  //    //  RF_PRINTLN(tst);
  //
  //    RF_PRINTLN(F("RFID::processTagData() calling reader->processTagData(_tag)"));
  //    uint32_t tag_id = reader->processTagData(_tag);
  //    
  //    RF_PRINT(F("Tag result from Reader: "));
  //    RF_PRINTLN(tag_id);
  //
  //    // Assuming successful tag-read at this point,
  //    // add the tag to Tags array if get_tag_from_scanner is 1.
  //    // This pushes tag_id as string directly into BTmenu::buff,
  //    // which then picks it up and processes it as if were manually entered.
  //    if (SerialMenu::Current->get_tag_from_scanner == 1) {
  //      char str[9];
  //      sprintf(str, "%lu", tag_id);
  //      strlcpy(SerialMenu::Current->buff, str, sizeof(SerialMenu::buff));
  //      SerialMenu::Current->get_tag_from_scanner = 0;
  //    }
  //
  //    // If tag is valid, immediatly update proximity-state.
  //    //if (tag_id > 0UL && GetTagIndex(tag_id) >= 0) { // 0 is valid index.
  //    if (tag_id > 0UL && ::Tags::TagSet.getTagIndex(tag_id) >= 0) { // 0 is valid index.  
  //      // Should this be the local function setProximityState()?
  //      //S.updateProximityState(1);
  //      setProximityState(1);
  //      last_tag_read_ms = current_ms;
  //      Serial.print(F("Authorized tag: "));
  //
  //    // Otherwise, don't do anything (not necessarily a failed proximity-state yet).
  //    } else {
  //      Serial.print(F("Unauthorized or invalid tag: "));
  //    }
  //    
  //    Serial.println(tag_id);
  //    
  //  }
  //
  //  void RFID::resetBuffer() {
  //    buff_index = 0U;
  //    //strncpy(buff, NULL, reader->raw_tag_length);
  //    //strncpy(buff, NULL, MAX_TAG_LENGTH);
  //    //memcpy(buff, 0, MAX_TAG_LENGTH);
  //    memset(buff, 0, MAX_TAG_LENGTH);
  //  }
  //
  //  void RFID::cycleReaderPower() {
  //    if (current_ms >= cycle_high_finish_ms || last_reader_power_cycle_ms == 0UL) {
  //      
  //      Serial.print(F("cycleReaderPower() tag read: "));
  //      if (last_tag_read_ms > 0UL) {
  //        //Serial.print((ms_since_last_tag_read)/1000UL);
  //        Serial.print(msSinceLastTagRead()/1000UL);
  //        Serial.print(F(" seconds ago"));
  //      } else {
  //        Serial.print(F("never"));
  //      }
  //
  //      Serial.print(F(", reader cycled: "));
  //      if (last_reader_power_cycle_ms > 0UL) {
  //        //Serial.print((ms_since_last_reader_power_cycle)/1000UL);
  //        Serial.print(msSinceLastReaderPowerCycle()/1000UL);
  //        Serial.println(F(" seconds ago"));
  //      } else {
  //        Serial.println(F("never"));
  //      }
  //
  //      RF_PRINTLN(F("cycleReaderPower() setting reader power LOW"));
  //            
  //      //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? LOW : HIGH);
  //      digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? LOW : HIGH);
  //      last_reader_power_cycle_ms = current_ms;
  //      
  //    } else if (current_ms >= cycle_low_finish_ms) {
  //      RF_PRINTLN(F("cycleReaderPower() setting reader power HIGH"));
  //      //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
  //      digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? HIGH : LOW);
  //    }
  //  }

  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void RFID::proximityStateController() {
    RF_PRINTLN(F("*** PROXIMITY ***"));
    RF_PRINT(F("last_tag_read_ms: ")); RF_PRINTLN(reader->last_tag_read_ms);
    RF_PRINT(F("last_reader_power_cycle_ms: ")); RF_PRINTLN(reader->last_reader_power_cycle_ms);
    RF_PRINT(F("msSinceLastTagRead(): ")); RF_PRINTLN(reader->msSinceLastTagRead());
    RF_PRINT(F("msSinceLastReaderPowerCycle(): ")); RF_PRINTLN(reader->msSinceLastReaderPowerCycle());
    RF_PRINT(F("ms_reader_cycle_total: ")); RF_PRINTLN(reader->ms_reader_cycle_total);
    RF_PRINT(F("tagLastReadTimeoutX1000(): ")); RF_PRINTLN(reader->tagLastReadTimeoutX1000());
    RF_PRINTLN(F("***"));
    
    // If NO TAG READ YET and reader has recently power cycled
    // This should probably calculate or use global setting for appropriate time-to-wait since last power cycle.
    if (
      reader->last_tag_read_ms == 0UL &&
      reader->last_reader_power_cycle_ms > 0UL &&
      reader->msSinceLastReaderPowerCycle() > 2000UL
      ){
      
      RF_PRINTLN(F("proximityStateController() startup GRACE period timeout, no tag found"));
      blinker->SlowBlink();
      setProximityState(0);
    
    // If last read is beyond TIMEOUT, and we've cycled reader at least once in that interval.
    } else if (
      reader->msSinceLastTagRead() > reader->tagLastReadTimeoutX1000() &&
      reader->last_reader_power_cycle_ms > 0UL &&
      reader->msSinceLastTagRead() > reader->msSinceLastReaderPowerCycle() &&
      reader->msSinceLastReaderPowerCycle() > 2000UL
      ){
      
      RF_PRINTLN(F("proximityStateController() TIMEOUT"));
      blinker->SlowBlink();
      setProximityState(0);

    // If last read is greater than reader-power-cycle-total AND
    // less than final timeout total, we're in the AGING zone.
    } else if (
      reader->last_tag_read_ms > 0UL &&
      reader->msSinceLastTagRead() > reader->ms_reader_cycle_total &&
      reader->msSinceLastTagRead() <= reader->tagLastReadTimeoutX1000()
      ){

      //RF_PRINTLN(F("### AGING ###"));
      //RF_PRINT(F("last_tag_read_ms: ")); RF_PRINTLN(last_tag_read_ms);
      //RF_PRINT(F("msSinceLastTagRead(): ")); RF_PRINTLN(msSinceLastTagRead());
      //RF_PRINT(F("ms_reader_cycle_total: ")); RF_PRINTLN(ms_reader_cycle_total);
      //RF_PRINT(F("tagLastReadTimeoutX1000(): ")); RF_PRINTLN(tagLastReadTimeoutX1000());
      //RF_PRINTLN(F("###  ###"));

      RF_PRINTLN(F("proximityStateController() AGING"));
      blinker->FastBlink();
      setProximityState(1);

    // If we're STILL YOUNG.
    } else if (
      reader->last_tag_read_ms > 0UL &&
      reader->msSinceLastTagRead() <= reader->ms_reader_cycle_total
      ){
        
      RF_PRINTLN(F("proximityStateController() still YOUNG"));
      blinker->Steady();
      setProximityState(1);

    // No expected condition was met (not sure what to do here yet).
    } else {
      RF_PRINTLN(F("proximityStateController() no condition was met (not necessarily a problem)"));
    }

    // TODO: Is there a better place for this?
    if (SerialMenu::run_mode == 0) digitalWrite(S.OUTPUT_SWITCH_PIN, proximity_state);
  }

  void RFID::setProximityState(int _state) {
    if (SerialMenu::run_mode == 0) {
      proximity_state = _state;
      
      if (proximity_state == 0) {
        reader->reader_power_cycle_high_duration = 3UL;
      } else {
        reader->reader_power_cycle_high_duration = 0UL;
      }
      
      S.updateProximityState(_state);
    }
  }


  /*  Static Vars & Functions  */

  // This is for aliasing the original Tags list to the new Tags list.
  // See beginning of .h file for typedef.
  TagArray& RFID::Tags = Tags::TagSet.tag_array;

  
