#ifndef __LOGGER_H__
#define __LOGGER_H__

  #include <Arduino.h>
  #include <stdarg.h>
  #include <string.h>
  #include "global.h"

  /*
    DEBUG and XX_DEBUG controls compilation of debug code, it DOES NOT control runtime debug behavior.
    If you comment-out DEBUG, the XX_LOG lines are defined as blank,
    and any lines between '#ifdef XX_DEBUG' and '#endif' will be ommitted from compilation.

    Disabling debug code can save program memory and ram.
    
    Each class also has its own controls for managing debug code.
    Comment each class's debug global to disable debug for that class only.

    For actual logging use LOG(level, string/char, newline <true/false> optional), LOG(level, number, num-base optional, newline <true/false> optional)
    or the class-specific XX_LOG(level, string/char, newline<true/false>).

    Grounding the debug pin pushes log level to 5 (and enables log to BT serial).

    See this for log levels:
    https://stackoverflow.com/questions/2031163/when-to-use-the-different-log-levels

    Log Levels:
      1 fatal
      2 error
      3 warn
      4 info
      5 debug
      6 trace
  */
  
  #define DEBUG        // master debug control
  #define INO_DEBUG    // main ino file
  #define BK_DEBUG     // blinker
  //#define CT_DEBUG   // controller
  #define RD_DEBUG   // reader
  //#define MU_DEBUG   // serial menu
  //#define SK_DEBUG   // stack
  //#define ST_DEBUG   // settings
  //#define SO_DEBUG   // storage
  //#define TA_DEBUG   // tags

  
  #ifdef DEBUG    // Macros are usually in all capital letters.
    #define FREERAM(...)   if(S.debugMode()){FreeRam(__VA_ARGS__);}
  #else
    #define FREERAM(...)
  #endif

  extern int LogLevel();

  // Free RAM calc.  From https://forum.arduino.cc/index.php?topic=431912.0
  extern int FreeRam(const char[] = "");

  // There is probably a better place for this, but it seems to be necessary here (and in main .ino file!?!?).
  // WARN: I think this might be causing undefined behavior when used with "SoftwareSerial *BTserial;" in main .ino file.
  //       I don't think there can be two declarations for a single extern, especially if one def is not extern.
  //extern SoftwareSerial *BTserial;  // = new SoftwareSerial(BT_RX_PIN, BT_TX_PIN);

  extern void printUptime(bool = false);
  
  // Checks if conditions are right to log to BTserial.
  extern bool canLogToBT();

  //extern void LOG(int level, void *dat, const int base, bool line = false);
  //extern void LOG(int level, void *dat, bool line = false);
  

  // Handles printing to BTserial with numbers, considers integer base.
  // 
  // The templating here allows to receive any type of parameter!!!
  template<typename T>
  extern void LOG(int level, T dat, const int base, bool line = false) {
    if (level > LogLevel()) return;

    if (canLogToBT()) {
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
  
  // Handles printing to BTserial with strings and char arrays
  //.
  // The templating here allows to receive any type of parameter!!!
  template<typename T>
  extern void LOG(int level, T dat, bool line = false) {
    if (level > LogLevel()) return;
    
    if (canLogToBT()) {      
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

  
