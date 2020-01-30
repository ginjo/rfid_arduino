  #include "logger.h"
  #include "settings.h"
  #include "menu.h"
  #include <stdio.h>

  extern int LogLevel() {
    //if (S.debugMode() && S.log_level < 5U) {
    if (S.debugMode() && S.enable_debug > S.log_level) {
      return S.enable_debug;
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

  //  // Converts milliseconds to readable h,m,s.
  //  // TODO: Make Uptime() a global function that returns c-string.
  //  extern void PrintUptime(bool line) {
  //    unsigned long milliseconds = millis();
  //    unsigned long seconds = milliseconds/1000;
  //    unsigned long minutes = seconds/60;
  //    unsigned long hours   = minutes/60;
  //    unsigned long rseconds = seconds % 60;
  //    unsigned long rminutes = minutes % 60;
  //
  //    char out[8] = {};
  //    sprintf(out, "%li:%li:%li", hours, rminutes, rseconds);
  //    
  //    LOG(4, out, line);
  //  }


  // Converts milliseconds to readable h:m:s.
  // NOTE: You must free or delete the dynamic memory pointed to by the result.
  char *Uptime() {
    unsigned long milliseconds = millis();
    unsigned long seconds = milliseconds/1000;
    unsigned long minutes = seconds/60;
    unsigned long hours   = minutes/60;
    unsigned long rseconds = seconds % 60;
    unsigned long rminutes = minutes % 60;

    char *_out = new char[9];
    snprintf(_out, 9, "%li:%li:%li", hours, rminutes, rseconds);
    return _out;
  }


  // Returns pointer to pre-log text.
  // NOTE: You must free or delete the dynamic memory pointed to by the result.
  char *PreLog(int level) {
    char *_out = new char[16];
    
    if (Menu::RunMode == 0) {
      char *uptime = Uptime();
      sprintf(_out, "%s ", uptime);
      delete uptime;
    }

    switch (level) {
      case (3) :
        sprintf(_out, "%sWARN ", _out);
        break;
      case (2) :
        sprintf(_out, "%sERROR ", _out);
        break;
      case (1) :
        sprintf(_out, "%sFATAL ", _out);
        break;
    }

    return _out;
  }


  // Handles final new-line at end of log entry.
  void PostLog(bool line) {
    
    if (CanLogToBT()) {      
      if (line == true) {
        BTserial->println("");
      }
    }
  
    if (line == true) {
      Serial.println("");
    }
  }

  
