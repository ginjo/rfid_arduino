// LED Blinker Class

#include <Arduino.h>
#ifndef __LED_BLINKER_H__
#define __LED_BLINKER_H__


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
    int intervals_length;
    unsigned long intervals[];

    // constructor
    Led(int);
    
    void begin(unsigned long[], int);
    void loop();
    void startPhase(int);
    int  ledStateChange();
    void handleBlinker();
  
  };

#endif
