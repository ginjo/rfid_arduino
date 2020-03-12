#ifndef __LOGGER_H__
#define __LOGGER_H__

  #include <Arduino.h>
  #include <stdarg.h>
  #include <string.h>
  #include "global.h"
  #include "serial_port.h"

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
  
  #ifdef DEBUG
    //#define BK_DEBUG   // blinker
    //#define CT_DEBUG   // controller
    //#define RD_DEBUG   // reader
    #define MU_DEBUG   // menu
    //#define SK_DEBUG   // stack
    //#define ST_DEBUG   // settings
    //#define SO_DEBUG   // storage
    //#define TA_DEBUG   // tags
  #endif


  #define PRELOG_SIZE 22
  #define UPTIME_SIZE 13


  // Forward declaration
  class Menu;

  extern bool TempDebug;

  extern uint8_t LogLevel();

  extern int FreeRam();  // (const char[] = "");

  extern bool Uptime(char*, int = UPTIME_SIZE);
  
  extern bool CanLogToBT();

  bool PreLog(int, char*, int = PRELOG_SIZE);

  void PostLog(bool);


  
  // Handles printing to SWserial with numbers, considers integer base.
  // 
  // The templating here allows to receive any type of parameter!!!
  //
  template<typename T>
  void LOG(int level, T dat, const int base, bool line = false) {
    if ((uint8_t)level > (uint8_t)LogLevel()) return;

    char prelog[PRELOG_SIZE] = {};
    PreLog(level, prelog, PRELOG_SIZE);

    for (uint8_t n = 0; n < SerialPort::Count; n++) {
      if (! SerialPort::List[n]) return;
      
      SerialPort *sp = SerialPort::List[n];
      
      if (sp && sp->can_output()) {
        sp->print(prelog);
        sp->print(dat, base);
      }
    }

    PostLog(line);
  }

  
  // Handles printing to SWserial with strings and char arrays
  //.
  // The templating here allows to receive any type of parameter!!!
  //
  template<typename T>
  void LOG(int level, T dat, bool line = false) {
    if ((uint8_t)level > (uint8_t)LogLevel()) return;

    char prelog[PRELOG_SIZE] = {};
    PreLog(level, prelog, PRELOG_SIZE);

    for (uint8_t n = 0; n < SerialPort::Count; n++) {
      if (! SerialPort::List[n]) return;
      
      SerialPort *sp = SerialPort::List[n];
      
      if (sp && sp->can_output()) {
        sp->print(prelog);
        sp->print(dat);
      }
    }

    PostLog(line);
  }
  
#endif

  
