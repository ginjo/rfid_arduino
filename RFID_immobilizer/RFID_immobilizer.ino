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
 // TODO: LED blinker needs a refactor: 1. Simplify, 2. Allow cycle count passed to begin();
 // TODO: I think macros need to be coordinated between each file, since they can clobber each other.
 // TODO: Implement blinker cycle limit setting.
 // TODO: Create menu options to set global variables.
 // TODO: Create var-to-eeprom address mapping for common settings (to be editable by menu).
 // TODO: Provide standard message when admin menu exits/transitions to run mode.
 // TODO: Sort out git repos in Documents/Arduino folder.
 // TODO: Better code documentation. User documentation.

  #include <SoftwareSerial.h>
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "rfid.h"
  #include "rdm6300_lib_example.cpp"


  // Brings up a simple menu cli on a BT serial port.

  // Brings up a blinker LED.
  Led Blinker(9);
  
  SoftwareSerial BTserial(2, 3); // RX | TX
  SerialMenu BTmenu(&BTserial, &Blinker);

  // The RFID reader.
  SoftwareSerial RfidSerial(5,4);
  RFID Rfid(&RfidSerial, &Blinker);

  // Rdm3600 library example
  //Rdm6300LibExample RdmExample; 

  void setup() {    
    Serial.begin(9600);
    while (! Serial) {
      delay(10);
    }
    
    //  int blinker_intervals[INTERVALS_LENGTH] = {50,20,50,2880};
    int blinker_intervals[INTERVALS_LENGTH] = {485,15};
    Blinker.begin(0, blinker_intervals);

    BTserial.begin(9600);

    BTmenu.begin();
    BTmenu.resetAdmin(2);

    RfidSerial.begin(9600);
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
      RfidSerial.listen();
      Rfid.loop();
      delay(1);

      //RdmExample.rdm6300._software_serial->listen();
      //RdmExample.loop();
    }
  }

  
