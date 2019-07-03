// LED Blinker Class



#ifndef __LED_BLINKER_H__
#define __LED_BLINKER_H__

  #include <Arduino.h>
  #include <string.h>

  #include "settings.h"
  
  // Max allowed size of intervals array.
  #define INTERVALS_LENGTH 20 // for a max of 10 on/off cycles
  
  
  class Led {
  public:
    // Sets the pin number
    int led_pin;  //LED_BUILTIN;
    
    // Sets initial state of led
    int led_state;
  
    // Sets starting phase
    int current_phase;

    // How many cycles have we run thru
    int cycle_count;

    // How many cycles to run for this instance, after begin() is called
    int num_cycles;
    
    // Generally, you should use "unsigned long" for variables that hold time.
    // The value will quickly become too large for an int to store.
    unsigned long current_ms;
    unsigned long previous_ms; // will store last time LED was updateBlinkerd

    // The intervals.
    int intervals[INTERVALS_LENGTH];

    // constructor
    Led(); // dummy for initialization.
    Led(int);
    
    void begin(int, const int[]);
    void update(int, const int[]);
    void loop();
    void startPhase(int);
    void handleBlinker();
    void printIntervals(const int[]);
    int  countIntervals(const int[]);

    void Steady();
    void Off();
    void SlowBlink();
    void FastBlink();
    void StartupBlink();
  
  };

#endif
