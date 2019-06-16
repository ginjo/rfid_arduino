#include "rfid.h"

  // constructor
  RFID::RFID(Stream *_serial_port, Led *_blinker) :
    buff({}),
    buff_index(0),
    current_ms(millis()),
    last_tag_read_ms(0),
    last_reader_power_cycle_ms(0),
    serial_port(_serial_port),
    blinker(_blinker),
    proximity_state(0)
  {
    ;
  }

  void RFID::begin() {
    // Starts up with whatever state we left off in.
    // Protects against thief using 'admin' to move
    // in fits and starts, since a failed proximity
    // timeout will set S.proximity_state to 0.
    // This also tells the proximityStateController
    // where we left off at power-down (or reset).
    proximity_state = S.proximity_state;

    Serial.print(F("Starting RFID reader with proximity state: "));
    Serial.println(proximity_state);
    
    digitalWrite(13, proximity_state);
    
    // Starts up the RFID reader.
    digitalWrite(S.READER_POWER_CONTROL_PIN, HIGH);
  }

  void RFID::loop() {
    current_ms = millis();
    
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

  unsigned long RFID::msSinceLastTagRead() {
    unsigned long result = current_ms - last_tag_read_ms;

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

  unsigned long RFID::msSinceLastReaderPowerCycle() {
    return current_ms - last_reader_power_cycle_ms;
  }

  unsigned long RFID::msReaderCycleTotal() {
    return S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION * 1000;
  }

  void RFID::pollReader() {
    if (serial_port->available()) {
      while (serial_port->available()) {
        buff[buff_index] = serial_port->read();
  
        //  DPRINT("(");
        //  DPRINT(buff_index);
        //  DPRINT(")");
        //  DPRINT(buff[buff_index], HEX);
        //  DPRINT(":");
        //  DPRINT(buff[buff_index], DEC);
  
        uint8_t final_index = RAW_TAG_LENGTH - 1;
  
        if (buff_index == 0 && buff[0] != 2) { // reset bogus read
          resetBuffer();
        } else if (buff_index == final_index && buff[final_index] != 3) { // reset bogus read
          resetBuffer();
          //Serial.println("");
        } else if (buff_index < final_index) { // good read, add comma to log and keep reading
          buff_index++;
          //Serial.print(",");
        } else { // tag complete, now process it
          //Serial.println("");
          processTagData(buff);
          last_tag_read_ms = current_ms;
          resetBuffer();
          return;
        }
      }

    } else if (
      msSinceLastTagRead() > S.READER_CYCLE_HIGH_DURATION * 1000 ||
      last_tag_read_ms == 0
    ) {
      cycleReaderPower();
    }    
  }

  void RFID::processTagData(uint8_t _tag[RAW_TAG_LENGTH]) {    
    uint8_t id_begin;
    uint8_t id_end;
     
    if (RAW_TAG_LENGTH == 14) {  // RDM6300 reader
      Serial.println((char *)_tag);
      //Serial.println(strtol(buff, NULL, 16));
      id_begin = 3;
      id_end   = 10;
      
    } else if (RAW_TAG_LENGTH == 10) {  // 7941E reader
      id_begin = 4;
      id_end   = 7;
    }
  
    uint8_t id_len = id_end - id_begin;
    char tmp_str[id_len] = "";
    
    for(int n=id_begin; n<=id_end; n++) {
      sprintf(tmp_str + strlen(tmp_str), "%x", _tag[n]);
    }

    Serial.print(F("Tag success: "));
    Serial.print((char *)tmp_str);
    Serial.print(", ");
    Serial.print(strtol((char *)tmp_str, NULL, 16));
    Serial.print(F(", at ms "));
    Serial.println(current_ms);
    
    strncpy(tmp_str, NULL, id_len);

    // assuming successful tag at this point
    S.updateProximityState(1);
  }

  void RFID::resetBuffer() {
    buff_index = 0;
    strncpy(buff, NULL, RAW_TAG_LENGTH);
  }

  void RFID::cycleReaderPower() {
    unsigned long cycle_low_finish_ms = last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION;
    unsigned long cycle_high_finish_ms = cycle_low_finish_ms + S.READER_CYCLE_HIGH_DURATION * 1000;

    DPRINT(F("cycleReaderPower() current_MS, last_tag_read, last_p_cycle, cycle_low_finish_ms, S.READER_CYCLE_HIGH_DURATION, RCHD * 1000, cycle_high_finish_ms: "));
    DPRINT(current_ms); DPRINT(",");
    DPRINT(last_tag_read_ms); DPRINT(",");
    DPRINT(last_reader_power_cycle_ms); DPRINT(",");
    DPRINT(cycle_low_finish_ms); DPRINT(",");
    DPRINT(S.READER_CYCLE_HIGH_DURATION); DPRINT(",");
    DPRINT(S.READER_CYCLE_HIGH_DURATION * 1000); DPRINT(",");
    DPRINTLN(cycle_high_finish_ms);
    
    if (current_ms >= cycle_high_finish_ms || last_reader_power_cycle_ms == 0) {
      DPRINTLN(F("cycleReaderPower() setting reader power LOW"));
      
      Serial.print(F("cycleReaderPower() tag read: "));
      if (last_tag_read_ms > 0) {
        Serial.print((msSinceLastTagRead())/1000);
        Serial.print(F(" seconds ago"));
      } else {
        Serial.print(F("Never!"));
      }

      Serial.print(F(", reader cycled: "));
      if (last_reader_power_cycle_ms > 0) {
        Serial.print((msSinceLastReaderPowerCycle())/1000);
        Serial.println(F(" seconds ago"));
      } else {
        Serial.println(F("Never!"));
      }
      
      last_reader_power_cycle_ms = current_ms;
      digitalWrite(S.READER_POWER_CONTROL_PIN, LOW);
      
    } else if (current_ms >= cycle_low_finish_ms) {
      DPRINTLN(F("cycleReaderPower() setting reader power HIGH"));
      digitalWrite(S.READER_POWER_CONTROL_PIN, HIGH);
    }
  }

  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void RFID::proximityStateController() {
    DPRINT(F("proximityStateController() msSinceLastTagRead, msSinceLastReaderPowerCycle, last-tag-read, last-p-cycle: "));
    DPRINT(msSinceLastTagRead()); DPRINT(",");
    DPRINT(msSinceLastReaderPowerCycle()); DPRINT(",");
    DPRINT(last_tag_read_ms); DPRINT(",");
    DPRINTLN(last_reader_power_cycle_ms);
    

    // If no tag-read yet and reader has recently power cycled
    // This should probably calculate or use global setting for appropriate time-to-wait since last power cycle.
    if (last_tag_read_ms == 0 && last_reader_power_cycle_ms > 0 && msSinceLastReaderPowerCycle() > 2000) {
      
      DPRINTLN(F("proximityStateController() startup grace period timeout, no tag found"));
      blinker->slowBlink();
      setProximityState(0);
    
    // if last read was too long ago, and we've cycled reader at least once in that interval.
    } else if (msSinceLastTagRead() > S.TAG_LAST_READ_TIMEOUT * 1000 && last_reader_power_cycle_ms > 0 && msSinceLastTagRead() > msSinceLastReaderPowerCycle()) {
      
      DPRINTLN(F("proximityStateController() timeout"));
      blinker->slowBlink();
      setProximityState(0);

    // If last read is greater than reader-power-cycle-total AND less than final timeout total,
    // we're in the "aging" zone.
    } else if (last_tag_read_ms > 0 && msSinceLastTagRead() > msReaderCycleTotal() && msSinceLastTagRead() <= (S.TAG_LAST_READ_TIMEOUT * 1000)) {

      DPRINTLN(F("proximityStateController() aging"));
      blinker->fastBlink();
      setProximityState(1);

    // If we're still young.
    } else if (last_tag_read_ms > 0 && msSinceLastTagRead() <= msReaderCycleTotal()) {
        DPRINTLN(F("proximityStateController() still young"));
        blinker->on();
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
    S.updateProximityState(_state);
  }

  
