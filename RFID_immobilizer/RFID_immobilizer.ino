/* 
 * Passive RFID activated switch, user-controllable with
 * activation delay, for use in generic engine kill module.
 * 
 */

  #include <SoftwareSerial.h>
  #include "led_blinker.h"
  #include "serial_menu.h"

  // Brings up a simple menu cli on a BT serial port.
  SoftwareSerial BTserial(2, 3); // RX | TX
  SerialMenu BTmenu(&BTserial);

  // Brings up a blinker LED.
  Led Blinker(9);

  void setup() {
    Serial.begin(9600);

    BTserial.begin(9600);

    BTmenu.begin();

    unsigned long blinker_intervals[INTERVALS_LENGTH] = {50,20,50,2880};
    Blinker.begin(blinker_intervals);
  }
  
  void loop() {
    // here is where you put code that needs to be running all the time.
    
    Blinker.loop();
    delay(10);

    BTmenu.loop();
    delay(10);
  }
  
