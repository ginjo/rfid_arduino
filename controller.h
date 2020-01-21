/* 
 * This controls the master output switch and its timing.
 */

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

  #include <Arduino.h>
  //#include <Stream.h>
  #include <string.h>
  #include <SoftwareSerial.h>

  #include "settings.h"
  #include "storage.h"
  #include "reader.h"
  #include "led_blinker.h"
  #include "menu.h"
  #include "tags.h"

  // See logger.h for master debug controls.
  #ifdef CT_DEBUG
    #define CT_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define CT_LOG(...)
  #endif


  class Controller {
  public:
  
    /***  Variables  ***/

    int proximity_state; //  Maybe put this in a state.h file (and class) derived from Storage.
    Reader * reader;
    Led **blinker;
    Led *beeper;

    
    /***  Constructors  ***/
    
    //Controller(Stream*, Led*, Reader*);
    //Controller(Stream*, Led*);
    Controller(Reader*, Led*[], Led*);


    /***  Functions  ***/
    
    void begin();
    void loop();
    int  setProximityState(int);
    //void updateProximityState(int);
    void proximityStateController();
    void initializeOutput();


    /***  Static Vars & Functions  ***/

    // This is experimental for the new Tags class
    // See tags.h for typedef TagArray.
    //typedef uint32_t TagArray[TAG_LIST_SIZE];
    // I don't think it's being used. Let's try disabling it.
    //static TagArray& Tags;
  
  };  // Controller

#endif

 
