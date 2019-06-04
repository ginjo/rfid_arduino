// LED Blinker Class



#ifndef __LED_BLINKER_H__
#define __LED_BLINKER_H__

  #include <Arduino.h>
  #include <string.h>

  #include "settings.h"
  
  
  // NONE of these work for defining array bounds for global constants.
  // (because they're not resolved at compile-time)
  //
  //extern const int INTERVALS_LENGTH;
  //extern const int INTERVALS_LENGTH = Settings.INTERVALS_LENGTH;
  //extern const int Settings.INTERVALS_LENGTH;
  //
  // Therefore it is necessary to use a macro, but not sure why it works.
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
    unsigned long previous_ms = 0; // will store last time LED was updated
    
    int intervals_count;
    int intervals[INTERVALS_LENGTH];
    //int intervals[10];


    // constructor
    Led(int);
    
    void begin(int, int[]);
    void update(int, int[]);
    void loop();
    void startPhase(int);
    void handleBlinker();
    void on();
    void off();
  
  };

#endif
