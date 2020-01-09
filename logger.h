#ifndef __LOGGER_H__
#define __LOGGER_H__

  #include <Arduino.h>
  #include <SoftwareSerial.h>
  #include <stdarg.h>

  /*
    DEBUG controls compilation of debug code, it DOES NOT control runtime debug behavior.
    If you comment-out DEBUG, the DPRINT & DPRINTLN lines are defined as blank,
    and any lines between '#ifdef DEBUG' and '#endif' will be ommitted from compilation.

    Disabling debug code can save program memory and ram.
    
    Each class also has its own controls for managing debug code.
    Comment each class's debug global to disable debug for that class only
  */
  #define DEBUG        // master debug control
  #define INO_DEBUG    // main ino file
  //#define BK_DEBUG     // blinker
  //#define RD_DEBUG   // reader
  //#define CT_DEBUG   // controller
  #define MU_DEBUG   // serial menu
  //#define SK_DEBUG   // stack
  //#define ST_DEBUG   // settings

  
  #ifdef DEBUG    // Macros are usually in all capital letters.
    #define DPRINT(...)    if(S.debugMode()){Serial.print(__VA_ARGS__);}    // BTserial.print(__VA_ARGS__);}    //DPRINT is a macro, debug print
    #define DPRINTLN(...)  if(S.debugMode()){Serial.println(__VA_ARGS__);}  // BTserial.println(__VA_ARGS__);}  //DPRINTLN is a macro, debug print with new line
    #define FREERAM(...)   if(S.debugMode()){FreeRam(__VA_ARGS__);}
  #else
    #define DPRINT(...)     //now defines a blank line
    #define DPRINTLN(...)   //now defines a blank line
    #define FREERAM(...)
  #endif

  // Free RAM calc.  From https://forum.arduino.cc/index.php?topic=431912.0
  extern int FreeRam(const char[] = "");

  // There is probably a better place for this.
  extern SoftwareSerial *BTserial;

  
  // Checks if conditions are right to log to BTserial.
  bool can_log_to_bt();

  // Handles printing to BTserial, with numbers and base.
  //
  template<typename T>
  extern void LOG(T dat, const int base, bool line = false) {
    if (can_log_to_bt()) {

      BTserial->print(dat, base);
      
      if (line == true) {
        BTserial->println("");
      }
    }

    Serial.print(dat, base);
    if (line == true) {
      Serial.println("");
    }
  }

  // Handles printing to BTserial, with strings and char arrays
  //.
  template<typename T>
  extern void LOG(T dat, bool line = false) {
    if (can_log_to_bt()) {
      BTserial->print(dat);
      if (line == true) {
        BTserial->println("");
      }
    }

    Serial.print(dat);
    if (line == true) {
      Serial.println("");
    }
  }
  
#endif

  
