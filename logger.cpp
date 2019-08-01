#include "logger.h"

  // Free RAM calc.  From https://forum.arduino.cc/index.php?topic=431912.0
  extern int FreeRam (const char txt[]) {
    extern int __heap_start, *__brkval; 
    int v; 
    int rslt = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    Serial.print(F("FREE RAM (")); Serial.print(rslt); Serial.print(F(") ")); Serial.println(txt);
    return rslt;
  }