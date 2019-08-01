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
 //       Update: Maybe run_mode should be a member of the RFID class (Controller class?).
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
 // TODO: - Consider instance vars in Reader for last_tag_read_id and current_tag_code (hex, I think).
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
 //       maybe because of a load-order thing with INO_PRINT. But this doesn't seem to happen
 //       when powered by USB.
 //       Update: The whole power-on bug was because load order wasn't allowing objects to initialize
 //       before being used.
 // TODO: √ Find string operations that have 0 as an argument, and see if they should use '\0' instead.
 //       According to docs 0 is same as '\0'. They are both int's.
 // TODO: √ Cleanup & fix bugs in SerialMenu UI.
 //       √ Prompt
 //       √ Empty selection for settings-menu.
 // TODO: √ Output Settings list to HW Serial on starup.
 // WARN: - Program storage space is at 80% usage.
 //       - Might need to move some strings back to SRAM (by eliminating F function for some).
 //       Update: This problem has been aleviated with class-specific debug control.
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
 // NOTE: √ Milestone achieved! All basic functions required for real-world use are working.
 //
 // TODO: √ Smooth out UI functionality in SerialMenu - it's still a little confusing what
 //       mode/state/options we're in at each prompt. Maybe there should be no generic prompts,
 //       always give a textual hint.
 // TODO: √Rearrange main .ino file load order, so initial proximity_state gets written out
 //       to master-switch-pin as early as possible.
 // TODO: √ Something is wrong with Tags checksum: it's the same for two different lists!
 // TODO: - Make tags checksum at least 32-bit. Actually, the 16-bit checksum works now.
 // TODO: √ Don't load all readers, only load the one we're using.
 // TODO: Create an initial_state (for the grace period) setting: 0=off, 1=on, 2=last-known (S.proximity_state).
 //       This may also require an initialState() function to compile the various settings/options at runtime.
 //
 //
 // FOR REFACTOR:
 //
 // TODO: √ Move reader functions of RFID class into Reader base,
 //       then use the loaded reader only for reader functionality,
 //       including cycling and tag parsing, but not master-switch management
 //       or led triggering or Tag operations like add, delete, authentication (validation?).
 //       √ Maybe move reader subclasses to their own readers.cpp file.
 // TODO: Create a Gate/Switch/Controller/State? class that handles all the other stuff RFID does now.
 //       This class should take a Reader and Blinker instance onboard, since it's the glue
 //       between those two entities.
 // TODO: √ Then create a Tags class that manages everything tag-related (that's not part of Reader).
 // TODO: √ Consider again having a Storage class that Tags, Settings, and State all subclass from.
 // 
 // TODO: Create a fail-safe button and/or a restore-defaults button.
 // TODO: √ Have BTmenu listen on hardware serial as well.
 // TODO: - For callback functions or event-response functions, consider "onBufferReady()" naming style.
 //       Examples: onTagReady(int tag_id), onMenuAddTag(), onSerialPortData(byte).
 //       Really? Do we want truly event-driven behavior? Would it be better to put a ready-tag
 //       in a static-member var, and let whatever function pick it up?
 //       Update: What we've done in SerialMenu with callbacks is sorta like this.
 // TODO: √ Change Settings::current to title-case Settings::Current.
 // TODO: √ Don't use strcpy or strncpy. DO use strlcpy (it ensures a null terminator).
 // NOTE: √ Milestone achieved! Created generic Storage base class, that is also a class template (CRTP).
 //       √ Settings are currently using this... next are tags (need a Tags class where 1 record is array of tags).
 // TODO: √ Refactor Storage Class, saving all needed data (including EEPROM address) in Storage instance.
 //       See storage.h. (also maybe mentioned in other places too, like Tags. storage.h should be the official TODO).
 // NEXT: √ Storage refactor compiles. Now need to decouple it from the other classes, then review code, then try it.
 //       √ This has been done with Tags, next do with Settings (then with State - for proximity_state).
 // TODO: Add validation code to storage.h to handle bad storage_name or bad eeprom_address.
 // TODO: Need validation routine in Tags or Reader to handle bad tag-id.
 // TODO: √ Handle reader looping for add-tag entirely within serial-menu class.
 //       √ Reader class should be unaware of SerialMenu instances.
 // TODO: √ Implement more efficient GetReader, and move to Reader::GetReader().
 //       See example file "C++ polymorphism with factory pattern in base.cpp".
 // TODO: Can all specific reader declarations/definitions go into a single file "readers.cpp" ???
 // TODO: √ Consider function pointers for SerialMenu callbacks, instead of the large switch/if/then statements.
 // TODO: √ menuAddTag and addTag are all messed up, in different ways, for both HW & SW interfaces.
 //       √ Need to make sure that SerialMenu class always cleans up after doing anything with menuAddTag.
 //       √ Currently, a messy state causes the run-level 0 to behave eratically, switching on/off the proximity-state
 //       eeeprom setting repeatedly.
 //       √ Letting SerialMenu timeout naturally and go into run-mode 0, also leaves a mess,
 //       yielding authorized tags but never activating the switch.
 //       LED remains in boot state.
 //       Update: I think most of this issue is solved was UB from out-of-mem.
 //       Double-check on BTserial interface.
 // TODO: Remove stuff from SerialMenu and Reader that isn't used any more.
 //       Remember to check obsolete macros and global vars in .h files.
 // TODO: √ Include uptime in cycleReaderPower periodic output.
 // TODO: Don't forget failsafe mode and reset-factory-defaults command.
 // TODO: - Undo the timing changes around calls to listen() for both soft-serial ports.
 //       Then split the whole menuAddTag into two functions: one for scanner and one for tty serial.
 // TODO: √ Consider disabling all clearing of serial ports, unless they are solving a specific problem.
 //       It's only done once anyway.
 // TODO: √ Longer delays may actually hinder serial ports. Consider shorter delays placed more strategically,
 //       like as long before the port needs to be read as possible. 
 // TODO: Add settings for rfid-reader-add-tag-delay and menu-add-tag-delay,
 //       so they can be adjusted without re-flashing device.
 // TODO: Implement backspace/delete (ascii 127, 8) for tty. May need to use form-feed chr also (12).
 // TODO: Make sure all 3 readers are working.
 //       Do all readers start with 2 and end with 3 ?
 //       Is each specific Reader subclass storing everything it needs for tag processing?
 // TODO: √ Create FREERAM macro that only runs if debug mode. Use it everywhere u have FreeRam() now,
 //       except at startup & other key places.

 
 
 
 
  #include <Arduino.h>
  #include <SoftwareSerial.h>
  #include "settings.h"
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "rfid.h"

  // Comment this line to disable debug code for this class.
  //#define INO_DEBUG
  #ifdef INO_DEBUG
    #define INO_PRINT(...) DPRINT(__VA_ARGS__)
    #define INO_PRINTLN(...) DPRINTLN(__VA_ARGS__)
  #else
    #define INO_PRINT(...)
    #define INO_PRINTLN(...)
  #endif


  // Declares blinker LED.
  Led *Blinker;

  // Declares a software-serial port for admin console.
  SoftwareSerial *BTserial;

  // Declares serial port for RFID reader.
  SoftwareSerial *RfidSerial;//(91,90);

  // Declares rfid reader instance.
  Reader *RfidReader;
  
  // Declares instance of RFID handler.
  RFID *Rfid;


  void setup() {
    
    // For debugging, so Settings operations can be logged.
    Serial.begin(57600);
    while (! Serial) delay(10);
    delay(15);
    Serial.println(F("Initialized default serial port @ 57600 baud"));

    FreeRam("setup() pre load-stngs");
    
    //Settings::Load(SETTINGS_EEPROM_ADDRESS);
    //Settings::Load(&Settings::Current);
    Settings::Load();

    // Normal, when debugging not needed.
    Serial.flush();
    Serial.begin(S.HW_SERIAL_BAUD);
    while (! Serial) delay(10);
    delay(15);
    Serial.print(F("Re-initialized serial port with loaded settings: "));
    Serial.println(S.HW_SERIAL_BAUD);

    Serial.print(F("Loaded Settings '"));
    Serial.print((char *)S.settings_name);
    //Serial.print(F("' with checksum 0x"));
    //Serial.print(S.calculateChecksum(), 16);
    Serial.print(F("' of size "));
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

    // Prints out all settings in tabular format.
    Serial.println();
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
      S.displaySetting(n, output);
      Serial.println(output);
    }
    Serial.println();

    FreeRam("setup() pre obj new");

    /*  Initialize main objects  */

    Blinker = new Led(S.LED_PIN);

    BTserial = new SoftwareSerial(S.BT_RXTX[0], S.BT_RXTX[1]); // RX | TX

    RfidSerial = new SoftwareSerial(S.RFID_SERIAL_RX, S.LED_PIN);

    RfidReader = Reader::GetReader(S.DEFAULT_READER);
    RfidReader->serial_port = RfidSerial;
    
    SerialMenu::HW = new SerialMenu(&Serial, RfidReader, Blinker, "HW");
    SerialMenu::SW = new SerialMenu(BTserial, RfidReader, Blinker, "SW");

    Rfid = new RFID(RfidReader, Blinker);

    FreeRam("setup() pre obj stp");
    
    
    /*  Run setups  */

    Rfid->initializeOutput();

    Blinker->StartupBlink();

    // Activates the software-serial port for admin console.
    BTserial->begin(S.BT_BAUD);

    // Loads tags to default location (Tags::TagSet).
    Tags::Load();

    // Activates the admin console.
    SerialMenu::Begin();

    // Activates the serial port for the RFID handler.
    RfidSerial->begin(S.RFID_BAUD);

    // Activates the RFID handler.
    Rfid->begin();

    FreeRam("end setup()");
    

  } // setup()


  void loop() {
    
    Blinker->loop();

    if (SerialMenu::run_mode > 0) {
      SerialMenu::Loop();
    } else if (SerialMenu::run_mode == 0) {
      RfidReader->loop();
      Rfid->loop();      
    }
    
  } // end loop()

  
