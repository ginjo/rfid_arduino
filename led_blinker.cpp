  #include "led_blinker.h"
  #include "logger.h"



  // Static pre-defined 2D interval array.
  const int Led::StaticIntervals[][INTERVALS_LENGTH] = {
    {1000},
    {0},
    {500,500},
    {80,80},
    {470,30},
    {70,0},
    {30,70},
    {200, 200},
  };


  void Led::PrintStaticIntervals() {
    // TODO: Should this be converted to use LOG function?
    HWserial->println(F("PrintStaticIntervals()"));
    for (int n = 0; n < 6; n++) {
      HWserial->print(n); HWserial->print(":");
      for (int m = 0; m < INTERVALS_LENGTH; m++) {
        HWserial->print(StaticIntervals[n][m]);
        HWserial->print(",");
      }
      HWserial->println("");
    }
  }


  Led::Led(int pin, const char _name[], const int _freq, const int _pwm) : 
    led_pin(pin),
    led_state(0),
    current_phase(0),
    cycle_count(0U),
    num_cycles(0U),
    current_ms(millis()),
    previous_ms(0UL),
    frequency(_freq),
    pwm(_pwm),
    intervals {StaticIntervals[1]}, // initializes to the "off()" interval set (all zeros).
    signature {}
  {
    //strlcpy(led_name, _name, 3);
    snprintf(led_name, 3, "%s", _name);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);
  }


  // Starts a new pattern (interval set) with num cycles, and/or freq and pwm
  //void Led::begin(const int _num_cycles, const int _intervals[], const int _freq, const int _pwm) {
  void Led::begin(const uint16_t _num_cycles, const int intervals_index, const int _freq, const int _pwm) {
    
    BK_LOG(5, F("Led.begin() "), false); BK_LOG(5, led_name, true);

    reset();
    
    num_cycles = _num_cycles;
    intervals = StaticIntervals[intervals_index];
    if (_freq >=0) frequency = _freq;
    if (_pwm >=0) pwm = _pwm;

    if (LogLevel() >= 5U) {
      printIntervals(intervals);
    }

    startPhase(0);
  }
  
  void Led::loop() {
    current_ms = millis();
    handleBlinker();
  }


  // Calls begin() only if params have changed.
  // Should generally use this instad of begin().
  //void Led::update(const int _num_cycles, const int _intervals[], const int _freq, const int _pwm) {
  void Led::update(const uint16_t _num_cycles, const int intervals_index, const int _freq, const int _pwm) {
    
    BK_LOG(6, F("Led.update() "), false); BK_LOG(6, led_name, true);
    #ifdef BK_DEBUG
      if (LogLevel() >= 6U) {
        // TODO: Find a way to get _num_cycles into printIntervals. 
        printIntervals(intervals);
        printIntervals(StaticIntervals[intervals_index]);
      }
    #endif

    // TODO: This isn't re-starting a pattern if the same pattern exists but expired (cycle_count > num_cycles).
    //       Should this look at the signature?
    //       Should the signature include any/all of these vars?
    //
    // Skips calling begin() if nothing has changed.
    if (
        _num_cycles == num_cycles &&
        (_freq >= 0 ? _freq == frequency : true) &&
        (_pwm >= 0 ? _pwm == pwm : true) &&
        (
          //memcmp(_intervals, intervals, INTERVALS_LENGTH) == 0 ||
          intervals == StaticIntervals[intervals_index] || // Are these the same memory address?
          (countIntervals(StaticIntervals[intervals_index]) == 0 && countIntervals(intervals) == 0)
        )
      )
    {
      BK_LOG(6, F("Led.update() skip"), true);
    } else {
      begin(_num_cycles, intervals_index, _freq, _pwm);      
    }
  }

  
  // Starts a new blinker phase, given int
  void Led::startPhase(int phz) {
        
    BK_LOG(6, "Led.startPhase ", false);
    BK_LOG(6, led_name, false);
    BK_LOG(6, " ", false);
    BK_LOG(6, phz, true);
    
    current_phase = phz;
    
    if (intervals[current_phase] > 0) {
      // Gives remainder of 0 or 1, so odd phases are state=1, and even phases are state=0
      led_state = (current_phase + 1) % 2;
    } else {
      led_state = 0;
    }

    // TODO: Should this set previous_ms to fresh millis() (cuz current_ms might be old here)?
    //       Or should this go AFTER the call to handleBlinker() below?
    previous_ms = current_ms = millis();
    
    // This is here to get an early start on processing the output.
    // Otherwise it could be milliseconds (or seconds!) before the main
    // loop gets around to handlBlinker(), and then we would have missed
    // one or more phases. This is important for blinker/beeper intervals
    // that are relatively short, for example 70ms.
    handleBlinker();
  }


  // Handles start-stop blinker and blinker cycling
  void Led::handleBlinker() {

    if (current_ms - previous_ms > (unsigned long)intervals[current_phase]) {
    // If the current interval has expired.
    
      if (current_phase < countIntervals(intervals) - 1) {
      // If we still have more intervals to run
      
        startPhase(current_phase + 1);
      
      } else if (num_cycles > 0) {
      // If we still have more cycles to run
        cycle_count ++;

        if (cycle_count < num_cycles) {
          startPhase(0);
        } else {
          off();
        }
        
      } else if (num_cycles == 0) {
        startPhase(0);
      }

      // startPhase() already calls handleBlinker(),
      // so we don't need to run it again immediately.
      return;
    }
    

    // Sets local debug level for the next chunk of code.
    int lv = 5;

    // Only writes to pins if data has changed, not every loop.
    if (signature[0] != led_pin || signature[1] != led_state || signature[2] != frequency || signature[3] != pwm) {
      if (frequency == 0 && pwm == 0U) {
        logWriteOneLine(lv);
        digitalWrite(led_pin, led_state);
      } else if (frequency > 0) {
        logWriteOneLine(lv);
        led_state ? tone(led_pin, frequency) : noTone(led_pin);
      } else if (pwm > 0) {
        logWriteOneLine(lv);
        led_state ? analogWrite(led_pin, pwm) : analogWrite(led_pin, 0U);
      }
    }

    logData(6);

    signature[0] = (int)led_pin;
    signature[1] = (int)led_state;
    signature[2] = (int)frequency;
    signature[3] = (int)pwm;
    
  } // handleBlinker()


  int Led::countIntervals(const int _intervals[]) {
    int n;
    for (n = 0; n < INTERVALS_LENGTH; n ++) {
      //BK_PRINT(F("_intervals: ")); BK_PRINT(n); BK_PRINT(F(" ")); BK_PRINTLN(_intervals[n]);
      if (_intervals[n] <= 0) {
        break;
      }
    }
    return n;
  }


  void Led::printIntervals(const int _intervals[]) {
    if (LogLevel() < 5U) return;
    BK_LOG(5, num_cycles, false);
    BK_LOG(5, ",", false);
    for (int n = 0; n < INTERVALS_LENGTH; n ++) {
      BK_LOG(5, " ", false);
      BK_LOG(5, _intervals[n], false);
    }
    BK_LOG(5, "", true);
  }


  void Led::logData(const int _lv) {
    if (LogLevel() >= _lv) {
      BK_LOG(_lv, "", true);
      BK_LOG(_lv, F("Led Dat "), false); BK_LOG(_lv, led_name, true);
      BK_LOG(_lv, F("cycle_count "), false); BK_LOG(_lv, cycle_count, true);
      BK_LOG(_lv, F("current_phase "), false); BK_LOG(_lv, current_phase, true);
      BK_LOG(_lv, F("interval "), false); BK_LOG(_lv, intervals[current_phase], true);
      BK_LOG(_lv, F("led_state "), false); BK_LOG(_lv, led_state, true);
      BK_LOG(_lv, F("current_ms "), false); BK_LOG(_lv, current_ms, true);
      BK_LOG(_lv, F("previous_ms "), false); BK_LOG(_lv, previous_ms, true);
      BK_LOG(_lv, "", true);
    }
  }


  void Led::logWriteOneLine(const int _lv) {
    if (LogLevel() >= _lv) {
      BK_LOG(_lv, F("LED write "), false);
      BK_LOG(_lv, led_name, false);
      BK_LOG(_lv, F(" pn"), false);
      BK_LOG(_lv, led_pin, false);
      BK_LOG(_lv, F(" st"), false);
      BK_LOG(_lv, led_state, false);
      BK_LOG(_lv, F(" fq"), false);
      BK_LOG(_lv, frequency, false);
      BK_LOG(_lv, F(" pw"), false);
      BK_LOG(_lv, pwm, false);
      BK_LOG(_lv, F(" in"), false);
      BK_LOG(_lv, intervals[current_phase], false);
      BK_LOG(_lv, F(" ph"), false);
      BK_LOG(_lv, current_phase, false);
      BK_LOG(_lv, F(" cy"), false);
      BK_LOG(_lv, cycle_count, false);
      //  BK_LOG(_lv, F(" pv"), false);
      //  BK_LOG(_lv, previous_ms, false);
      BK_LOG(_lv, F(" cr"), false);
      BK_LOG(_lv, current_ms, true);
           
    }
  }
  

  // TODO: How deeply does this reset the Led? Should it reset intervals too? No, I don't think so.
  void Led::reset() {
    led_state = 0;
    current_phase = 0;
    cycle_count = 0U;
  }

  // Manually forces led state low
  void Led::go_low() {
    led_state = LOW;
    digitalWrite(led_pin, LOW);
  }

  // Manually forces led state high
  void Led::go_high() {
    led_state = HIGH;
    digitalWrite(led_pin, HIGH);
  }


  /*  Pattern presets  */
  
  void Led::steady() {
    BK_LOG(6, F("Led.steady() "), false); BK_LOG(6, led_name, true);
    update(0, 0);
  }

  void Led::off() {
    BK_LOG(6, F("Led.off() "), false); BK_LOG(6, led_name, true);
    update(0, 1);
  }

  void Led::slowBlink() {
    BK_LOG(6, F("Led.slowBlink() "), false); BK_LOG(6, led_name, true);
    update(0, 2);
  }

  void Led::fastBlink() {
    BK_LOG(6, F("Led.fastBlink() "), false); BK_LOG(6, led_name, true);
    update(0, 3);
  }

  void Led::startupBlink() {
    BK_LOG(6, F("Led.startupBlink() "), false); BK_LOG(6, led_name, true);
    update(0, 4);
  }

  void Led::once() {
    BK_LOG(6, F("Led.once() "), false); BK_LOG(6, led_name, true);
    update(1, 5);
  }

  void Led::fastBeep(uint16_t _count) {
    BK_LOG(6, F("Led.fastBeep() "), false); BK_LOG(6, led_name, false); BK_LOG(6, F(" "), false); BK_LOG(6, _count, true);
    update(_count, 3);
  }

  void Led::slowBeep(uint16_t _count) {
    BK_LOG(6, F("Led.slowBeep() "), false); BK_LOG(6, led_name, false); BK_LOG(6, F(" "), false); BK_LOG(6, _count, true);
    update(_count, 2);
  }

  void Led::shortBeep(uint16_t _count) {
    BK_LOG(6, F("Led.shortBeep() "), false); BK_LOG(6, led_name, false); BK_LOG(6, F(" "), false); BK_LOG(6, _count, true);
    update(_count, 6);
  }

  void Led::mediumBeep(uint16_t _count) {
    BK_LOG(6, F("Led.mediumBeep() "), false); BK_LOG(6, led_name, false); BK_LOG(6, F(" "), false); BK_LOG(6, _count, true);
    update(_count, 7);
  }

  
  
