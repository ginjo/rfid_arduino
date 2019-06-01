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
    if (serial_port->available() && millis() - previous_ms > TAG_READ_INTERVAL) {
      buff[buff_index] = serial_port->read();

      Serial.print("(");
      Serial.print(buff_index);
      Serial.print(")");
      Serial.print(buff[buff_index], HEX);
      Serial.print(":");
      Serial.print(buff[buff_index], DEC);

      int final_index = RAW_TAG_LENGTH - 1;

      if (buff_index == 0 && buff[0] != 2) { // reset bogus read
        resetBuffer();
      } else if (buff_index == final_index && buff[final_index] != 3) { // reset bogus read
        resetBuffer();
        Serial.println("");
      } else if (buff_index < final_index) { // good read, add comma to log and keep reading
        buff_index++;
        Serial.print(",");
      } else { // tag complete, now process it
        Serial.println("");


        /* Rough Processing */
        
        int id_begin;
        int id_end;
         
        if (RAW_TAG_LENGTH == 14) {  // RDM6300 reader
          Serial.println((char *)buff);
          //Serial.println(strtol(buff, NULL, 16));
          id_begin = 3;
          id_end   = 10;
        } else if (RAW_TAG_LENGTH == 10) {  // 7941E reader
          id_begin = 4;
          id_end   = 7;
        }

        int id_len = id_end - id_begin;
        char tmp_str[id_len] = "";
        
        for(int n=id_begin; n<=id_end; n++) {
          sprintf(tmp_str + strlen(tmp_str), "%x", buff[n]);
        }
        Serial.println((char *)tmp_str);
        Serial.println(strtol((char *)tmp_str, NULL, 16));
        strncpy(tmp_str, NULL, id_len);

        
        previous_ms = millis();
        resetBuffer();
      }
      
    } else {
      ;
    }
  }

  void RFID::resetBuffer() {
    buff_index = 0;
    strncpy(buff, NULL, RAW_TAG_LENGTH);
  }

  
