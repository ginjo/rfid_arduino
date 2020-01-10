#ifndef __LOGGER_H__
#define __LOGGER_H__

  #include <Arduino.h>
  //#include <SoftwareSerial.h>
  #include <stdarg.h>
  #include "global.h"

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

  // There is probably a better place for this, but it seems to be necessary here (and in main .ino file!?!?).
  // WARN: I think this might be causing undefined behavior when used with "SoftwareSerial *BTserial;" in main .ino file.
  //       I don't think there can be two declarations for a single extern, especially if one def is not extern.
  //extern SoftwareSerial *BTserial;  // = new SoftwareSerial(BT_RX_PIN, BT_TX_PIN);

  
  // Checks if conditions are right to log to BTserial.
  extern bool canLogToBT();

  // Handles printing to BTserial, with numbers and base.
  //
  template<typename T>
  extern void LOG(T dat, const int base, bool line = false) {
    if (canLogToBT()) {

      BTserial->print(dat, base);
      
      if (line == true) {
        BTserial->println("");
      }
      // Experimental delay trying to fix startup weirdness on BTserial,
      // but I don't think this is the issue.
      //delay(2);
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
    if (canLogToBT()) {
      BTserial->print(dat);
      if (line == true) {
        BTserial->println("");
      }
      // Experimental delay trying to fix startup weirdness on BTserial,
      // but I don't think this is the issue.      
      delay(2);
    }

    Serial.print(dat);
    if (line == true) {
      Serial.println("");
    }
  }
  
#endif

  
