// LED Blinker Class

#include <Arduino.h>
#ifndef __LED_BLINKER_H__
#define __LED_BLINKER_H__
#define INTERVALS_LENGTH 10


  // This should really go in a Utility class.
  // It is only here as a quick fix.    
  // Free memory from https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
  #ifdef __arm__
  // should use uinstd.h to define sbrk but Due causes a conflict
  extern "C" char* sbrk(int incr);
  #else  // __ARM__
  extern char *__brkval;
  #endif  // __arm__
  
  static int freeMemory() {
    char top;
  #ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
  #else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
  #endif  // __arm__
  }


  class Led {
  public:
    // Sets the pin number
    int led_pin; // = 9; //LED_BUILTIN;// the number of the LED pin
    
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

    // Sets duration of led phases (milliseconds).
    // Should always be even num of elements.
    // It LOOKS like the variable-length array needs to be declared
    // at the end of these declarations, otherwise other vars will
    // clobber its memory locations.
    // TODO: Set a defined number of elements possible in the intervals[] array,
    // like maybe 16 or 32. How much ram will this take?
    //const int intervals_length = 8;
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
