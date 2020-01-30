// LED Blinker Class

#ifndef __LED_BLINKER_H__
#define __LED_BLINKER_H__

  #include <Arduino.h>
  #include <string.h>

  #include "global.h"
  #include "settings.h"

  // See logger.h for master debug controls.
  #ifdef BK_DEBUG
    #define BK_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define BK_LOG(...)
  #endif
  
  // Max allowed size of intervals array.
  #define INTERVALS_LENGTH 2 // for a max of 1 on/off cycles.
  
  
  class Led {
  public:

    /*  Static  */
    
    // Static pre-defined 2D interval array.
    static const int StaticIntervals[][INTERVALS_LENGTH];

    // Prints all StaticIntervals.
    static void PrintStaticIntervals();
    

    /*  Instance  */
  
    // Sets the pin number
    int led_pin;  //LED_BUILTIN;

    // Sets led name
    char led_name[3];
    
    // Sets initial state of led
    int led_state;
  
    // Sets starting phase
    int current_phase;

    // How many cycles have we run thru
    uint16_t cycle_count;

    // How many cycles to run for this instance, after begin() is called
    uint16_t num_cycles;
    
    // Generally, you should use "unsigned long" for variables that hold time.
    // The value will quickly become too large for an int to store.
    unsigned long current_ms;
    unsigned long previous_ms; // will store last time LED was updateBlinkerd

    // Beeper output frequency (Hz)
    // See https://ux.stackexchange.com/questions/14170/is-there-a-standard-for-the-frequencies-and-or-duration-used-for-beeps
    int frequency;

    // PWM output value (0-255 on pins 3, 5, 6, 9, 10, 11)
    int pwm;

    // The intervals.
    //int intervals[INTERVALS_LENGTH]; // this is not a pointer, it's a value.
    const int *intervals; // pointer to const array of ints.

    // A status signature, to know when state has changed
    int signature[4]; // led_pin, led_state, frequency, pin

    // constructor
    Led(int, const char[], const int=0, const int=0);
    
    //void begin(int, const int[], const int=-1, const int=-1);
    void begin(uint16_t, const int, const int=-1, const int=-1);
    //void update(int, const int[], const int=-1, const int=-1);
    void update(uint16_t, const int, const int=-1, const int=-1);
    void loop();
    void startPhase(int = 0);
    void handleBlinker();
    void printIntervals(const int[]);
    int  countIntervals(const int[]);
    void printData();
    void reset();

    void steady();
    void off();
    void slowBlink();
    void fastBlink();
    void startupBlink();
    void once();
    void fastBeep(uint16_t=0U);
    void slowBeep(uint16_t=0U);
    void go_low();
    void go_high();
  
  };

#endif

  
