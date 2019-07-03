#ifndef __LOGGER_H__
#define __LOGGER_H__

  #include <Arduino.h>
  //#include "settings.h"
  
  #define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
  #ifdef DEBUG    //Macros are usually in all capital letters.
    #define DPRINT(...)    if(S.enable_debug){Serial.print(__VA_ARGS__);} // BTserial.print(__VA_ARGS__);}    //DPRINT is a macro, debug print
    #define DPRINTLN(...)  if(S.enable_debug){Serial.println(__VA_ARGS__);} // BTserial.println(__VA_ARGS__);}  //DPRINTLN is a macro, debug print with new line
  #else
    #define DPRINT(...)     //now defines a blank line
    #define DPRINTLN(...)   //now defines a blank line
  #endif

#endif
