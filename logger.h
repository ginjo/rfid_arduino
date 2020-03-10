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
  
  #ifdef DEBUG
    #define INO_DEBUG    // main ino file
    //#define BK_DEBUG   // blinker
    //#define CT_DEBUG   // controller
    //#define RD_DEBUG   // reader
    #define MU_DEBUG   // menu
    //#define SK_DEBUG   // stack
    //#define ST_DEBUG   // settings
    //#define SO_DEBUG   // storage
    //#define TA_DEBUG   // tags
  #endif
  
  #ifdef DEBUG    // Macros are usually in all capital letters.
    #define FREERAM(...)   if(S.debugMode()){FreeRam(__VA_ARGS__);}
  #else
    #define FREERAM(...)
  #endif

  // Forward declaration
  class Menu;

  extern uint8_t LogLevel();

  extern int FreeRam(const char[] = "");

  // NOTE: You must free or delete the dynamic memory pointed to by the result.
  extern const char *Uptime();
  
  extern bool CanLogToBT();

  // NOTE: You must free or delete the dynamic memory pointed to by the result.
  char const *PreLog(int);

  // TODO: Should this return like PreLog() ?
  void PostLog(bool);

  
  // Handles printing to SWserial with numbers, considers integer base.
  // 
  // The templating here allows to receive any type of parameter!!!
  //
  template<typename T>
  extern void LOG(int level, T dat, const int base, bool line = false) {
    if ((uint8_t)level > (uint8_t)LogLevel()) return;

    const char *prelog = PreLog(level);
    
    //if (CanLogToBT() || ! SWserial->is_bt) {
    if (SWserial && SWserial->can_output()) {
      SWserial->print(prelog);
      SWserial->print(dat, base);
    }

    //if (CanLogToBT() || ! HWserial->is_bt) {
    if (HWserial && HWserial->can_output()) {
      HWserial->print(prelog);
      HWserial->print(dat, base);
    }

    PostLog(line);

    delete prelog;
  }

  
  // Handles printing to SWserial with strings and char arrays
  //.
  // The templating here allows to receive any type of parameter!!!
  //
  template<typename T>
  extern void LOG(int level, T dat, bool line = false) {
    if ((uint8_t)level > (uint8_t)LogLevel()) return;

    const char *prelog = PreLog(level);
    
    //if (CanLogToBT() || ! SWserial->is_bt) {
    if (SWserial && SWserial->can_output()) {
      SWserial->print(prelog);
      SWserial->print(dat);
    }

    //if (CanLogToBT() || ! HWserial->is_bt) {
    if (HWserial && HWserial->can_output()) {
      HWserial->print(prelog);
      HWserial->print(dat);
    }

    PostLog(line);

    delete prelog;
  }
  
#endif

  
