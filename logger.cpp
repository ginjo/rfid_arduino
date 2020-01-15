  #include "logger.h"
  #include "settings.h"
  #include "menu.h"


  // Free RAM calc.  From https://forum.arduino.cc/index.php?topic=431912.0
  extern int FreeRam (const char txt[]) {
    extern int __heap_start, *__brkval; 
    int v; 
    int rslt = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    Serial.print(F("FREE RAM (")); Serial.print(rslt); Serial.print(F(") ")); Serial.println(txt);
    //LOG(F("FREE RAM (")); LOG(rslt); LOG(F(") ")); LOG(txt, true);
    return rslt;
  }

  // Checks if conditions are right to log to BTserial.
  extern bool canLogToBT() {
    //return (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW);
    return (
      digitalRead(BT_STATUS_PIN) == LOW &&
      (S.log_to_bt || digitalRead(DEBUG_PIN) == LOW || TempDebug) &&
      (Menu::run_mode == 0 || ! Menu::Current)
    );
  }

  // Converts milliseconds to readable h,m,s.
  extern void printUptime(bool line) {
    unsigned long milliseconds = millis();
    unsigned long seconds = milliseconds/1000;
    unsigned long minutes = seconds/60;
    unsigned long hours   = minutes/60;
    unsigned long rseconds = seconds % 60;
    unsigned long rminutes = minutes % 60;

    char out[] = "";
    sprintf(out, "%li:%li:%li", hours, rminutes, rseconds);
    
    LOG(out, line);
  }
  
