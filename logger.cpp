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

  bool can_log_to_bt() {
    return (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW);
  }

  // Handle logging to BTserial
  //  extern void LogToBT (String string, int base, bool line) {
  //    if (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW) { 
  //      if (line == true) {
  //        if (base <0) {
  //          BTserial->println((String)string);
  ////        } else {
  ////          BTserial->println("Hello", (char)HEX);
  //        }
  //      } else {
  //        if (base <0) {
  //          BTserial->println((String)string);
  //        } else {
  //          //BTserial->println((String)string, (int)base);
  //          BTserial->println(123, (int)HEX);
  //        }
  //      }
  //    }
  //  }

  //  extern void LogToBT (char * string, bool line, int base) {
  //    if (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW) { 
  //      
  //      if (base >=0) {
  //        BTserial->print(strtol(string, NULL, base));
  //      } else {
  //        BTserial->print(string);
  //      }
  //
  //      if (line == true) {
  //        BTserial->println("");
  //      }
  //        
  //    }
  //  }

  //  extern void LogToBT(const __FlashStringHelper * string, bool line) {
  //    BTserial->print(string);
  //    
  //    if (line == true) {
  //      BTserial->println("");
  //    }
  //  }

  //  extern void LogToBT(const String& string, bool line) {
  //    BTserial->print(string);
  //    
  //    if (line == true) {
  //      BTserial->println("");
  //    }
  //  }

  
