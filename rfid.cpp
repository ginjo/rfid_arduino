#include "rfid.h"

  // Constructors
  RFID::RFID() {} // dummy for initialization
  //RFID::RFID(Stream *_serial_port, Led *_blinker, Reader *_reader) :
  RFID::RFID(Stream *_serial_port, Led *_blinker) :
    buff {},
    buff_index(0UL),
    proximity_state(0),
    current_ms(millis()),
    last_tag_read_ms(0UL),
    last_reader_power_cycle_ms(0UL),
    reader_power_cycle_high_duration(0UL),

    ms_since_last_tag_read(0UL),
    ms_since_last_reader_power_cycle(0UL),
    ms_reader_cycle_total(0UL),
    cycle_low_finish_ms(0UL),
    cycle_high_finish_ms(0UL),
    //tag_last_read_timeout_x_1000(0UL),
    
    serial_port(_serial_port),
    blinker(_blinker)
    //reader(_reader)
    
  { ; }

  //  RFID::~RFID() {
  //    delete reader;
  //  }

  void RFID::begin() {
    pinMode(S.OUTPUT_SWITCH_PIN, OUTPUT);
    pinMode(S.READER_POWER_CONTROL_PIN, OUTPUT);
    
    
    // Starts up with whatever state we left off in.
    // Protects against thief using 'admin' to move
    // in fits and starts, since a failed proximity
    // timeout will set S.proximity_state to 0.
    // This also tells the proximityStateController
    // where we left off at power-down (or reset).
    proximity_state = S.proximity_state;

    // Temp test to see if this class works with manually instanciated reader.
    //reader = new WL125; // I think this works.
    // This might work on the surface, but I don't think it allows each instance
    // to properly differentiate from the base class.
    //WL125 * reader = &WL125(); 

    // This must go after the Readers array is defined and initialized.
    ReaderArraySetup(); 

    // Sets local 'reader' to instance of Reader.
    reader = GetReader(S.DEFAULT_READER);

    // Switches the main load according to current proximity_state.
    // This turns on the load if saved prox-state was "on".
    // This begins the courtesey grace period until the system can
    // start processing tags (at which time, it will immediately
    // shut down output until a successful tag read).
    // TODO: Protect this action with a gate of some sort.
    //       Use a setting, or a pin, or a key-press.
    digitalWrite(S.OUTPUT_SWITCH_PIN, proximity_state);

    DPRINT(F("RFID::begin() reader->reader_name: "));
    DPRINTLN(reader->reader_name);
    Serial.print(F("Starting RFID reader "));
    Serial.print(reader->reader_name);
    Serial.print(F(", with EEPROM proximity state "));
    Serial.println(proximity_state);
    //  Serial.print(F(", and output switch pin: "));
    //  Serial.println(S.OUTPUT_SWITCH_PIN);
    
    //digitalWrite(S.OUTPUT_SWITCH_PIN, proximity_state);
    
    // Initializes the reader power/reset control.
    //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
    // This first one is to clear any built-up charge, as something is holding the reader low at startup.
    digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? LOW : HIGH);
    delay(50);
    digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? HIGH : LOW);
  }

  void RFID::loop() {
    // IMPORTANT: Some numbers use instance (global) vars, and some use functions.
    //   Which uses which might not make sense, but it's what works.
    //   Before finding the "magic" balance, I was getting calculation
    //   errors in the comparisons, maybe from corrupted memory/data,
    //   but I still don't really know what caused it.
    //   Anyway, be very careful if modifying the vars & functions of this class.

    
    DPRINT(F("*** RFID LOOP BEGIN "));
    current_ms = millis();
    DPRINT(current_ms); DPRINTLN(" ***");
    
    cycle_low_finish_ms = (uint32_t)(last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION);
    cycle_high_finish_ms = (uint32_t)(cycle_low_finish_ms + readerPowerCycleHighDuration()*1000UL);
    
    DPRINT(F("cycle_low_finish_ms: ")); DPRINTLN(cycle_low_finish_ms);
    DPRINT(F("cycle_high_finish_ms: ")); DPRINTLN(cycle_high_finish_ms);
    
    //ms_since_last_tag_read = (uint32_t)(current_ms - last_tag_read_ms);
    //ms_since_last_reader_power_cycle = (uint32_t)(current_ms - last_reader_power_cycle_ms);
    ms_reader_cycle_total = (uint32_t)(S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION*1000UL);
    //tag_last_read_timeout_x_1000 = (uint32_t)(S.TAG_LAST_READ_TIMEOUT*1000UL);

    
    /***  Displays most if not all local vars.       ***/
    /***  TODO: Should this be put into a function?  ***/
    /***                                             ***/
    DPRINT(F("last_tag_read_ms: "));
      DPRINTLN(last_tag_read_ms);
    DPRINT(F("last_reader_power_cycle_ms: "));
      DPRINTLN(last_reader_power_cycle_ms);
    DPRINT(F("msSinceLastTagRead(): "));
      DPRINTLN(msSinceLastTagRead());
    DPRINT(F("msSinceLastReaderPowerCycle(): "));
      DPRINTLN(msSinceLastReaderPowerCycle());
    DPRINT(F("ms_reader_cycle_total: "));
      DPRINTLN(ms_reader_cycle_total);
    DPRINT(F("tagLastReadTimeoutX1000(): "));
      DPRINTLN(tagLastReadTimeoutX1000());
    DPRINT(F("readerPowerCycleHighDuration()"))
      DPRINTLN(readerPowerCycleHighDuration());
    DPRINT(F("RPCHD*1000: "));
      DPRINTLN(readerPowerCycleHighDuration()*1000UL);
    DPRINT(F("S.READER_CYCLE_LOW_DURATION: "));
      DPRINTLN(S.READER_CYCLE_LOW_DURATION);
    DPRINTLN(F("***"));
    /***                                               ***/
    
    if (msSinceLastTagRead() > S.TAG_READ_SLEEP_INTERVAL) {
      // Checks the rfid reader for new data.
      pollReader();

      // Check fuel pump timeout on every loop.
      // TODO: Is this appropriate here? It was outside (below) the sleep block before.
      proximityStateController();
    }
  }

  uint32_t RFID::msSinceLastTagRead() {
    return (uint32_t)(current_ms - last_tag_read_ms);
  }

  uint32_t RFID::msSinceLastReaderPowerCycle() {
    return (uint32_t)(current_ms - last_reader_power_cycle_ms);
  }

  uint32_t RFID::msReaderCycleTotal() {
    //return S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + readerPowerCycleHighDuration() * 1000;
    //return S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION*1000;
    return (uint32_t)(S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION*1000UL);
  }

  uint32_t RFID::readerPowerCycleHighDuration() {
    if (reader_power_cycle_high_duration > 0UL) {
      return reader_power_cycle_high_duration;
    } else {
      return S.READER_CYCLE_HIGH_DURATION;
    }
  }

  uint32_t RFID::tagLastReadTimeoutX1000() {
    return (uint32_t)(S.TAG_LAST_READ_TIMEOUT*1000UL);
  }

  //  uint32_t RFID::cycleLowFinishMs() {
  //    return last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION;
  //  }
  //
  //  uint32_t RFID::cycleHighFinishMs() {
  //    return cycleLowFinishMs() + readerPowerCycleHighDuration()*1000;
  //  }

  

  // Polls reader serial port and processes incoming tag data.
  void RFID::pollReader() {
    // If data available on RFID serial port, do something.
    DPRINT(F("RFID::pollReader() reader->reader_name: "));
    DPRINTLN(reader->reader_name);    
    DPRINT(F("RFID::pollReader() reader->raw_tag_length: "));
    DPRINTLN(reader->raw_tag_length);
    
    if (serial_port->available()) {
      while (serial_port->available()) {
        if (buff_index >= MAX_TAG_LENGTH || buff_index >= reader->raw_tag_length) {
          resetBuffer();
          continue;
        }
        
        buff[buff_index] = serial_port->read();

        DPRINT(F("(")); DPRINT(buff_index); DPRINT(F(")"));
        if (
          (buff[buff_index] >= 48U && buff[buff_index] <= 57U) ||
          (buff[buff_index] >= 65U && buff[buff_index] <= 70U) ||
          (buff[buff_index] >= 97U && buff[buff_index] <= 102U)
        ) { DPRINT((char)buff[buff_index]); }
        
        DPRINT(":");
        DPRINT(buff[buff_index], HEX); DPRINT(":");
        DPRINT(buff[buff_index], DEC);

        // TODO: Move this to main RFID::loop() function?
        //uint8_t final_index = S.RAW_TAG_LENGTH - 1;
        uint8_t final_index = reader->raw_tag_length - 1U;
  
        if (buff_index == 0U && buff[0U] != 2U) { // reset bogus read
          resetBuffer();
        } else if (buff_index == final_index && buff[final_index] != 3U) { // reset bogus read
          resetBuffer();
          DPRINTLN("");
        } else if (buff_index > final_index || buff_index >= MAX_TAG_LENGTH) { // reset bogus read
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
      msSinceLastTagRead() > readerPowerCycleHighDuration() * 1000UL ||
      last_tag_read_ms == 0UL
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
    
    DPRINT(F("Tag result from Reader: "));
    DPRINTLN(tag_id);

    // assuming successful tag at this point?

    // If tag is valid, immediatly update proximity-state.
    if (tag_id > 0UL) {
      // Should this be the local function setProximityState()?
      //S.updateProximityState(1);
      setProximityState(1);
      last_tag_read_ms = current_ms;
      Serial.print(F("Valid tag: "));

    // Otherwise, don't do anything (not necessarily a failed proximity-state yet).
    } else {
      Serial.print(F("INVALID tag: "));
    }
    
    Serial.println(tag_id);
    
  }

  void RFID::resetBuffer() {
    buff_index = 0U;
    //strncpy(buff, NULL, reader->raw_tag_length);
    //strncpy(buff, NULL, MAX_TAG_LENGTH);
    //memcpy(buff, 0, MAX_TAG_LENGTH);
    memset(buff, 0, MAX_TAG_LENGTH);
  }

  void RFID::cycleReaderPower() {
    if (current_ms >= cycle_high_finish_ms || last_reader_power_cycle_ms == 0UL) {
      
      Serial.print(F("cycleReaderPower() tag read: "));
      if (last_tag_read_ms > 0UL) {
        //Serial.print((ms_since_last_tag_read)/1000UL);
        Serial.print(msSinceLastTagRead()/1000UL);
        Serial.print(F(" seconds ago"));
      } else {
        Serial.print(F("never"));
      }

      Serial.print(F(", reader cycled: "));
      if (last_reader_power_cycle_ms > 0UL) {
        //Serial.print((ms_since_last_reader_power_cycle)/1000UL);
        Serial.print(msSinceLastReaderPowerCycle()/1000UL);
        Serial.println(F(" seconds ago"));
      } else {
        Serial.println(F("never"));
      }

      DPRINTLN(F("cycleReaderPower() setting reader power LOW"));
            
      //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? LOW : HIGH);
      digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? LOW : HIGH);
      last_reader_power_cycle_ms = current_ms;
      
    } else if (current_ms >= cycle_low_finish_ms) {
      DPRINTLN(F("cycleReaderPower() setting reader power HIGH"));
      //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
      digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? HIGH : LOW);
    }
  }

  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void RFID::proximityStateController() {
    DPRINTLN(F("*** PROXIMITY ***"));
    DPRINT(F("last_tag_read_ms: ")); DPRINTLN(last_tag_read_ms);
    DPRINT(F("last_reader_power_cycle_ms: ")); DPRINTLN(last_reader_power_cycle_ms);
    DPRINT(F("msSinceLastTagRead(): ")); DPRINTLN(msSinceLastTagRead());
    DPRINT(F("msSinceLastReaderPowerCycle(): ")); DPRINTLN(msSinceLastReaderPowerCycle());
    DPRINT(F("ms_reader_cycle_total: ")); DPRINTLN(ms_reader_cycle_total);
    DPRINT(F("tagLastReadTimeoutX1000(): ")); DPRINTLN(tagLastReadTimeoutX1000());
    DPRINTLN(F("***"));
    
    // If NO TAG READ YET and reader has recently power cycled
    // This should probably calculate or use global setting for appropriate time-to-wait since last power cycle.
    if (
      last_tag_read_ms == 0UL &&
      last_reader_power_cycle_ms > 0UL &&
      msSinceLastReaderPowerCycle() > 2000UL
      ){
      
      DPRINTLN(F("proximityStateController() startup GRACE period timeout, no tag found"));
      blinker->SlowBlink();
      setProximityState(0);
    
    // If last read is beyond TIMEOUT, and we've cycled reader at least once in that interval.
    } else if (
      msSinceLastTagRead() > tagLastReadTimeoutX1000() &&
      last_reader_power_cycle_ms > 0UL &&
      msSinceLastTagRead() > msSinceLastReaderPowerCycle() &&
      msSinceLastReaderPowerCycle() > 2000UL
      ){
      
      DPRINTLN(F("proximityStateController() TIMEOUT"));
      blinker->SlowBlink();
      setProximityState(0);

    // If last read is greater than reader-power-cycle-total AND
    // less than final timeout total, we're in the AGING zone.
    } else if (
      last_tag_read_ms > 0UL &&
      msSinceLastTagRead() > ms_reader_cycle_total &&
      msSinceLastTagRead() <= tagLastReadTimeoutX1000()
      ){

      //DPRINTLN(F("### AGING ###"));
      //DPRINT(F("last_tag_read_ms: ")); DPRINTLN(last_tag_read_ms);
      //DPRINT(F("msSinceLastTagRead(): ")); DPRINTLN(msSinceLastTagRead());
      //DPRINT(F("ms_reader_cycle_total: ")); DPRINTLN(ms_reader_cycle_total);
      //DPRINT(F("tagLastReadTimeoutX1000(): ")); DPRINTLN(tagLastReadTimeoutX1000());
      //DPRINTLN(F("###  ###"));

      DPRINTLN(F("proximityStateController() AGING"));
      blinker->FastBlink();
      setProximityState(1);

    // If we're STILL YOUNG.
    } else if (
      last_tag_read_ms > 0UL &&
      msSinceLastTagRead() <= ms_reader_cycle_total
      ){
        
      DPRINTLN(F("proximityStateController() still YOUNG"));
      blinker->Steady();
      setProximityState(1);

    // No expected condition was met (not sure what to do here yet).
    } else {
      DPRINTLN(F("proximityStateController() no condition was met (not necessarily a problem)"));
    }

    digitalWrite(S.OUTPUT_SWITCH_PIN, proximity_state);
  }

  void RFID::setProximityState(int _state) {
    proximity_state = _state;
    
    if (proximity_state == 0) {
      reader_power_cycle_high_duration = 3UL;
    } else {
      reader_power_cycle_high_duration = 0UL;
    }
    
    S.updateProximityState(_state);
  }

  
