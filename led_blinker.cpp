  #include "led_blinker.h"
  
  Led::Led(int pin, const char _name[]) : 
    led_pin(pin),
    led_state(0),
    current_phase(0),
    cycle_count(0),
    num_cycles(0),
    current_ms(millis()),
    previous_ms(0),
    intervals {}
  {
    strlcpy(led_name, _name, 3);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);
  }
  
  void Led::begin(int _num_cycles, const int _intervals[INTERVALS_LENGTH]) {
    if(S.debugMode()) {
      Serial.print(F("Led::begin current, new:")); Serial.println(led_name);
      printIntervals(intervals);
      printIntervals(_intervals);
    }
    
    //	pinMode(led_pin, OUTPUT);
    //  digitalWrite(led_pin, LOW);

    // Initialize state
    led_state = LOW;
    current_phase = 0;
    cycle_count = 0;
    num_cycles = _num_cycles;

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
    Serial.print(countIntervals(_intervals));
    Serial.print(",");
    for (int n = 0; n < INTERVALS_LENGTH; n ++) {
      Serial.print(" ");
      Serial.print(_intervals[n]);
    }
    Serial.println("");
  }

  // Calls begin() only if params have changed.
  // Should generally use this instad of begin().
  void Led::update(int _num_cycles, const int _intervals[INTERVALS_LENGTH]) {
    BK_PRINT(F("Led::update _intervals[0]: ")); BK_PRINTLN(_intervals[0]);
    if(S.debugMode()) {
      Serial.print(F("Led::update current, new: ")); Serial.println(led_name);
      printIntervals(intervals);
      printIntervals(_intervals);
    }

    // TODO: FIX: This doesn't work when intervals are all 0, as they are for Off().
    // Update: The added logic of comparing countIntervals == 0 for current & new
    //         should solve the problem now.
    if (
         _num_cycles == num_cycles &&
         (
            memcmp(_intervals, intervals, INTERVALS_LENGTH) == 0 ||
            (countIntervals(_intervals) == 0 && countIntervals(intervals) == 0)
         )
       )
    {
      BK_PRINTLN(F("Led::update() skipping begin()"));
    } else {
      //Serial.println(F("Led::update() calling begin()"));
      begin(_num_cycles, _intervals);      
    }
  }
  
  // Starts a new blinker phase, given int
  void Led::startPhase(int phz) {
    current_phase = phz;
    if (phz == 0) {
      cycle_count ++;
      
      // TODO: This should be refactored so that it reverts to
      // previous interval set after cycle_count goes above num_cycles.
      if (num_cycles > 0 && cycle_count > num_cycles) {
        reset();
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
    //BK_PRINT(F("LED write: ")); BK_PRINT(led_pin); BK_PRINT(F(" ")); BK_PRINTLN(led_state);
    digitalWrite(led_pin, led_state);
  }

  void Led::reset() {
    led_state = 0;
    current_phase = 0;
    cycle_count = 0;
    memset(intervals, 0, INTERVALS_LENGTH);
  }


  /*  Preset patterns  */
  
  void Led::Steady() {
    BK_PRINT(F("Led::Steady() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {1000};
    update(0, _intervals);
  }

  void Led::Off() {
    BK_PRINT(F("Led::Off() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {0};
    update(0, _intervals);
  }

  void Led::SlowBlink() {
    BK_PRINT(F("Led::SlowBlink() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {500,500};
    update(0, _intervals);
  }

  void Led::FastBlink() {
    BK_PRINT(F("Led::FastBlink() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {70,70};
    update(0, _intervals);
  }

  void Led::StartupBlink() {
    BK_PRINT(F("Led::StartupBlink() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {470,30};
    update(0, _intervals);
  }

  void Led::Once() {
    BK_PRINT(F("Led::Once() ")); BK_PRINTLN(led_name);
    const int _intervals[INTERVALS_LENGTH] = {120};
    update(1, _intervals);    
  }

  
