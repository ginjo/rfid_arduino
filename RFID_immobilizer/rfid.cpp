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

    // TODO: Put this block (or at least part of it) in a pollReader() function.
    // Then create a local while() loop to gather all avail serial data
    // and pass it to the function, all in the same processor loop cycle.
    
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

    //  Serial.print(F("RFID::msSinceLastTagRead(): last-read-ms-ago, curr-ms, last-ms: "));
    //  Serial.print(result);
    //  Serial.print(", ");
    //  Serial.print(current_ms);
    //  Serial.print(", ");
    //  Serial.println(last_tag_read_ms);
    
    //  Serial.print("RFID::msSinceLastTagRead(): ");
    //  Serial.println(result);

    return result;
  }

  void RFID::pollReader() {
    if (serial_port->available()) {
      buff[buff_index] = serial_port->read();

      //  Serial.print("(");
      //  Serial.print(buff_index);
      //  Serial.print(")");
      //  Serial.print(buff[buff_index], HEX);
      //  Serial.print(":");
      //  Serial.print(buff[buff_index], DEC);

      int final_index = RAW_TAG_LENGTH - 1;

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
        processTagData(*buff);
        last_tag_read_ms = current_ms;
        resetBuffer();
      }
      
    } else if (msSinceLastTagRead() > S.READER_CYCLE_HIGH_DURATION) {
      cycleReaderPower();
    }    
  }

  void RFID::processTagData(uint8_t * _tag[RAW_TAG_LENGTH]) {    
    int id_begin;
    int id_end;
     
    if (RAW_TAG_LENGTH == 14) {  // RDM6300 reader
      Serial.println((char *)_tag);
      //Serial.println(strtol(buff, NULL, 16));
      id_begin = 3;
      id_end   = 10;
      
    } else if (RAW_TAG_LENGTH == 10) {  // 7941E reader
      id_begin = 4;
      id_end   = 7;
    }
  
    int id_len = id_end - id_begin;
    char tmp_str[id_len] = "";
    
    for(int n=id_begin; n<=id_end; n++) {
      sprintf(tmp_str + strlen(tmp_str), "%x", _tag[n]);
    }
    
    Serial.print((char *)tmp_str);
    Serial.print(", ");
    Serial.println(strtol((char *)tmp_str, NULL, 16));
    strncpy(tmp_str, NULL, id_len);

    // assuming successful tag at this point
    //blinker->on(); // moved to proximityStateController experimentally
    S.updateProximityState(1);
  }

  void RFID::resetBuffer() {
    buff_index = 0;
    strncpy(buff, NULL, RAW_TAG_LENGTH);
  }

  void RFID::cycleReaderPower() {
    unsigned long cycle_low_finish_ms = last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION;
    unsigned long cycle_high_finish_ms = last_reader_power_cycle_ms + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION;
    
    //  Serial.print(F("cycleReaderPower() current, last, cycle_low_finish_ms, cycle_high_finish_ms: "));
    //  Serial.print(current_ms); Serial.print(",");
    //  Serial.print(last_reader_power_cycle_ms); Serial.print(",");
    //  Serial.print(cycle_low_finish_ms); Serial.print(",");
    //  Serial.println(cycle_high_finish_ms);
    
    if (current_ms >= cycle_high_finish_ms || last_reader_power_cycle_ms == 0) {
      // Serial.println("cycleReaderPower() setting reader power LOW");
      Serial.print(F("cycleReaderPower(), last tag read: "));
      Serial.print((msSinceLastTagRead())/1000);
      Serial.println(F(" seconds ago"));
      last_reader_power_cycle_ms = current_ms;
      digitalWrite(S.READER_POWER_CONTROL_PIN, LOW);
    } else if (current_ms >= cycle_low_finish_ms) {
      // Serial.println(F("cycleReaderPower() setting reader power HIGH"));
      digitalWrite(S.READER_POWER_CONTROL_PIN, HIGH);
    } //else if (current_ms > cycle_high_finish_ms) {
    //  Serial.print(F("cycleReaderPower() updating last_reader_power_cycle_ms, last tag read: "));
    //  Serial.print((msSinceLastTagRead())/1000);
    //  Serial.println(F(" seconds ago"));
    //  last_reader_power_cycle_ms = current_ms;   //millis();
    //}
  }

  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void RFID::proximityStateController() {
    // if last read was too long ago
    if (msSinceLastTagRead() > S.TAG_LAST_READ_TIMEOUT * 1000) {

      //Serial.println(F("proximityStateController() in 'timeout'"));
        
      blinker->slowBlink();
      setProximityState(0);

    // if last read was less than final timeout but greater than the first reader-power-cycle.
    // basically, if we're in the "aging" zone.
    } else if (msSinceLastTagRead() <= (S.TAG_LAST_READ_TIMEOUT * 1000) &&
        msSinceLastTagRead() > S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION
      ) {

      //Serial.println(F("proximityStateController() in 'aging'"));

      if (proximity_state == 0 || last_tag_read_ms == 0) {
        //Serial.println(F("proximityStateController() aging with no previous tag read"));
        blinker->slowBlink();
        setProximityState(0);
      } else if (S.proximity_state == 1) {
        //Serial.println(F("proximityStateController() aging with S.proximity_state == 1"));
        blinker->fastBlink();
        setProximityState(1);
      }

    // if we're still young (haven't had time for a reader-power-cycle yet)..
    } else if (msSinceLastTagRead() <= S.TAG_READ_SLEEP_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION) {
      
      if (S.proximity_state == 1) {
        //Serial.println(F("proximityStateController() still young with S.proximity_state == 1"));
        blinker->on();
        setProximityState(1);
      }
    } else {
      Serial.println(F("proximityStateController() no condition was met (not necessarily a problem)"));
    }

    // TODO: Replace 13 with a S.<setting>
    digitalWrite(13, proximity_state);
  }

  void RFID::setProximityState(int _state) {
    proximity_state = _state;
    S.updateProximityState(_state);
  }

  
