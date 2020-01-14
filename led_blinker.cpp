  #include "led_blinker.h"

  Led::Led(int pin, const char _name[], const int _freq, const int _pwm) : 
    led_pin(pin),
    led_state(0),
    current_phase(0),
    cycle_count(0),
    num_cycles(0),
    current_ms(millis()),
    previous_ms(0),
    frequency(_freq),
    pwm(_pwm),
    intervals {}
  {
    strlcpy(led_name, _name, 3);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);
  }
  
  void Led::begin(int _num_cycles, const int _intervals[INTERVALS_LENGTH], const int _freq, const int _pwm) {
    #ifdef BK_DEBUG
      if(S.debugMode()) {
        LOG(F("Led::begin current, new:")); LOG(led_name, true);
        printIntervals(intervals);
        printIntervals(_intervals);
      }
    #endif

    // Moved to constructor.
    //	pinMode(led_pin, OUTPUT);
    //  digitalWrite(led_pin, LOW);

    // Initialize state
    led_state = LOW;
    current_phase = 0;
    cycle_count = 0;
    num_cycles = _num_cycles;
    if (_freq >=0) frequency = _freq;
    if (_pwm >=0) pwm = _pwm;

    // Copy _intervals to intervals
    //for (int n = 0; n < INTERVALS_LENGTH && _intervals[n] > 0; n ++) {
    for (int n = 0; n < INTERVALS_LENGTH; n ++) {
      intervals[n] = _intervals[n];
    }

    startPhase(0);
  }
  
  void Led::loop() {
    //BK_PRINTLN(F("*** LED LOOP BEGIN ***"));
    current_ms = millis();
    handleBlinker();
  }
  
  int Led::countIntervals(const int _intervals[INTERVALS_LENGTH]) {
    int n;
    for (n = 0; n < INTERVALS_LENGTH; n ++) {
      //BK_PRINT(F("_intervals: ")); BK_PRINT(n); BK_PRINT(F(" ")); BK_PRINTLN(_intervals[n]);
      if (_intervals[n] <= 0) {
        break;
      }
    }
    return n;
  }

  void Led::printIntervals(const int _intervals[INTERVALS_LENGTH]) {
    LOG(countIntervals(_intervals));
    LOG(",");
    for (int n = 0; n < INTERVALS_LENGTH; n ++) {
      LOG(" ");
      LOG(_intervals[n]);
    }
    LOG("", true);
  }

  // Calls begin() only if params have changed.
  // Should generally use this instad of begin().
  void Led::update(int _num_cycles, const int _intervals[INTERVALS_LENGTH], const int _freq, const int _pwm) {
    BK_PRINT(F("Led::update _intervals[0]: ")); BK_PRINTLN(_intervals[0]);
    
    #ifdef BK_DEBUG
      if(S.debugMode()) {
        LOG(F("Led::update current, new: ")); LOG(led_name, true);
        printIntervals(intervals);
        printIntervals(_intervals);
      }
    #endif

    if (
         _num_cycles == num_cycles &&
         (_freq >= 0 ? _freq == frequency : true) &&
         (_pwm >= 0 ? _pwm == pwm : true) &&
         (
            memcmp(_intervals, intervals, INTERVALS_LENGTH) == 0 ||
            (countIntervals(_intervals) == 0 && countIntervals(intervals) == 0)
         )
       )
    {
      BK_PRINTLN(F("Led::update() skipping begin()"));
    } else {
      begin(_num_cycles, _intervals, _freq, _pwm);      
    }
  }
  
  // Starts a new blinker phase, given int
  void Led::startPhase(int phz) {
    current_phase = phz;
    if (phz == 0) {
      cycle_count ++;
      
      // TODO: This should be refactored so that it reverts to
      // previous interval-set after cycle_count goes above num_cycles.
      if (num_cycles > 0 && cycle_count > num_cycles) {
        //reset();
        led_state = 0;
        return;
      }
    }

    // //don't bother changing state if interval is 0
    // Sets state according to intervals[phz].
    // Sets state to 0 if intervals[phz] == 0
    if (intervals[phz] > 0) {
      led_state = (phz + 1) % 2;
    } else {
      led_state = 0;
    }
    
    previous_ms = current_ms;
  }
    
  // Handles start-stop blinker and blinker cycling
  void Led::handleBlinker() {
    //BK_PRINTLN("Led::handleBlinker() current data:");
    //BK_PRINTLN(current_phase);
    //BK_PRINTLN(intervals[current_phase]);
    //BK_PRINTLN(led_state);
    //BK_PRINTLN(current_ms);
    //BK_PRINTLN(previous_ms);
    //BK_PRINTLN(" ");

    // If the current interval has expired
    if (current_ms - previous_ms >= (unsigned long)intervals[current_phase]) {
      // Increments the led phase, or resets it to zero,
      // then calls startPhase()
      //int ary_size = sizeof(intervals)/sizeof(*intervals);
      if (current_phase >= countIntervals(intervals) - 1) {
        startPhase(0);
      } else {
        startPhase(current_phase + 1);
      }
    }

    // For debugging the raw outpout.
    // BK_PRINT(F("LED write: ")); BK_PRINT(led_pin); BK_PRINT(F(" ")); BK_PRINTLN(led_state);
    //digitalWrite(led_pin, led_state);
    
    if (frequency == 0 && pwm == 0U) {
      BK_PRINT(F("LED write: ")); BK_PRINT(led_pin); BK_PRINT(F(", ")); BK_PRINTLN(led_state);
      digitalWrite(led_pin, led_state);
    } else if (frequency > 0) {
      BK_PRINT(F("LED tone: ")); BK_PRINT(led_pin); BK_PRINT(F(", ")); BK_PRINT(led_state); BK_PRINT(F(", ")); BK_PRINTLN(frequency);
      led_state ? tone(led_pin, frequency) : noTone(led_pin);
    } else if (pwm > 0) {
      BK_PRINT(F("LED pwm: ")); BK_PRINT(led_pin); BK_PRINT(F(", ")); BK_PRINT(led_state); BK_PRINT(F(", ")); BK_PRINTLN(pwm);
      led_state ? analogWrite(led_pin, pwm) : analogWrite(led_pin, 0U);
    }
  } // handleBlinker()

  void Led::reset() {
    led_state = 0;
    current_phase = 0;
    cycle_count = 0;
    //memset(intervals, 0, INTERVALS_LENGTH);
  }

  // Manually forces led state
  void Led::go_low() {
    led_state = LOW;
    digitalWrite(led_pin, LOW);
  }

  void Led::go_high() {
    led_state = HIGH;
    digitalWrite(led_pin, HIGH);
  }


  /*  Pattern presets  */
  
  void Led::steady() {
    BK_PRINT(F("Led::steady() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {1000};
    update(0, _intervals);
  }

  void Led::off() {
    BK_PRINT(F("Led::off() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {0};
    update(0, _intervals);
  }

  void Led::slowBlink() {
    BK_PRINT(F("Led::slowBlink() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {500,500};
    update(0, _intervals);
  }

  void Led::fastBlink() {
    BK_PRINT(F("Led::fastBlink() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {70,70};
    update(0, _intervals);
  }

  void Led::startupBlink() {
    BK_PRINT(F("Led::startupBlink() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {470,30};
    update(0, _intervals);
  }

  void Led::once() {
    BK_PRINT(F("Led::once() ")); BK_PRINTLN(led_name);
    reset();
    const int _intervals[INTERVALS_LENGTH] = {70};
    update(1, _intervals);    
  }

  void Led::fastBeep(int _count) {
    BK_PRINT(F("Led::fastBeep() ")); BK_PRINT(led_name); BK_PRINT(F(" ")); BK_PRINTLN(_count);
    const int _intervals[INTERVALS_LENGTH] = {70,70};
    update(_count, _intervals);        
  }

  void Led::slowBeep(int _count) {
    BK_PRINT(F("Led::fastBeep() ")); BK_PRINT(led_name); BK_PRINT(F(" ")); BK_PRINTLN(_count);
    const int _intervals[INTERVALS_LENGTH] = {500,500};
    update(_count, _intervals);        
  }

  
