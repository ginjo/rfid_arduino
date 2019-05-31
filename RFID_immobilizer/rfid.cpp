#include "rfid.h"

  // constructor
  RFID::RFID(Stream *_serial_port) :
    buff({}),
    buff_index(0),
    previous_ms(0),
    serial_port(_serial_port)
  {
    ;
  }

  void RFID::begin() {
    ;
  }

  void RFID::loop() {
    if (serial_port->available() && millis() - previous_ms > 1000) {
      buff[buff_index] = serial_port->read();

      Serial.print("(");
      Serial.print(buff_index);
      Serial.print(")");
      Serial.print(buff[buff_index]);

      if (buff_index == 0 && buff[0] != 2) { // reset bogus read
        resetBuffer();
      } else if (buff_index == 13 && buff[13] != 3) { // reset bogus read
        resetBuffer();
        Serial.println("");
      } else if (buff_index < 13) { // good read, add comma to log and keep reading
        buff_index++;
        Serial.print(",");
      } else { // tag complete, now process it
        Serial.println("");
        Serial.println((char *)buff);
        previous_ms = millis();
        resetBuffer();
      }
      
    } else {
      ;
    }
  }

  void RFID::resetBuffer() {
    buff_index = 0;
    strncpy(buff, NULL, 14);
  }

  
