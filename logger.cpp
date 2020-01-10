  #include "logger.h"
  #include "settings.h"
  #include "menu.h"


  // Free RAM calc.  From https://forum.arduino.cc/index.php?topic=431912.0
  extern int FreeRam (const char txt[]) {
    extern int __heap_start, *__brkval; 
    int v; 
    int rslt = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    //Serial.print(F("FREE RAM (")); Serial.print(rslt); Serial.print(F(") ")); Serial.println(txt);
    LOG(F("FREE RAM (")); LOG(rslt); LOG(F(") ")); LOG(txt, true);
    return rslt;
  }

  // Checks if conditions are right to log to BTserial.
  // The scope of this function is lexical (logger.cpp, logger.h?).
  extern bool canLogToBT() {
    //return (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW);
    return (
      digitalRead(BT_STATUS_PIN) == LOW) &&
      ((int)S.log_to_bt > 0 || S.debugMode()) &&
      (Menu::run_mode == 0 || !Menu::Current)
    ;
  }
  
