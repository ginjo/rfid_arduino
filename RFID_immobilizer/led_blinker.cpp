// LED Blinker Class

#include "led_blinker.h"
  
  Led::Led(int byt) : 
    led_pin(byt)  //,
  	//intervals_length(len)
  	//intervals(inv) // this doesn't seem to work here,
  	// so use the itterative pattern below:
  {
  	//Serial.println("Beginning LED constructor\r\n");
    //for (int n = 0; n < len; n ++) {
    //  intervals[n] = inv[n];
    //	// Serial.print("n: ");
    //	// Serial.println(n);
    //	// Serial.print("inv[n]: ");
    //	// Serial.println(inv[n]);
    //	// Serial.println("\r\n");
    //}
  
  	// Serial.println("Finished LED contructor");
   ;
  }
  
  
  void Led::setup(int inv[], int len) {
  	pinMode(led_pin, OUTPUT);
  	start_blinker = HIGH;
    // I don't think you can do sizeof on a passed-in array.
    //intervals_length = sizeof(inv)/sizeof(inv[0]);
    intervals_length = len;
    Serial.print("Led::setup intervals_length: ");
    Serial.println(intervals_length);
    
    for (int n = 0; n < intervals_length; n ++) {
      intervals[n] = inv[n];
      Serial.print("n: ");
      Serial.println(n);
      Serial.print("inv[n]: ");
      Serial.println(inv[n]);
      Serial.println("\r\n");
    }
  
    Serial.print("intervals: ");
    Serial.print(intervals_length);
    Serial.print(",");
    for (int n = 0; n < intervals_length; n ++) {
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
      
      Serial.println(phase);
      Serial.println(intervals[phase]);
      Serial.println(current_ms);
      Serial.println(previous_ms_saved);
      Serial.println(led_state);
      Serial.println(" ");
  
      // Increments the led phase, or resets it to zero,
      // then calls startPhase()
      //int ary_size = sizeof(intervals)/sizeof(*intervals);
      if (phase >= intervals_length - 1) {
        startPhase(0);
      } else {
        startPhase(phase + 1);
      }
    }
  }
