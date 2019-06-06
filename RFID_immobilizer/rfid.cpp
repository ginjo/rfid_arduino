#include "rfid.h"

  // constructor
  RFID::RFID(Stream *_serial_port, Led *_blinker) :
    buff({}),
    buff_index(0),
    last_tag_read_ms(0),
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
    digitalWrite(13, proximity_state);
    
    // Starts up the RFID reader.
    digitalWrite(S.READER_POWER_CONTROL_PIN, HIGH);
  }

  void RFID::loop() {
    unsigned long current_ms = millis();
    unsigned long ms_since_last_tag_read = current_ms - last_tag_read_ms;

    // TODO: Put this block in a pollReader() function.
    if (ms_since_last_tag_read > S.TAG_READ_INTERVAL) {
      
      if (serial_port->available()) {
        buff[buff_index] = serial_port->read();
  
        Serial.print("(");
        Serial.print(buff_index);
        Serial.print(")");
        Serial.print(buff[buff_index], HEX);
        Serial.print(":");
        Serial.print(buff[buff_index], DEC);
  
        int final_index = RAW_TAG_LENGTH - 1;
  
        if (buff_index == 0 && buff[0] != 2) { // reset bogus read
          resetBuffer();
        } else if (buff_index == final_index && buff[final_index] != 3) { // reset bogus read
          resetBuffer();
          Serial.println("");
        } else if (buff_index < final_index) { // good read, add comma to log and keep reading
          buff_index++;
          Serial.print(",");
        } else { // tag complete, now process it
          Serial.println("");
          processTagData(*buff);
          last_tag_read_ms = current_ms;
          resetBuffer();
        }
        
      } else if (ms_since_last_tag_read > S.READER_CYCLE_HIGH_DURATION) {
        cycleReaderPower();
      }
    }

    // Check fuel pump timeout on every loop
    proximityStateController();
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
    Serial.println((char *)tmp_str);
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
    unsigned long current_ms = millis();
    unsigned long ms_a = last_reader_power_cycle + S.READER_CYCLE_LOW_DURATION;
    unsigned long ms_b = last_reader_power_cycle + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION;
    
    //  Serial.print("cycleReaderPower() current, last, ms_a, ms_b: ");
    //  Serial.print(current_ms); Serial.print(",");
    //  Serial.print(last_reader_power_cycle); Serial.print(",");
    //  Serial.print(ms_a); Serial.print(",");
    //  Serial.println(ms_b);
    
    if (current_ms > last_reader_power_cycle && current_ms < ms_a) {
      //Serial.println("cycleReaderPower() setting reader power LOW");
      digitalWrite(S.READER_POWER_CONTROL_PIN, LOW);
    } else if (current_ms > ms_a && current_ms < ms_b) {
      //Serial.println(F("cycleReaderPower() setting reader power HIGH"));
      digitalWrite(S.READER_POWER_CONTROL_PIN, HIGH);
    } else if (current_ms > ms_b) {
      Serial.print(F("cycleReaderPower() updating last_reader_power_cycle, last tag read: "));
      Serial.print((current_ms - last_tag_read_ms)/1000);
      Serial.println(F(" seconds ago"));
      last_reader_power_cycle = millis();
    }
  }

  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void RFID::proximityStateController() {
    unsigned long current_ms = millis();
    unsigned long ms_since_last_tag_read = current_ms - last_tag_read_ms;

    // if last read was too long ago
    if (ms_since_last_tag_read > S.TAG_LAST_READ_TIMEOUT * 1000) {

      //Serial.println(F("proximityStateController() in 'timeout' stage"));
        
      int slow_blink[INTERVALS_LENGTH] = {500,500};
      blinker->update(0, slow_blink);
      setProximityState(0);

    // if last read was less than final timeout but greater than the first reader-power-cycle.
    // basically, if we're in the "aging" zone.
    } else if (ms_since_last_tag_read <= (S.TAG_LAST_READ_TIMEOUT * 1000) &&
        ms_since_last_tag_read > S.TAG_READ_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION
      ) {

      //Serial.println(F("proximityStateController() in 'aging' stage"));

      if (proximity_state == 0 || last_tag_read_ms == 0) {
        int slow_blink[INTERVALS_LENGTH] = {500,500};
        blinker->update(0, slow_blink);
        setProximityState(0);
      } else if (S.proximity_state == 1) {
        int fast_blink[INTERVALS_LENGTH] = {70,70};
        blinker->update(0, fast_blink);
        setProximityState(1);
      }

    // if we're still young (haven't had time for a reader-power-cycle yet)..
    } else if (ms_since_last_tag_read <= S.TAG_READ_INTERVAL + S.READER_CYCLE_LOW_DURATION + S.READER_CYCLE_HIGH_DURATION) {
      
      if (S.proximity_state == 1) {
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
