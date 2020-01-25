  #include "logger.h"
  #include "settings.h"
  #include "menu.h"

  extern int LogLevel() {
    if (S.debugMode() && S.log_level < 5U) {
      return 5;
    } else {
      return (int)S.log_level;
    }
  }

  // Free RAM calc.  From https://forum.arduino.cc/index.php?topic=431912.0
  extern int FreeRam (const char txt[]) {
    extern int __heap_start, *__brkval; 
    int v; 
    int rslt = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    Serial.print(F("FREE RAM (")); Serial.print(rslt); Serial.print(F(") ")); Serial.println(txt);
    //LOG(4, F("FREE RAM (")); LOG(4, rslt); LOG(4, F(") ")); LOG(4, txt, true);
    return rslt;
  }

  // Checks if conditions are right to log to BTserial.
  extern bool CanLogToBT() {
    //return (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW);
    return (
      digitalRead(BT_STATUS_PIN) == LOW &&
      (S.log_to_bt || digitalRead(DEBUG_PIN) == LOW || TempDebug) &&
      (Menu::RunMode == 0 || ! Menu::Current)
    );
  }

  // Converts milliseconds to readable h,m,s.
  // TODO: Make Uptime() a global function that returns c-string.
  extern void PrintUptime(bool line) {
    unsigned long milliseconds = millis();
    unsigned long seconds = milliseconds/1000;
    unsigned long minutes = seconds/60;
    unsigned long hours   = minutes/60;
    unsigned long rseconds = seconds % 60;
    unsigned long rminutes = minutes % 60;

    char out[] = "";
    sprintf(out, "%li:%li:%li", hours, rminutes, rseconds);
    
    LOG(4, out, line);
  }
  
  
