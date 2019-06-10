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
 // TODO: √ LED blinker needs a refactor: 1. Simplify, 2. Allow cycle count passed to begin();
 //       Done except for cycle count.
 // TODO: Implement blinker cycle limit setting.
 // TODO: I think macros need to be coordinated between each file, since they can clobber each other.
 // TODO: Create menu options to set global variables.
 // TODO: Create var-to-eeprom address mapping for common settings (to be editable by menu).
 // TODO: Provide standard message when admin menu exits/transitions to run mode.
 // TODO: Sort out git repos in Documents/Arduino folder.
 // TODO: Better code documentation. User documentation.
 // TODO: √ Don't allow admin mode to extend the startup grace period.
 // TODO: Implement a beeper/buzzer.
 // TODO: I think SerialMenu.run_mode should be a global, since it's needed in RFID class.
 // TODO: Consolidate SerialMenu handling of input between checkSerialPort() and runCallbacks(),
 //       so basically everything should be a callback.
 // TODO: Create exitAdmin() function for cleanup & logging.
 // TODO: √ Make the common blink patterns into constants or S.<setting>.
 //       fast_blink_intervals[], slow_blink_intervals, startup_admin_timeout_intervals[], etc..
 //       Still need to store these common intervals in S.<settings>.
 // TODO: √ Startup grace period needs to END immediately after startup,
 //       if NO tags are found after the FIRST reader power cycle.
 //       √ At this point, proximity_state and S.proximity_state need to be fully timed-out.
 // TODO: Consider renaming S.proximity_state to S.last_proximity_state.
 // TODO: √ Fix bug in addTagString (I think it's input_mode issue).
 // TODO: √ Consider a function setProximityState(on-or-off, hight-or-low, whatever).
 // TODO: Add status_text and updateStatusText() to efficiently display status change in logs.
 //       This should display proximity_state status as text-label, whenever it changes.
 //       Use enum StatusText {recent, aging, expired}
 // TODO: Reduce active logging lines in SerialMenu serial port reading.
 // TODO: Show S.<settings> values along with list (in admin mode).
 // TODO: Complete add-tag and delete-tag processes.
 // TODO: Also implement add-tag-from-scan option.
 

  #include <SoftwareSerial.h>
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "rfid.h"
  #include "rdm6300_lib_example.cpp"

  // TODO: Implement settings with eeprom, instead of #define macros, something like this:
  #include "settings.h";  

  // Brings up a simple menu cli on a BT serial port.

  // Brings up a blinker LED.
  Led Blinker(8);
  
  SoftwareSerial BTserial(2, 3); // RX | TX
  SerialMenu BTmenu(&BTserial, &Blinker);

  // The RFID reader.
  SoftwareSerial RfidSerial(4,6); // not sending anything to reader, tx on unused pin
  RFID Rfid(&RfidSerial, &Blinker);

  // Rdm3600 library example
  //Rdm6300LibExample RdmExample; 

  void setup() {    
    Serial.begin(9600);
    while (! Serial) {
      delay(10);
    }

    Serial.println(F("Booting RFID Immobilizer..."));

    //Serial.print(F("Settings loaded reader_cycle_timeout_ms: "));
    //Serial.println(Settings.reader_cycle_timeout_ms);
    
    //  int blinker_intervals[INTERVALS_LENGTH] = {50,20,50,2880}; // slow pilot light
    //int blinker_intervals[INTERVALS_LENGTH] = {480,20}; // 120bpm mostly-on twinkle
    //Blinker.begin(0, blinker_intervals);
    Blinker.startupBlink();

    BTserial.begin(9600);

    BTmenu.begin();
    //BTmenu.setAdminWithTimeout(2);

    RfidSerial.begin(9600);
    Rfid.begin();

    //RdmExample.begin();
    
  } // end setup
  
  void loop() {
    // here is where you put code that needs to be running all the time.
    
    Blinker.loop();

    if (BTmenu.run_mode > 0) {
      BTserial.listen();
      delay(1);
      BTmenu.loop();
      
    } else {
      //RdmExample.rdm6300._software_serial->listen();
      //RdmExample.loop();
      
      RfidSerial.listen();
      Rfid.loop();
      delay(1);
    }

  } // end loop

  
