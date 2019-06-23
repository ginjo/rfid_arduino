// LED Blinker Class

#include "led_blinker.h"
  
  Led::Led(int pin) : 
    led_pin(pin),
    led_state(0),
    current_phase(0),
    cycle_count(0),
    num_cycles(0),
    current_ms(millis()),
    previous_ms(0),
    intervals({})
  {
    ;
  }
  
  void Led::begin(int _num_cycles, const int _intervals[INTERVALS_LENGTH]) {
    if(S.enable_debug) {
      Serial.println(F("Led::begin with _intervals, intervals:"));
      printIntervals(_intervals);
      printIntervals(intervals);
    }
    
  	pinMode(led_pin, OUTPUT);

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
    current_ms = millis();
    handleBlinker();
  }

  int Led::countIntervals(const int _intervals[INTERVALS_LENGTH]) {
    int n;
    for (n = 0; n <= INTERVALS_LENGTH; n ++) {
      //DPRINT("_intervals: "); DPRINT(n); DPRINT(" "); DPRINTLN(_intervals[n]);
      if (_intervals[n] <= 0 || n == INTERVALS_LENGTH) {
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
    DPRINT(F("Led::update _intervals[0]: ")); DPRINTLN(_intervals[0]);
    if(S.enable_debug) {
      Serial.println(F("Led::update with _intervals, intervals:"));
      printIntervals(_intervals);
      printIntervals(intervals);
    }
    
    if (
        _num_cycles == num_cycles &&
        memcmp(_intervals, intervals, sizeof(_intervals)) == 0
       )
    {
      //Serial.println(F("Led::update() skipping begin()"));
    } else {
      //Serial.println(F("Led::update() calling begin()"));
      begin(_num_cycles, _intervals);      
    }
  }
  
  // Starts a new blinker phase, given int
  void Led::startPhase(int phz = 0) {
    current_phase = phz;
    if (phz == 0) {
      cycle_count ++;
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

  void Led::Steady() {
    DPRINT(F("Led::Steady(), _intervals[0]: "));
    const int _intervals[INTERVALS_LENGTH] = {1000};
    DPRINTLN(_intervals[0]);
    update(0, _intervals);
  }

  void Led::Off() {
    DPRINTLN(F("Led::Off()"));
    const int _intervals[INTERVALS_LENGTH] = {0};
    update(0, _intervals);
  }

  void Led::SlowBlink() {
    DPRINTLN(F("Led::SlowBlink()"));
    const int _intervals[INTERVALS_LENGTH] = {500,500};
    update(0, _intervals);
  }

  void Led::FastBlink() {
    DPRINTLN(F("Led::FastBlink()"));
    const int _intervals[INTERVALS_LENGTH] = {70,70};
    update(0, _intervals);
  }

  void Led::StartupBlink() {
    DPRINTLN(F("Led::StartupBlink()"));
    const int _intervals[INTERVALS_LENGTH] = {480,20};
    update(0, _intervals);
  }
    
  // Handles start-stop blinker and blinker cycling
  void Led::handleBlinker() {
  
    //Serial.println(current_phase);
    //Serial.println(intervals[current_phase]);
    //Serial.println(led_state);
    //Serial.println(current_ms);
    //Serial.println(previous_ms);
    //Serial.println(" ");

    // If the current interval has expired
    if (current_ms - previous_ms >= intervals[current_phase]) {
      // Increments the led phase, or resets it to zero,
      // then calls startPhase()
      //int ary_size = sizeof(intervals)/sizeof(*intervals);
      if (current_phase >= countIntervals(intervals) - 1) {
        startPhase(0);
      } else {
        startPhase(current_phase + 1);
      }
    }

    digitalWrite(led_pin, led_state);
  }
