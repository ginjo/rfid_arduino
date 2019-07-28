#ifndef __LOGGER_H__
#define __LOGGER_H__

  #include <Arduino.h>
  //#include "settings.h"

  // DEBUG controls compilation of debug code, it does not control runtime debug behavior.
  //  If you comment-out DEBUG, the DPRINT & DPRINTLN lines are defined as blank,
  //  and any lines between '#ifdef DEBUG' and '#endif' will be ommitted from compilation.
  #define DEBUG
  
  #ifdef DEBUG    //Macros are usually in all capital letters.
    #define DPRINT(...)    if(S.debugMode()){Serial.print(__VA_ARGS__);} // BTserial.print(__VA_ARGS__);}    //DPRINT is a macro, debug print
    #define DPRINTLN(...)  if(S.debugMode()){Serial.println(__VA_ARGS__);} // BTserial.println(__VA_ARGS__);}  //DPRINTLN is a macro, debug print with new line
  #else
    #define DPRINT(...)     //now defines a blank line
    #define DPRINTLN(...)   //now defines a blank line
  #endif

#endif
