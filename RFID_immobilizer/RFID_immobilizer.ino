/* 
 * Passive RFID activated switch, user-controllable with
 * activation delay, for use in generic engine kill module.
 * 
 */

  #include <SoftwareSerial.h>
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "rfid.h"

  // Brings up a simple menu cli on a BT serial port.
  SoftwareSerial BTserial(2, 3); // RX | TX
  SerialMenu BTmenu(&BTserial);

  // Brings up a blinker LED.
  Led Blinker(9);

  // The RFID reader.
  SoftwareSerial RDM6300(7, 8);
  RFID Rfid(&RDM6300);


  void setup() {
    unsigned long blinker_intervals[INTERVALS_LENGTH] = {50,20,50,2880};
    Blinker.begin(blinker_intervals);
    
    Serial.begin(9600);

    BTserial.begin(9600);

    BTmenu.begin();
    BTmenu.resetAdmin(2);

    //  unsigned long blinker_intervals[INTERVALS_LENGTH] = {50,20,50,2880};
    //  Blinker.begin(blinker_intervals);

    RDM6300.begin(9600);
    Rfid.begin();
  }
  
  void loop() {
    // here is where you put code that needs to be running all the time.
    
    Blinker.loop();
    //delay(2);

    if (BTmenu.run_mode > 0) {
      BTserial.listen();
      delay(2);
      BTmenu.loop();
      //delay(2);
      
    } else {
      RDM6300.listen();
      delay(1);
      Rfid.loop();
      //delay(2);
    }
  }
  
