// LED Blinker Class

#include <Arduino.h>
#ifndef __LED_BLINKER_H__
#define __LED_BLINKER_H__
#define INTERVALS_LENGTH 10

  class Led {
  public:
    // Sets the pin number
    int led_pin;  //LED_BUILTIN;
    
    // Sets initial state of led (phase 0)
    int led_state = LOW;
  
    // Sets starting phase
    int phase = 0;
  
    // Tells blinker to start if HIGH
    int start_blinker = LOW;
    
    // Generally, you should use "unsigned long" for variables that hold time.
    // The value will quickly become too large for an int to store.
    unsigned long previous_ms = 0; // will store last time LED was updated
    unsigned long previous_ms_saved = 0; // just for reporting
    int intervals_count;
    unsigned long intervals[INTERVALS_LENGTH];

    // constructor
    Led(int);
    
    //void begin(unsigned long[], int);
    void begin(unsigned long[]);
    void loop();
    void startPhase(int);
    int  ledStateChange();
    void handleBlinker();
  
  };

#endif
