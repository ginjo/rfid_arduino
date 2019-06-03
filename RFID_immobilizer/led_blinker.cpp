// LED Blinker Class

#include "led_blinker.h"
  
  Led::Led(int pin) : 
    led_pin(pin)
  {
    ;
  }
  
  void Led::begin(int _num_cycles, int _intervals[]) {
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
      if (intervals[n] > 0) {
        intervals_count = n+1;
      }
    }

      Serial.print(F("Led::intervals[]: "));
      Serial.print(intervals_count);
      Serial.print(",");
      for (int n = 0; n < INTERVALS_LENGTH; n ++) {
        Serial.print(" ");
        Serial.print(intervals[n]);
      }
      Serial.println("");

    startPhase(0);
  }
  
  void Led::loop() {
    handleBlinker();
  }

  // Calls begin() only if params have changed.
  // Should generally use this instad of begin().
  void Led::update(int _num_cycles, int _intervals[]) {
    if (
        _num_cycles == num_cycles &&
        memcmp(_intervals, intervals, sizeof(_intervals)) == 0
       )
    {
      //Serial.println("Led::update() skipping begin()");
    } else {
      //Serial.println("Led::update() calling begin()");
      begin(_num_cycles, _intervals);      
    }
  }
  
  // Starts a new blinker phase, given int
  void Led::startPhase(int phz = 0) {
    current_phase = phz;
    if (phz == 0) {
      cycle_count ++;
    }

    // don't bother changing state if interval is 0
    if (intervals[phz] > 0) {
      led_state = (phz + 1) % 2;
    }
    
    previous_ms = millis();
  }

  void Led::on() {
    int intervals[INTERVALS_LENGTH] = {1000};
    update(0, intervals);
  }

  void Led::off() {
    int intervals[INTERVALS_LENGTH] = {0,1000};
    update(0, intervals);
  }
  
  // Handles start-stop blinker and blinker cycling
  void Led::handleBlinker() {
    unsigned long current_ms = millis();
  
    //Serial.println(current_phase);
    //Serial.println(intervals[current_phase]);
    //Serial.println(led_state);
    //Serial.println(current_ms);
    //Serial.println(previous_ms);
    //Serial.println(" ");

    // If the current interval has expired
    if (current_ms - previous_ms >= intervals[current_phase]) {
      // save the last time you started a new phase
      previous_ms = current_ms;
  
      // Increments the led phase, or resets it to zero,
      // then calls startPhase()
      //int ary_size = sizeof(intervals)/sizeof(*intervals);
      if (current_phase >= intervals_count - 1) {
        startPhase(0);
      } else {
        startPhase(current_phase + 1);
      }
    }

    digitalWrite(led_pin, led_state);
  }
