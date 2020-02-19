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

    buff {},
    buff_index(0UL),
    current_ms(millis()),
    tag_last_read_ms(0UL),
    last_power_cycle_ms(0UL),
    power_cycle_high_duration_override_ms(0UL),
    cycle_low_finish_ms(0UL),
    //cycle_high_finish_ms(0UL),
    tag_last_read_id(0UL),
    serial_port(NULL) // experimental, so that serial_port is initialized
        
  {
    LOG(4, F("Loading reader "));
  }



  /*  Instance functions  */

  char *Reader::name() {return (char *)Name;}

  uint32_t Reader::msSinceLastTagRead() {
    return (uint32_t)(current_ms - tag_last_read_ms);
  }

  uint32_t Reader::msSinceLastPowerCycle() {
    return (uint32_t)(current_ms - last_power_cycle_ms);
  }

  uint32_t Reader::tagLastReadHardTimeoutX1000() {
    return (uint32_t)(S.tag_last_read_hard_timeout*1000UL);
  }

  uint32_t Reader::cycleHighFinishMs() {
    //return cycle_low_finish_ms + powerCycleHighDurationMs()*1000UL;
    return cycle_low_finish_ms + powerCycleHighDurationMs();
  }

  uint32_t Reader::processTagData(uint8_t[]) {
    LOG(2, F("Base Reader called processTagData"), true);
    return 0UL;
  }

  void Reader::resetBuffer() {
    // RD_LOG(6, "Reader::resetBuffer", true); // This makes a mess of other log items.
    buff_index = 0U;
    //strncpy(buff, NULL, reader->raw_tag_length);
    //strncpy(buff, NULL, MAX_TAG_LENGTH);
    //memcpy(buff, 0, MAX_TAG_LENGTH);
    memset(buff, 0U, MAX_TAG_LENGTH);
  }


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
    
    // TODO: Should this be put into a function? Yes, I think so!
    RD_LOG(6, F("tag_last_read_ms "), false); RD_LOG(6, tag_last_read_ms, true);
    RD_LOG(6, F("msSinceLastTagRead "), false); RD_LOG(6, msSinceLastTagRead(), true);
    RD_LOG(6, F("last_power_cycle_ms "), false); RD_LOG(6, last_power_cycle_ms, true);
    RD_LOG(6, F("msSinceLastPowerCycle "), false); RD_LOG(6, msSinceLastPowerCycle(), true);
    RD_LOG(6, F("powerCycleHighDurationMs "), false); RD_LOG(6, powerCycleHighDurationMs(), true);
    RD_LOG(6, F("power_cycle_high_duration_override_ms "), false); RD_LOG(6, power_cycle_high_duration_override_ms, true);
    RD_LOG(6, F("cycle_low_finish_ms "), false); RD_LOG(6, cycle_low_finish_ms, true);
    RD_LOG(6, F("cycleHighFinishMs "), false); RD_LOG(6, cycleHighFinishMs(), true);

    // Limits reader serial port polling to once per tag_read_sleep_interval (1000 ms).
    if (msSinceLastTagRead() > S.tag_read_sleep_interval) {
      pollReader();
    }

    RD_LOG(6, "tag_last_read_id ", false); RD_LOG(6, tag_last_read_id, true);
    RD_LOG(6, F("READER LOOP END "), false); RD_LOG(6, millis(), true);
  }


  /*
    Gets calculated value as ms. Accepts a multiplier that if > 1
    multiplies the current override before doing the logic calcs.
    This is how the progressive power cycle growth/decay works.

    Don't just run this function and multiply the result.
    Always pass in the multiplier, if you intend to change the override.
    
    Gets the lowest of three values (a good example of generic technique
    for that). Pass in a multiplier (int8_t) to get a changed value that
    falls within the confines of the prescribed logic.

    The multiplier represents a a percentage, or decimal with 2 places,
    so 150 == 150% or 1.5. This is fixed point math... to avoid floating point math.
    With uint8_t, the highest you can go is 255, so 255/100 == 2.55 multiplier (255%) max.
    Use this graphing calc to visualize: https://www.desmos.com/calculator/kbbcqwqqwt
    The formula for growth is y = n(f)^x, where n is starting point (usually 1),
    x is current itteration (time, in the generic sense), and f is
    the growth factor. Y is the result that beomes the new itteration x.

    For the purposes of this application, if the multiplier (growth factor) is < 100,
    you are talking fractional multiplier, which gives you decay instead of growth.
  */
  uint32_t Reader::powerCycleHighDurationMs(uint8_t multiplier) {
    if (power_cycle_high_duration_override_ms != 0UL) {
      uint32_t o = power_cycle_high_duration_override_ms * multiplier / 100;
      uint32_t s = S.tag_last_read_soft_timeout * 1000UL;
      uint32_t m = S.reader_cycle_high_max * 1000UL;

      if (o <= s-1 && o <= m)         o = o;
      else if (s-1 <= o && s-1 <= m)  o = s-1000UL;
      else                            o = m;

      return (o < 1000UL ? 1000UL : o);
    } else {
      return S.tag_last_read_soft_timeout * 1000UL;
    }
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
    /*
      If no data on Controller serial port and sufficient conditions exist,
      then call cycle-reader-power.

      The divide by 2 reduces short-term (by half) interval duration,
      if manual tag reads have been made after half the of the current
      cycle duration has passed. Then the duration goes back to the set
      override within one cycle. This is not necessary but is not harmful
      and could be helpful. Without the divide by 2, the reader will always
      wait to cycle until a tag has not been read for the cycle-high duration.

      TODO: Should this condition be inside the cycleReaderPower() function,
      instead of here (leaving this as just a simple 'else')?
      If this condition were moved to cycleReaderPower, it would wrap
      the rest of that function. OR is there a reason
      to keep this logic out of that function? Is the function used
      for anything different or called from anywhere else? (No, not used anywehre else right now).
    */
      //msSinceLastTagRead() > (powerCycleHighDurationMs() * 1000UL)/2 ||
      msSinceLastTagRead() > powerCycleHighDurationMs()/2 ||
      tag_last_read_ms == 0UL // ||         // if tag has never been read.
      //current_ms >= cycle_low_finish_ms  // to make sure cycleReader is called when it needs to go HIGH from LOW.
    ) 
    {
      // This log line puts out a LOT of data but can be useful.
      //RD_LOG(6, F("poll-calling-cycl "), false); RD_LOG(6, msSinceLastTagRead(), false); RD_LOG(6, F(" "), false); RD_LOG(6, powerCycleHighDurationMs() * 1000UL, true); 
      cycleReaderPower();
    }
  }


  /*
    TODO (Reader): I think part of this function should stay in Controller,
    the part that decides what to do with a successfull tag read/authentication.
    The big question is: Where/what do we do with the successfully read tag? Where do we store it?
    
    Processes a received array of tag bytes.
    NOTE (from Controller): array args in func definitions can only use absolute constants, not vars.
  */
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

    // Calls reader-specific process code.
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
      LOG(4, F("Authorized tag xx"));

    } else {
    // Otherwise, don't do anything (not necessarily a failed proximity-state yet).
    
      //LOG(4, F("Unauthorized or invalid tag: "));
      LOG(4, F("Tag not valid/authorized xx"));
    }

    // This mask works but takes a couple hundred bytes of progmem!
    //char str[10] = {};
    //sprintf(str, "xxxxxx%4lu", tag_id % 10000);
    //LOG(4, str, true);

    // This mask works and takes less progmem.
    // This is a good example of using remainders from dividing by
    // 10, 100, 1000, etc. to get the right-most digits of a larger number.
    LOG(4, tag_id % 10000, true);

    // Stores the tag id in an instance variable.
    tag_last_read_id = tag_id;
    
  } // processTag()


  /*
    If conditions are right, sets the reader power low. Then, during
    a future pass thru the loop, sets reader power high again.
  */
  void Reader::cycleReaderPower() {
    if (current_ms >= cycleHighFinishMs() || last_power_cycle_ms == 0UL) {
    // Sets reader power Off (LOW or HIGH electrical state dependent on reader type).
    
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
      
      LOG(5, F(", interval ms "));
      LOG(5, powerCycleHighDurationMs(), true);
      

      RD_LOG(6, F("Reader LOW"), true);

      /*
        If power_cycle_high_duration_override_ms is not 0,
        call powerCycleHighDurationMs() with a factor of 2,
        which will increment power_cycle_high_duration_override_ms by doubling it.

        TODO: Should the multiplier here be a user-setting?
              Would the user ever have any reason to adjust the multiplier?
      */
      if (power_cycle_high_duration_override_ms) {
        power_cycle_high_duration_override_ms = powerCycleHighDurationMs(150); // 150 == 150% == 1.5 multiplier.
      }
            
      digitalWrite(READER_POWER_CONTROL_PIN, power_control_logic ? LOW : HIGH);
      last_power_cycle_ms = current_ms;
      
    } else if (current_ms >= cycle_low_finish_ms) {
    // Sets reader power On (HIGH or LOW electrical state dependent on reader type).
    
      // This log line will produce a LOT of output but can be useful for troubleshooting.
      //RD_LOG(6, F("cycleReaderPower setting HIGH, high-dur "), false); RD_LOG(6, powerCycleHighDurationMs, true);
      
      digitalWrite(READER_POWER_CONTROL_PIN, power_control_logic ? HIGH : LOW);
    }
  }

  
