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
  SerialMenu BTmenu(&BTserial);

  // Brings up a blinker LED with specified patter of on/off intervals.
  unsigned long blinker_intervals[4] = {50,20,50,2880};
  Led Blinker(9); 

  void setup() {
    Serial.begin(9600);
    delay(100);

    BTserial.begin(9600);
    delay(100);


//  FIX:
//  These two items together crash the arduino repeatedly and quickly.
//  But any other combination of the setup calls, does not crash.

    BTmenu.begin();
    delay(100);

    Blinker.begin(blinker_intervals, 4);
    delay(100);
   
//    BTmenu.begin();
//    delay(100);
  }
  
  void loop() {
    // here is where you put code that needs to be running all the time.
    
    Blinker.loop();
    delay(50);
//
//    BTmenu.loop();
//    delay(50);
  }
  
