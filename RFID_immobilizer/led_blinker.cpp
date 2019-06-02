// LED Blinker Class

#include "led_blinker.h"
  
  Led::Led(int pin) : 
    led_pin(pin)
  {
    ;
  }
  
  //void Led::begin(unsigned long inv[], int len) {
  void Led::begin(unsigned long inv[]) {
  	pinMode(led_pin, OUTPUT);
    // just while starting up arduino
    digitalWrite(led_pin, HIGH);
    
  	start_blinker = HIGH;

    for (int n = 0; n < INTERVALS_LENGTH && inv[n] > 0; n ++) {
      intervals[n] = inv[n];
      intervals_count = n+1;
      
      //  Serial.print("n: ");
      //  Serial.println(n);
      //  Serial.print("inv[n]: ");
      //  Serial.println(inv[n]);
      //  Serial.println("");
    }
  
    Serial.print(F("Led::intervals[]: "));
    Serial.print(intervals_count);
    Serial.print(",");
    for (int n = 0; n < INTERVALS_LENGTH; n ++) {
      Serial.print(" ");
      Serial.print(intervals[n]);
    }
    Serial.println("");
  }
  
  void Led::loop() {
    handleBlinker();
  }
  
  // Starts a new blinker phase, given int
  void Led::startPhase(int phz) {
    phase = phz;
    digitalWrite(led_pin, ledStateChange());
    previous_ms = millis();
  }
  
  // Toggles and returns led state
  int Led::ledStateChange() {
    if (led_state == HIGH) {
      led_state = LOW;
    } else {
      led_state = HIGH;
    }
    return led_state;
  }
  
  // Handles start-stop blinker and blinker cycling
  void Led::handleBlinker() {
    if (start_blinker == HIGH) {
      start_blinker = LOW;
      led_state = LOW;
      startPhase(0);
    }
  
    // Checks to see if it's time to toggle the led
    unsigned long current_ms = millis();
  
    //Serial.println(phase);
    //Serial.println(intervals[phase]);
    //Serial.println(led_state);
    //Serial.println(current_ms);
    //Serial.println(previous_ms);
    //Serial.println(previous_ms_saved);
    //Serial.println(" ");
  
    if (current_ms - previous_ms >= intervals[phase]) {
      // save the last time you started a new phase
      previous_ms_saved = previous_ms;
      previous_ms = current_ms;
  
      // Increments the led phase, or resets it to zero,
      // then calls startPhase()
      //int ary_size = sizeof(intervals)/sizeof(*intervals);
      if (phase >= intervals_count - 1) {
        startPhase(0);
      } else {
        startPhase(phase + 1);
      }
    }
  }
