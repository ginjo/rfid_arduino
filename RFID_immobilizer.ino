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
 // TODO: √ Sort out git repos in Documents/Arduino folder.
 // TODO: Better code documentation. User documentation.
 // TODO: √ Don't allow admin mode to extend the startup grace period.
 // TODO: Implement a beeper/buzzer.
 // TODO: I think SerialMenu.run_mode should be a global, since it's needed in multiple classes.
 //       But where should it live? Then logging can have its own file/class,
 //       and it can access multiple serial outputs, maybe?
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
 // TODO: √ Complete add-tag and delete-tag processes.
 // TODO: √Save tags to EEPROM.
 // TODO: √ Also implement add-tag-from-scan option.
 // TODO: √ Create version & date-time handling.
 // TODO: √ Reorganize functions so that "rfid" tag functions are in RFID class,
 //       "menu" functions in SerialMenu class, and "settings" functions in Settings class.
 // TODO: Consider using variadic macros to create a LOGGER that can print to variable outputs.
 //       See http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 //       You might need to include stdarg.h.
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
 // TODO: √ Require a CR or CR/LF to enter single characters for SerialMenu.
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
 // TODO: Create protection/failsafe against user-input of bad/empty/out-of-bounds settings.
 //       For example admin_timeout should NEVER go below 5s,
 //       And the initial admin_timeout of 2s should not be modifiable by user.
 //       Generally make sure the Arduino cannot be bricked (requiring a re-flash).
 // TODO: √ Create a setting & control for reader-power-cycle polarity.
 //       Yes, but this is now done in the Reader (sub) classes.
 // TODO: √ Complete the Reader function that decides what reader to use.
 // TODO: Convert settings, readers, led patterns to lists & enums, if possible. (settings are done).
 //       √ Also create settings for all possible current uses of literal data,
 //       mostly numbers, but maybe some strings?
 // DONE: √ Finally fixed the nasty bugs in RFID and Reader, well most anyway. Ongoing search for UB.
 // TODO: √ Try swapping the RFID::loop() functions back to the way they were... Still work?
 // TODO: √ Try converting other RFID uses of tag_last_read_timeout_x_1000 to tagLastReadTimeoutX1000().
 // TODO: - Consider instance vars in Reader for current_tag_id and current_tag_code (hex, I think).
 //       This is trickier that it seems because it would create a curcular requirement between classes.
 //       Turning the logging features into their own class might help.
 // TODO: - I got debug logging to work with BTmenu, but it breaks the RFID cycle.
 //       It prevents the Reader instance for processing the tag. Is this also a UB whack-a-mole thing?
 //       Upon further inspection, the problem appears to be that the RFID serial channel is getting
 //       garbled data, or at least the program is seeing it as garbled. So, I disabled the BTmenu logging.
 // TODO: √ Put all literal data that hasn't been incorporated into Settings into macros.
 // TODO: √ Find a way to use F() to wrap Settings var names in getSettingsByIndex().
 // TODO: √ When power is first applied, reader-reset-pin appears to be held low indefinitely,
 //       and reader failes to read tag (after the first one at boot time).
 //       But even when reader is manually triggered, Ard fails to process the tag successfully.
 //       A warm (not cold!) reboot of the Ard is necessary to get it working again.
 //       Update: this only happens in production mode - in debug mode, the reader works fine. Arrrgg!
 //       √ Update: I might have fixed this - that pin didn't have a pinMode() call, leaving it floating.
 // TODO: √ Booting under external power in debug mode (debug pin) is causing problems,
 //       maybe because of a load-order thing with DPRINT. But this doesn't seem to happen
 //       when powered by USB.
 //       Update: The whole power-on bug was because load order wasn't allowing objects to initialize
 //       before being used.
 // TODO: √ Find string operations that have 0 as an argument, and see if they should use '\0' instead.
 //       According to docs 0 is same as '\0'. They are both int's.
 // TODO: √ Cleanup & fix bugs in SerialMenu UI.
 //       √ Prompt
 //       √ Empty selection for settings-menu.
 // TODO: √ Output Settings list to HW Serial on starup.
 // WARN: Program storage space is at 80% usage.
 //       Might need to move some strings back to SRAM (by eliminating F function for some).
 // TODO: √ Create SETTINGS_VALUE_SIZE and use it for calls to getSettingByIndex().
 // TODO: √ Functionally, everything is working, but S.getChecksum is returning a different number
 //       between boot and first loop.
 //       Update: It appears to be the out-of-range dummy ports. Stop using them!
 //       Ok, this is fixed now! Don't use out-of-range pins, will cause UB.
 // TODO: - With debug pin, don't change S.enable_debug, because saving any setting
 //       will then save enable_debug as 1. Just read the pin directly.
 //       Actually, it's ok, just don't boot with debug pin low for an admin session.
 //       You can always hold debug pin low after boot, for temporary debug mode.
 // TODO: √ Create a Settings function 'debugMode()' which compiles enable_debug with debug-pin.
 // NOTE: Milestone achieved! All basic functions required for real-world use are working.
 //
 // TODO: √ Smooth out UI functionality in SerialMenu - it's still a little confusing what
 //       mode/state/options we're in at each prompt. Maybe there should be no generic prompts,
 //       always give a textual hint.
 // TODO: √Rearrange main .ino file load order, so initial proximity_state gets written out
 //       to master-switch-pin as early as possible.
 // TODO: √ Something is wrong with Tags checksum: it's the same for two different lists!
 // TODO: - Make tags checksum at least 32-bit. Actually, the 16-bit checksum works now.
 // TODO: Don't load all readers, only load the one we're using.
 // TODO: Create an initial_state (for the grace period) setting: 0=off, 1=on, 2=last-known.
 //       This may also require an initialState() function to compile the various settings at runtime.
 //
 // TODO: Move reader functions of RFID class into Reader base,
 //       then use the loaded reader only for reader functionality,
 //       including cycling and tag parsing, but not master-switch management
 //       or led triggering or Tag operations like add, delete, authentication (validation?).
 // TODO: Create a Gate class that handles all the other stuff RFID does now.
 // TODO: Consider again having a Storage class that Tags, Settings, and State all subclass from.
 // TODO: Create a fail-safe button.
 // TODO: Have BTmenu listen on hardware serial as well.
 // TODO: For callback functions or event-response functions, use "onBufferReady()" naming style.
 //       Examples: onTagReady(int tag_id), onMenuAddTag(), onSerialPortData(byte).
 
 
  #include <SoftwareSerial.h>
  // TODO: Implement settings with eeprom, instead of #define macros, something like this:
  #include "settings.h"
  
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "reader.h"
  #include "rfid.h"


  // TODO: Consider moving these to static class vars.

  // Declares blinker LED.
  Led *Blinker;

  // Declares a serial port for admin console.
  SoftwareSerial *BTserial;
  
  // Declares instance of admin console.
  SerialMenu *BTmenu;
  SerialMenu *HWmenu;

  // Declares serial port for RFID reader.
  SoftwareSerial *RfidSerial;//(91,90);
  
  // Declares instance of RFID handler.
  //extern RFID Rfid;
  RFID *Rfid; // used in SerialMenu to add tags.


  void setup() {
    
    // For debugging, so Settings operations can be logged.
    Serial.begin(57600);
    while (! Serial) {
      delay(15);
    }
    delay(15);
    Serial.println(F("Initialized default serial port @ 57600 baud"));
    
    Settings::Load(SETTINGS_EEPROM_ADDRESS);

    // Normal, when debugging not needed.
    Serial.flush();
    Serial.begin(S.HW_SERIAL_BAUD);
    while (! Serial) {
      delay(15);
    }
    delay(15);
    Serial.print(F("Re-initialized serial port from loaded settings: "));
    Serial.println(S.HW_SERIAL_BAUD);

    Serial.print(F("Loaded Settings "));
    Serial.print((char *)S.settings_name);
    Serial.print(F(", with checksum 0x"));
    Serial.print(S.getChecksum(), 16);
    Serial.print(F(", of size "));
    Serial.println(sizeof(S));

    Serial.print(F("Booting RFID Immobilizer, "));
    Serial.print(VERSION);
    Serial.print(F(", "));
    Serial.println(TIMESTAMP);

    // For manual debug/log mode.
    pinMode(S.DEBUG_PIN, INPUT_PULLUP);

    int debug_pin_status = digitalRead(S.DEBUG_PIN);

    Serial.print(F("Debug pin status: "));
    Serial.println(debug_pin_status);

    if (debug_pin_status == LOW) {
      Serial.println(F("Debug pin LOW ... enabling debug"));
      S.enable_debug = 1;
    }
    

    /*  Initialize main objects  */

    Blinker = new Led(S.LED_PIN);

    BTserial = new SoftwareSerial(S.BT_RXTX[0], S.BT_RXTX[1]); // RX | TX
    //BTserial(S.BT_RXTX[0], S.BT_RXTX[1]); // RX | TX

    BTmenu = new SerialMenu(BTserial, Blinker);
    HWmenu = new SerialMenu(&Serial, Blinker);

    RfidSerial = new SoftwareSerial(S.RFID_SERIAL_RX, S.LED_PIN);
    //RfidSerial(S.RFID_SERIAL_RX, S.LED_PIN);

    Rfid = new RFID(RfidSerial, Blinker);


    /*  Run setups  */

    Blinker->StartupBlink();

    // Activates the serial port for admin console.
    BTserial->begin(S.BT_BAUD);

    // Activates the admin console.
    BTmenu->begin();
    HWmenu->begin();

    // Activates the serial port for the RFID handler.
    RfidSerial->begin(S.RFID_BAUD);

    // Activates the RFID handler.
    Rfid->begin();

    // Prints out all settings in tabular format.
    Serial.println();
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
      S.displaySetting(n, output);
      Serial.println(output);
    }

  } // setup()


  void loop() {
    
    Blinker->loop();
    
    if (SerialMenu::run_mode > 0) {
      BTserial->listen();
      //delay(1);
      while (! BTserial->isListening()) delay(15);
      //delay(10);
      delay(BTmenu->get_tag_from_scanner ? 25 : 1);
      BTmenu->loop();
      HWmenu->loop();
    }
  
    if (SerialMenu::run_mode == 0 || BTmenu->get_tag_from_scanner > 0) {
      RfidSerial->listen();
      //delay(1);
      while (! RfidSerial->isListening()) delay(15);
      delay(BTmenu->get_tag_from_scanner ? 50 : 0);
      Rfid->loop();
    }
    
    //  if (BTmenu->get_tag_from_scanner > 0) {
    //    DPRINTLN(F("BTmenu->get_tag_from_scanner > 0"));
    //    RfidSerial->listen();
    //    delay(1);
    //    while (! RfidSerial->isListening()) delay(15);
    //    delay(50);
    //    Rfid->loop();
    //    
    //  } else if (BTmenu->run_mode == 0) {      
    //    RfidSerial->listen();
    //    delay(1);
    //    while (! RfidSerial->isListening()) delay(15);
    //    Rfid->loop();
    //    
    //  }
    
  } // end loop()

  
