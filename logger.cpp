  #include <stdio.h>
  #include "logger.h"
  #include "settings.h"
  #include "menu.h"
  #include "serial_port.h"


  bool log_in_progress = false; // Controlls PreLog() output for multi-part log entries.


  extern uint8_t LogLevel() {
    if (S.debugMode()) {
      return (uint8_t)S.debug_level;
    } else {
      return (uint8_t)S.log_level;
    }
  }


  // Free RAM calc.  From https://forum.arduino.cc/index.php?topic=431912.0
  // TODO: Convert this to a simple function with no printing.
  extern int FreeRam (const char txt[]) {
    extern int __heap_start, *__brkval; 
    int v; 
    int rslt = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    if (HWserial->can_output()) {
      HWserial->print(F("FREE RAM (")); HWserial->print(rslt); HWserial->print(F(") ")); HWserial->println(txt);
    }
    //LOG(4, F("FREE RAM (")); LOG(4, rslt); LOG(4, F(") ")); LOG(4, txt, true);
    return rslt;
  }


  // Checks if conditions are right to log to any BT device.
  extern bool CanLogToBT() {
    //return (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW);
    return (
      digitalRead(BT_STATUS_PIN) == LOW &&
      (S.log_to_bt || digitalRead(DEBUG_PIN) == LOW || TempDebug) &&
      (Menu::RunMode == 0 || ! Menu::Current)
    );
  }


  // Converts milliseconds to readable h:m:s.ms
  // NOTE: You must free or delete the dynamic memory pointed to by the result.
  const char *Uptime() {
    uint8_t output_length = 13;
    
    unsigned long milliseconds = millis();
    unsigned long seconds  = milliseconds/1000;
    unsigned long rmillis  = milliseconds % 1000;
    unsigned long minutes  = seconds/60;
    unsigned long rseconds = seconds % 60;
    unsigned long hours    = minutes/60;
    unsigned long rminutes = minutes % 60;

    char *_out = new char[output_length];
    snprintf_P(_out, output_length, PSTR("%02li:%02li:%02li.%-3li"), hours, rminutes, rseconds, rmillis);
    return _out;
  }


  // Returns pointer to pre-log text.
  // NOTE: You must free or delete the dynamic memory pointed to by the result.
  const char *PreLog(int level) {
    uint8_t output_length = 22;
    
    char *_out = new char[output_length];

    if (log_in_progress) return _out;
    log_in_progress = true;
    
    if (Menu::RunMode == 0) {
      const char *uptime = Uptime();
      snprintf_P(_out, output_length, PSTR("%s  "), uptime);
      delete uptime;
    }

    switch (level) {
      case (3) :
        sprintf_P(_out, PSTR("%sWARN: "), _out);
        break;
      case (2) :
        sprintf_P(_out, PSTR("%sERROR: "), _out);
        break;
      case (1) :
        sprintf_P(_out, PSTR("%sFATAL: "), _out);
        break;
    }

    return _out;
  }


  // Handles final new-line at end of log entry.
  void PostLog(bool line) {
    
    if (line == true && SWserial && SWserial->can_output()) {
      SWserial->println("");
    }
  
    if (line == true && HWserial && HWserial->can_output()) {
      HWserial->println("");
    }

    if (line) log_in_progress = false;
  }

  
