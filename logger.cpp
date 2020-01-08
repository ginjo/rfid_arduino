  #include "logger.h"
  #include "settings.h"
  #include "menu.h"

  // Free RAM calc.  From https://forum.arduino.cc/index.php?topic=431912.0
  extern int FreeRam (const char txt[]) {
    extern int __heap_start, *__brkval; 
    int v; 
    int rslt = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    Serial.print(F("FREE RAM (")); Serial.print(rslt); Serial.print(F(") ")); Serial.println(txt);
    return rslt;
  }

  // Handle logging to BTserial
  //extern void LogToBT (bool line, ...) {
  extern void LogToBT (bool line, String args) {
    if (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW) { 

      //Serial.println("LogToBT sending output to BTserial");
      //BTserial->println("LogToBT sending output to BTserial");
      
      //va_list args;
      //va_start(args, line);
      //char * arg = va_arg(args, char*);
      
      if (line == true) {
        //BTserial->println(arg);
        BTserial->println((String)args);
      } else {
        //BTserial->print(arg);
        BTserial->print((String)args);
      }
      
      //va_end(args);
    }
    
  }

  
  
