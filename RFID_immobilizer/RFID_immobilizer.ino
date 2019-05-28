/* 
 * Passive RFID activated switch, user-controllable with
 * activation delay, for use in generic engine kill module.
 * 
 */

  #include <SoftwareSerial.h>
  #include "led_blinker.h"
  #include "serial_menu.h"

  // Brings up a simple menu cli on a BT serial port.
  // Don't use both of these lines at the same time.
  SoftwareSerial BTserial(2, 3); // RX | TX
  //SerialMenu BTmenu(2, 3, 9600);
  SerialMenu BTmenu(BTserial);

  // Brings up a blinker LED with specified patter of on/off intervals.
  int blinker_intervals[4] = {50,20,50,2880};
  Led Blinker(9); 

  void setup() {
    Serial.begin(9600);
    BTserial.begin(9600);
    delay(100);
    BTmenu.setup();
    Blinker.setup(blinker_intervals, 4);
  }
  
  void loop() {
    // here is where you put code that needs to be running all the time.
    
    BTmenu.loop();
    Blinker.loop();

    // only for debugging
    //delay(500);
  }
  
