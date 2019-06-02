/* 
 * Passive RFID activated switch, user-controllable with
 * activation delay, for use in generic engine kill module.
 * 
 */

 // TODO: Change all 'previous_ms' to 'last_<whatever>'.
 // TODO: Close fuel switch at startup, so we don't have to wait for software load.
 //       But track if there was a 'fatal' missing-tag timeout, using EEPROM.
 //       If there WAS a previous timeout, keep fuel switch OPEN at startup,
 //       until a valid tag is read (then clear the 'fatal' event from EEPROM.
 

  #include <SoftwareSerial.h>
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "rfid.h"
  #include "rdm6300_lib_example.cpp"


  // Brings up a simple menu cli on a BT serial port.
  SoftwareSerial BTserial(2, 3); // RX | TX
  SerialMenu BTmenu(&BTserial);

  // Brings up a blinker LED.
  Led Blinker(9);

  // The RFID reader.
  SoftwareSerial RDM6300(5,4);
  RFID Rfid(&RDM6300);

  // Rdm3600 library example
  //Rdm6300LibExample RdmExample; 

  void setup() {
    Serial.begin(9600);
    
    delay(1000);
    
    unsigned long blinker_intervals[INTERVALS_LENGTH] = {50,20,50,2880};
    Blinker.begin(blinker_intervals);

    BTserial.begin(9600);

    BTmenu.begin();
    BTmenu.resetAdmin(2);

    RDM6300.begin(9600);
    Rfid.begin();

    //RdmExample.begin();
  }
  
  void loop() {
    // here is where you put code that needs to be running all the time.
    
    Blinker.loop();

    if (BTmenu.run_mode > 0) {
      BTserial.listen();
      delay(1);
      BTmenu.loop();
      
    } else {
      RDM6300.listen();
      Rfid.loop();
      delay(20);

      //RdmExample.rdm6300._software_serial->listen();
      //RdmExample.loop();
    }
  }

  
