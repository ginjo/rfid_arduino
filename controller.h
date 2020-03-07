/* 
 * This controls the master output switch and its timing.
 */

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

  #include <Arduino.h>
  #include <string.h>
  #include <SoftwareSerial.h>

  #include "reader.h"
  #include "led_blinker.h"

  // See logger.h for master debug controls.
  #ifdef CT_DEBUG
    #define CT_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define CT_LOG(...)
  #endif


  class Controller {
  public:
  
    /***  Instance Variables  ***/

    int proximity_state; //  Maybe put this in a state.h file (and class) derived from Storage.
    Reader * reader;
    //Led *blinker;
    //Led *beeper;
    int ctrl_status; // For logging ctrl state change only once per loop.

    
    /***  Constructors  ***/
    
    //Controller(Stream*, Led*, Reader*);
    //Controller(Stream*, Led*);
    //Controller(Reader*, Led*[], Led*); // This accepts a pointer to an array of Led pointers.
    //Controller(Reader*, Led[], Led*);
    Controller(Reader*);


    /***  Functions  ***/
    
    void begin();
    void loop();
    int  setProximityState(int);
    //void updateProximityState(int);
    void proximityStateController();
    void initializeOutput();
    uint32_t softCycleTotalMs();
  
  };  // Controller

#endif
  
 
