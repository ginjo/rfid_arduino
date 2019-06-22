/* 
 * Passive RFID activated switch, user-controllable with
 * activation delay, for use in generic engine kill module.
 * 
 */

 // TODO: Change all 'previous_ms' to 'last_<whatever>'.
 // TODO: √ Close fuel switch at startup, so we don't have to wait for software load.
 //       But track if there was a 'fatal' missing-tag timeout, using EEPROM.
 //       If there WAS a previous timeout, keep fuel switch OPEN at startup,
 //       until a valid tag is read (then clear the 'fatal' event from EEPROM.
 // TODO: √ LED blinker needs a refactor: 1. Simplify, 2. Allow cycle count passed to begin();
 //       Done except for cycle count.
 // TODO: Implement blinker cycle limit setting.
 // TODO: √ I think macros need to be coordinated between each file, since they can clobber each other.
 //       Most macros can be turned into settings (S.<setting>).
 // TODO: √ Create menu options to set global variables. See above about macros.
 // TODO: - Create var-to-eeprom address mapping for common settings (to be editable by menu).
 //       I think this is no longer needed, since we're using EEPROM.put(<a-struct-object>).
 // TODO: √ Provide standard message when admin menu exits/transitions to run mode.
 // TODO: Sort out git repos in Documents/Arduino folder.
 // TODO: Better code documentation. User documentation.
 // TODO: √ Don't allow admin mode to extend the startup grace period.
 // TODO: Implement a beeper/buzzer.
 // TODO: I think SerialMenu.run_mode should be a global, since it's needed in multiple classes.
 //       But where should it live?
 // TODO: √ Consolidate SerialMenu handling of input between checkSerialPort() and runCallbacks(),
 //       so basically everything should be a callback.
 // TODO: √ Create exitAdmin() function for cleanup & logging.
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
 // TODO: √ Reduce active logging lines in SerialMenu serial port reading.
 // TODO: √ Show S.<settings> values along with list (in admin mode).
 // TODO: Complete add-tag and delete-tag processes.
 // TODO: Also implement add-tag-from-scan option.
 // TODO: √ Create version & date-time handling.
 // TODO: Reorganize functions so that rfid tag functions are in RFID class,
 //       menu functions in SerialMenu class, and settings functions in Settings class.
 // TODO: Consider using variadic macros to create a LOGGER that can print to variable outputs.
 //       See http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 // TODO: √ Tag output to serial console is always the same tag, regardless of what was scanned.
 //       This holds true even after cold restart of entire board.
 // TODO: √ Need to ignore READER_CYCLE_HIGH_DURATION at beginning of run mode,
 //       and cycle reader immediately if no tag found yet. Then immediately
 //       timeout the tag if still not found.
 //       This is all because READER_CYCLE_HIGH_DURATION could be set rather high,
 //       and would allow too much grace period at startup.
 // TODO: √ cycleReaderPower() isn't triggering when READER_CYCLE_HIGH_DURATION is set to 50,
 //       and tag-timeout is set to 60. So there's a timing issue when settings are not 5, 25 respectively.
 //       This was an issue with integer arithmatic and not enough bits.
 // TODO: √ Maybe refactor proximityStateController() to have smaller conditions and more else-if blocks.
 //       Also move the subconditions to a single flat if-then-else statement.
 // TODO: Require a CR or CR/LF to enter single characters for SerialMenu.
 //       This might just be a matter of always using 'line' mode instead of 'char' mode.
 //       This is necessary to use some terminal apps like BLE-Terminal on ios
 //       to use the BLE (HM-10) adapter on arduino. Also consider HM-12 for dual BT access (2.1, 4.0).
 // TODO: √ If proximity state goes low, set reader-cycle-high duration to 3 or 5 seconds.
 //       Otherwise a flakey tag sitting on top of the reader may miss its only change to be read
 //       in the scenario where tag-timeout and reader-cycle are both set very high.
 //       May need a class-global variable for that purpose, so we don't have to mess
 //       with the S.<setting>.
 // TODO: √ In normal operation, after admin session timeout (or '0' quit),
 //       proximity-state is being briefly set to 0, even though the reader hasn't cycled yet.
 //       This needs to be fixed so startup grace-period NEVER times out without
 //       first trying a reader power cycle.
 // TODO: √ Finish converting reader_power_cycle_high_duration to readerPowerCycleHighDuration()
 // TODO: Create protection/failsafe against user-input of bad settings.
 //       For example admin_timeout should NEVER go below 5s,
 //       And the initial admin_timeout of 2s should not be modifiable by user.
 //       Generally make sure the Arduino cannot be bricked (requiring a re-flash).
 // TODO: Create a setting & control for reader-power-cycle polarity.

 
 
  #include <SoftwareSerial.h>
  // TODO: Implement settings with eeprom, instead of #define macros, something like this:
  #include "settings.h";
  
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "reader.h"
  #include "rfid.h"

  // Brings up a blinker LED.
  Led Blinker(8);

  // Creates serial port for admin console.
  SoftwareSerial BTserial(2, 3); // RX | TX

  // Creates instance of admin console.
  SerialMenu BTmenu(&BTserial, &Blinker);

  // Creates serial port for RFID reader.
  SoftwareSerial RfidSerial(4,6); // not sending anything to reader, so tx on unused pin

  // Creates instance of RFID reader.
  Reader * RfidReader = Readers[2];

  // Creates instance of RFID handler.
  // (RfidReader is already a pointer, so we don't use '&' reference).
  RFID Rfid(&RfidSerial, &Blinker, RfidReader);


  void setup() {    
    Serial.begin(9600);
    while (! Serial) {
      delay(10);
    }

    Serial.print(F("Booting RFID Immobilizer, "));
    Serial.print(VERSION);
    Serial.print(", ");
    Serial.println(TIMESTAMP);

    // Calls global function to populate the Readers array of pointers to specific reader subclasses.
    readerArraySetup();

    // Calls the fast-startup-blinker pattern.
    Blinker.StartupBlink();

    // Activates the serial port for admin console.
    BTserial.begin(9600);

    // Activates the admin console.
    BTmenu.begin();

    // Activates the serial port for the RFID handler.
    RfidSerial.begin(9600);

    // Activates the RFID handler.
    Rfid.begin();

  } // end setup


  void loop() {
    // here is where you put code that needs to be running all the time.
    
    Blinker.loop();
    
    //  unsigned long current_ms = millis();
    //  if (current_ms/1000 > 17) {Blinker.SlowBlink();}
    //  else if (current_ms/1000 > 12) {Blinker.FastBlink();}
    //  else if (current_ms/1000 > 7) {Blinker.Steady();}
    
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

  
