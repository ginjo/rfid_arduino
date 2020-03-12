  #include <stdio.h>
  #include "logger.h"
  #include "settings.h"
  #include "menu.h"
  #include "serial_port.h"


  bool TempDebug = false;
  
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
  int FreeRam() {   // (const char txt[]) {
    extern int __heap_start, *__brkval; 
    int v; 
    int rslt = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    //  if (HWserial->can_output()) {
    //    HWserial->print(F("FREE RAM (")); HWserial->print(rslt); HWserial->print(F(") ")); HWserial->println(txt);
    //  }
    return rslt;
  }


  // Checks if conditions are right to log to any BT device.
  bool CanLogToBT() {
    //return (S.log_to_bt > 0 && Menu::run_mode == 0 && digitalRead(BT_STATUS_PIN) == LOW);
    return (
      digitalRead(BT_STATUS_PIN) == LOW &&
      (S.log_to_bt || digitalRead(DEBUG_PIN) == LOW || TempDebug) &&
      (Menu::RunMode == 0 || ! Menu::Current)
    );
  }


  // Converts milliseconds to readable h:m:s.ms
  // NOTE: You must free or delete the dynamic memory pointed to by the result.
  bool Uptime(char *_out, int _size) {
        
    unsigned long milliseconds = millis();
    unsigned long seconds  = milliseconds/1000;
    unsigned long rmillis  = milliseconds % 1000;
    unsigned long minutes  = seconds/60;
    unsigned long rseconds = seconds % 60;
    unsigned long hours    = minutes/60;
    unsigned long rminutes = minutes % 60;

    snprintf_P(_out, _size, PSTR("%02li:%02li:%02li.%-3li"), hours, rminutes, rseconds, rmillis);
    return _out ? true : false;
  }


  bool PreLog(int _level, char *_out, int _size) {
    
    //  if (_out[0]) {
    //    SerialPort::List[0]->print(F("prelog '"));
    //    SerialPort::List[0]->print(_out);
    //    SerialPort::List[0]->println(F("'"));
    //  }

    if (log_in_progress) return false;
    
    log_in_progress = true;
    
    if (Menu::RunMode == 0) {
      char uptime[UPTIME_SIZE] = {};
      Uptime(uptime, UPTIME_SIZE);
      
      snprintf_P(_out, _size, PSTR("%s  "), uptime);
    }

    switch (_level) {
      case (3) :
        snprintf_P(_out, _size, PSTR("%sWARN: "), _out);
        break;
      case (2) :
        snprintf_P(_out, _size, PSTR("%sERROR: "), _out);
        break;
      case (1) :
        snprintf_P(_out, _size, PSTR("%sFATAL: "), _out);
        break;
    }
    
    //  if (_out[0]) {
    //    SerialPort::List[0]->print(F("prelog '"));
    //    SerialPort::List[0]->print(_out);
    //    SerialPort::List[0]->println(F("'"));
    //  }
    return true;
  }



  // Handles final new-line at end of log entry.
  void PostLog(bool line) {

    for (uint8_t n = 0; n < SerialPort::Count; n++) {
      if (! SerialPort::List[n]) return;
      
      SerialPort *sp = SerialPort::List[n];
      
      if (line == true && sp && sp->can_output()) {
        sp->println("");
      }      
    }

    if (line) log_in_progress = false;
  }

  
