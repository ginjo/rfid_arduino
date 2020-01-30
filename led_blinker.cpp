  #include "led_blinker.h"
  #include "logger.h"


  // Static pre-defined 2D interval array.
  const int Led::StaticIntervals[][INTERVALS_LENGTH] = {
    {1000},
    {0},
    {500,500},
    {80,80},
    {470,30},
    {70},
    {80,80},
    {500,500}
  };

  void Led::PrintStaticIntervals() {
    Serial.println(F("PrintStaticIntervals()"));
    for (int n = 0; n < 8; n++) {
      Serial.print(n); Serial.print(":");
      for (int m = 0; m < INTERVALS_LENGTH; m++) {
        Serial.print(StaticIntervals[n][m]);
        Serial.print(",");
      }
      Serial.println("");
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
  
  //void Led::begin(const int _num_cycles, const int _intervals[], const int _freq, const int _pwm) {
  void Led::begin(const uint16_t _num_cycles, const int intervals_index, const int _freq, const int _pwm) {
    
    BK_LOG(5, F("Led::begin old, new: "), false); BK_LOG(5, led_name, true);
    if (LogLevel() >= 5) {
      printIntervals(intervals);
      printIntervals(StaticIntervals[intervals_index]);
    }

    // Initialize state
    led_state = LOW;
    current_phase = 0;
    cycle_count = 0U;
    num_cycles = _num_cycles;
    intervals = StaticIntervals[intervals_index];
    if (_freq >=0) frequency = _freq;
    if (_pwm >=0) pwm = _pwm;

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
    
    BK_LOG(6, F("Led::update old, new: "), false); BK_LOG(6, led_name, true);
    #ifdef BK_DEBUG
      if (LogLevel() >= 6) {
        printIntervals(intervals);
        printIntervals(StaticIntervals[intervals_index]);
      }
    #endif

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
      BK_LOG(6, F("Led::update() skipping begin()"), true);
    } else {
      begin(_num_cycles, intervals_index, _freq, _pwm);      
    }
  }
  
  // Starts a new blinker phase, given int
  void Led::startPhase(int phz) {
    current_phase = phz;
    if (phz == 0) {
      //cycle_count ++;
      
      // TODO: This should be refactored so that it reverts to
      // previous interval-set after cycle_count goes above num_cycles.
      if (num_cycles > 0U && cycle_count > num_cycles) {
        //reset();
        led_state = 0;
        return;
      } else if (num_cycles > 0U) {
        cycle_count ++;
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

    // Sets local debug level for the next chunk of code.
    int lv = 5;

    // Only writes to pins if data has changed, not every loop.
    if (signature[0] != led_pin || signature[1] != led_state || signature[2] != frequency || signature[3] != pwm) {
      if (frequency == 0 && pwm == 0U) {
        BK_LOG(lv, F("LED write: "), false); BK_LOG(lv, led_pin, false); BK_LOG(lv, F(", "), false); BK_LOG(lv, led_state, true);
        digitalWrite(led_pin, led_state);
      } else if (frequency > 0) {
        BK_LOG(lv, F("LED tone: "), false); BK_LOG(lv, led_pin, false); BK_LOG(lv, F(", "), false); BK_LOG(lv, led_state, false); BK_LOG(lv, F(", "), false); BK_LOG(lv, frequency, true);
        led_state ? tone(led_pin, frequency) : noTone(led_pin);
      } else if (pwm > 0) {
        BK_LOG(lv, F("LED pwm: "), false); BK_LOG(lv, led_pin, false); BK_LOG(lv, F(", "), false); BK_LOG(lv, led_state, false); BK_LOG(lv, F(", "), false); BK_LOG(lv, pwm, true);
        led_state ? analogWrite(led_pin, pwm) : analogWrite(led_pin, 0U);
      }
    }

    printData();

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
    if (LogLevel() < 5) return;
    BK_LOG(5, num_cycles, false);
    BK_LOG(5, ",", false);
    for (int n = 0; n < INTERVALS_LENGTH; n ++) {
      BK_LOG(5, " ", false);
      BK_LOG(5, _intervals[n], false);
    }
    BK_LOG(5, "", true);
  }

  void Led::printData() {
    BK_LOG(6, F("Led Current Data "), false); BK_LOG(6, led_name, true);
    BK_LOG(6, F("current_phase "), false); BK_LOG(6, current_phase, true);
    BK_LOG(6, F("cycle_count "), false); BK_LOG(6, cycle_count, true);
    BK_LOG(6, F("interval "), false); BK_LOG(6, intervals[current_phase], true);
    BK_LOG(6, F("led_state "), false); BK_LOG(6, led_state, true);
    BK_LOG(6, F("current_ms "), false); BK_LOG(6, current_ms, true);
    BK_LOG(6, F("previous_ms "), false); BK_LOG(6, previous_ms, true);
    BK_LOG(6, "", true);
  }
  

  // TODO: How deeply does this reset the Led? Should it reset intervals too?
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
    BK_LOG(6, F("Led::steady() "), false); BK_LOG(6, led_name, true);
    update(0, 0);
  }

  void Led::off() {
    BK_LOG(6, F("Led::off() "), false); BK_LOG(6, led_name, true);
    update(0, 1);
  }

  void Led::slowBlink() {
    BK_LOG(6, F("Led::slowBlink() "), false); BK_LOG(6, led_name, true);
    update(0, 2);
  }

  void Led::fastBlink() {
    BK_LOG(6, F("Led::fastBlink() "), false); BK_LOG(6, led_name, true);
    update(0, 3);
  }

  void Led::startupBlink() {
    BK_LOG(6, F("Led::startupBlink() "), false); BK_LOG(6, led_name, true);
    update(0, 4);
  }

  void Led::once() {
    BK_LOG(6, F("Led::once() "), false); BK_LOG(6, led_name, true);
    reset();
    update(1, 5);
  }

  void Led::fastBeep(uint16_t _count) {
    BK_LOG(6, F("Led::fastBeep() "), false); BK_LOG(6, led_name, false); BK_LOG(6, F(" "), false); BK_LOG(6, _count, true);
    //const int _intervals[INTERVALS_LENGTH] = {80,80};
    //update(_count, _intervals);
    update(_count, 6);
  }

  void Led::slowBeep(uint16_t _count) {
    BK_LOG(6, F("Led::slowBeep() "), false); BK_LOG(6, led_name, false); BK_LOG(6, F(" "), false); BK_LOG(6, _count, true);
    //const int _intervals[INTERVALS_LENGTH] = {500,500};
    //update(_count, _intervals);
    update(_count, 7);
  }

  
