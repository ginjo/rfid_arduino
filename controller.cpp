  #include "controller.h"
  #include <EEPROM.h>

  // Constructors
  // Receives a Reader and an array of Led objecs (*RGB[] from .ino file).
  Controller::Controller(Reader *_reader, Led *_blinker[], Led *_beeper) :
    //proximity_state(0),
    proximity_state(S.proximity_state_startup == 2 ? EEPROM.read(0) : S.proximity_state_startup),
    reader(_reader),
    blinker(_blinker),
    beeper(_beeper),
    ctrl_status(0)
  { ; }


  // TODO: Some (all?) of this needs to be moved to Reader setup().
  void Controller::begin() {
    // Make sure to run this somewhere, if not here.
    initializeOutput();
    
    // Sets local 'reader' to instance of Reader.
    //reader = GetReader(S.default_reader); // Moving to main .ino, to be passed into Controller::Controller().

    LOG(4, F("Starting Controller with "), false);
    LOG(4, reader->name(), false);
    LOG(5, F(", proximity "), false);
    LOG(5, proximity_state, false);
    LOG(5, F(", switch pin "), false);
    LOG(5, OUTPUT_SWITCH_PIN, false);
    LOG(4, "", true);
        
    // Initializes the reader power/reset control.
    //digitalWrite(READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
    // This first one is to clear any built-up charge, as something is holding the reader low at startup.
    digitalWrite(READER_POWER_CONTROL_PIN, reader->power_control_logic ? LOW : HIGH);
    delay(50);
    digitalWrite(READER_POWER_CONTROL_PIN, reader->power_control_logic ? HIGH : LOW);
    
  } // setup()


  void Controller::loop() {
    reader->loop();
    proximityStateController();
  }


  // Initializes output switch.
  void Controller::initializeOutput() {
        
    // Starts up with whatever state we left off in.
    // Protects against thief using 'admin' to move
    // in fits and starts, since a failed proximity
    // timeout will set S.proximity_state to 0.
    // This also tells the proximityStateController
    // where we left off at power-down (or reset).
    //proximity_state = S.proximity_state;

    blinker[0]->off();
    blinker[1]->off();
    blinker[2]->off();
    beeper->off();

    LOG(4, F("Setting outpt switch per prox-state: "));
    LOG(4, proximity_state, true);
    // Switches the main load according to current proximity_state.
    // This turns on the load if saved prox-state was "on".
    // This begins the courtesey grace period until the system can
    // start processing tags (at which time, it will immediately
    // shut down output until a successful tag read).
    digitalWrite(OUTPUT_SWITCH_PIN, proximity_state);
    if (proximity_state) {
      blinker[1]->startupBlink();
    } else {
      blinker[0]->startupBlink();
    }
  }


  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void Controller::proximityStateController() {
    CT_LOG(6, F("*** proximityStateController()"), true);
    CT_LOG(6, F("last_tag_read_ms: "), false); CT_LOG(6, reader->last_tag_read_ms, true);
    CT_LOG(6, F("last_reader_power_cycle_ms: "), false); CT_LOG(6, reader->last_reader_power_cycle_ms, true);
    CT_LOG(6, F("msSinceLastTagRead(): "), false); CT_LOG(6, reader->msSinceLastTagRead(), true);
    CT_LOG(6, F("msSinceLastReaderPowerCycle(): "), false); CT_LOG(6, reader->msSinceLastReaderPowerCycle(), true);
    CT_LOG(6, F("ms_reader_cycle_total: "), false); CT_LOG(6, reader->ms_reader_cycle_total, true);
    CT_LOG(6, F("tagLastReadTimeoutX1000(): "), false); CT_LOG(6, reader->tagLastReadTimeoutX1000(), true);
    CT_LOG(6, F("***"), true);
    

    if (
    // If NO TAG READ AT STARTUP and reader has recently power cycled.
    // This should probably calculate or use global setting for
    // appropriate time-to-wait since last power cycle.

      reader->last_tag_read_ms == 0UL &&
      reader->last_reader_power_cycle_ms > 0UL &&
      reader->msSinceLastReaderPowerCycle() > 2000UL
      ){

      if (ctrl_status != 1) LOG(3, F("TIMEOUT: grace period"), true); // only prints once.
      
      CT_LOG(6, F("Timeout: grace"), true);
      blinker[0]->slowBlink();
      blinker[1]->off();
      //blinker[2]->off();
      beeper->slowBeep(3);
      setProximityState(0);
      reader->reader_power_cycle_high_duration = 3UL;

      ctrl_status = 1;
    
    } else if (
    // If last read is beyond TIMEOUT, and we've cycled reader at least once in that interval.
      
      reader->msSinceLastTagRead() > reader->tagLastReadTimeoutX1000() &&
      reader->last_reader_power_cycle_ms > 0UL &&
      reader->msSinceLastTagRead() > reader->msSinceLastReaderPowerCycle() &&
      reader->msSinceLastReaderPowerCycle() > 2000UL
      ){

      if (ctrl_status != 2) LOG(3, F("TIMEOUT: general"), true); // only prints once.
      
      CT_LOG(6, F("Timeout: general"), true);
      blinker[0]->slowBlink();
      blinker[1]->off();
      //blinker[2]->off();
      beeper->slowBeep(3);
      setProximityState(0);
      reader->reader_power_cycle_high_duration = 3UL;

      ctrl_status = 2;

    } else if (
    // If last read is greater than reader-power-cycle-total AND
    // less than final timeout total, we're in the AGING zone.
    
      reader->last_tag_read_ms > 0UL &&
      reader->msSinceLastTagRead() > reader->ms_reader_cycle_total &&
      reader->msSinceLastTagRead() <= reader->tagLastReadTimeoutX1000()
      ){

      if (ctrl_status != 3) LOG(3, F("AGING"), true); // only prints once.

      CT_LOG(6, F("Aging"), true);
      if (proximity_state) {
        blinker[0]->off();
        blinker[1]->fastBlink();
      } else {
        blinker[0]->fastBlink();
        blinker[1]->off();        
      }
      
      //blinker[2]->off();
      beeper->fastBeep();
      
      setProximityState(1);
      reader->reader_power_cycle_high_duration = 3UL;

      ctrl_status = 3;

    } else if (
    // If we're FRESH and still young.
    
      reader->last_tag_read_ms > 0UL &&
      reader->msSinceLastTagRead() <= reader->ms_reader_cycle_total
      ){

      if (ctrl_status != 4) LOG(5, F("FRESH"), true); // only prints once.
        
      CT_LOG(6, F("Fresh"), true);
      blinker[0]->off();
      blinker[1]->steady();
      //blinker[2]->off();
      beeper->off();
      setProximityState(1);
      reader->reader_power_cycle_high_duration = 0UL;

      ctrl_status = 4;

    
    } else {
    // No expected condition was met (not sure what to do here, if anything).
    // This condition is not necessarily a problem.

      blinker[2]->off();

      if (ctrl_status != 0) LOG(5, F("PASS"), true); // only prints once.
      
      CT_LOG(6, F("Pass"), true); // always prints

      // Only changes status if debugging, otherwise info-level debug will show
      // some statuses (like TIMEOUT) at every loop.
      if (LogLevel() >=5) ctrl_status = 0; 
    }

    // TODO: Is there a better place for this? UPDATE: I don't think so.
    if (Menu::run_mode == 0) digitalWrite(OUTPUT_SWITCH_PIN, proximity_state);
    
  } // proximityStateController()

  int Controller::setProximityState(int _state) {
    if (Menu::run_mode == 0) {
      int previous_proximity_state = proximity_state;
      proximity_state = _state;

      if (proximity_state != previous_proximity_state && S.proximity_state_startup == 2) {
        LOG(4, F("Calling EEPROM.update with prox-state: "));
        LOG(4, proximity_state, true);
        EEPROM.update(STATE_EEPROM_ADDRESS, proximity_state);
      }
    }
    return proximity_state;
  }


  /*  Static Vars & Functions  */

  // This is for aliasing the original Tags list to the new Tags list.
  // See beginning of .h file for typedef.
  // This is not currently used, but leaving here as example
  // of c++ variable aliasing (referencing).
  //TagArray& Controller::Tags = Tags::TagSet.tag_array;

  
