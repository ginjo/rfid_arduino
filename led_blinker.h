// LED Blinker Class



#ifndef __LED_BLINKER_H__
#define __LED_BLINKER_H__

  #include <Arduino.h>
  #include <string.h>

  #include "settings.h"

  // See logger.h for master debug controls.
  #ifdef BK_DEBUG
    #define BK_PRINT(...) DPRINT(__VA_ARGS__)
    #define BK_PRINTLN(...) DPRINTLN(__VA_ARGS__)
  #else
    #define BK_PRINT(...)
    #define BK_PRINTLN(...)
  #endif

  #define LED_RED_PIN 8
  #define LED_GREEN_PIN 7
  #define LED_BLUE_PIN 6
  
  // Max allowed size of intervals array.
  #define INTERVALS_LENGTH 10 // for a max of 5 on/off cycles
  
  
  class Led {
  public:
    // Sets the pin number
    int led_pin;  //LED_BUILTIN;

    // Sets led name
    char led_name[3];
    
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
    Led(int, const char[]);
    
    void begin(int, const int[]);
    void update(int, const int[]);
    void loop();
    void startPhase(int = 0);
    void handleBlinker();
    void printIntervals(const int[]);
    int  countIntervals(const int[]);
    void reset();

    void steady();
    void off();
    void slowBlink();
    void fastBlink();
    void startupBlink();
    void once();
    void go_low();
    void go_high();
  
  };

#endif
