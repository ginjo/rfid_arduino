  #include "controller.h"
  #include "global.h"
  #include "settings.h"
  #include "reader.h"
  #include "menu.h"
  
  #include <EEPROM.h>

  // Constructors
  Controller::Controller(Reader *_reader) :
    //proximity_state(0),
    proximity_state(S.proximity_state_startup == 2 ? EEPROM.read(STATE_EEPROM_ADDRESS) : S.proximity_state_startup),
    reader(_reader),
    ctrl_status(0)
  { ; }


  // TODO: Some (all?) of this needs to be moved to Reader setup().
  void Controller::begin() {
    // Make sure to run this somewhere, if not here.
    initializeOutput();

    LOG(4, F("Starting Controller with "), false);
    LOG(4, reader->name(), false);
    LOG(5, F(", state "), false);
    LOG(5, proximity_state, false);
    LOG(5, F(", out pin "), false);
    LOG(5, OUTPUT_SWITCH_PIN, false);
    LOG(4, "", true);
        
    // Initializes the reader power/reset control.
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


    LOG(4, F("Setting outpt switch: "));
    LOG(4, proximity_state, true);
    // Switches the main load according to current proximity_state.
    // This turns on the load if saved prox-state was "on".
    // This begins the courtesey grace period until the system can
    // start processing tags (at which time, it will immediately
    // shut down output until a successful tag read).
    digitalWrite(OUTPUT_SWITCH_PIN, proximity_state);

    RGB[0]->off();
    RGB[1]->off();
    RGB[2]->off();
    Beeper->off();
    
    if (proximity_state) {
      RGB[1]->startupBlink();
    } else {
      RGB[0]->startupBlink();
    }
  }


  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void Controller::proximityStateController() {
    CT_LOG(6, F("* Controller *"), true);
    CT_LOG(6, F("last_tag_read_ms "), false); CT_LOG(6, reader->last_tag_read_ms, true);
    CT_LOG(6, F("last_reader_power_cycle_ms "), false); CT_LOG(6, reader->last_reader_power_cycle_ms, true);
    CT_LOG(6, F("msSinceLastTagRead() "), false); CT_LOG(6, reader->msSinceLastTagRead(), true);
    CT_LOG(6, F("msSinceLastReaderPowerCycle() "), false); CT_LOG(6, reader->msSinceLastReaderPowerCycle(), true);
    CT_LOG(6, F("ms_reader_cycle_total "), false); CT_LOG(6, reader->ms_reader_cycle_total, true);
    //CT_LOG(6, F("tagLastReadTimeoutX1000() "), false); CT_LOG(6, reader->tagLastReadTimeoutX1000(), true);
    CT_LOG(6, F("* *"), true);
    

    if (
    // TIMEOUT (grace period)
    // If no tag read at startup and reader has recently power cycled.
    //
    // This should probably calculate or use global setting for
    // appropriate time-to-wait since last power cycle. (isn't this done?)

      reader->last_tag_read_ms == 0UL &&
      reader->last_reader_power_cycle_ms > 0UL &&
      reader->msSinceLastReaderPowerCycle() > 2000UL
      ){
      
      CT_LOG(6, F("Timeout:grace"), true);
      RGB[0]->slowBlink();
      RGB[1]->off();
      
      setProximityState(0);
      reader->reader_power_cycle_high_duration = 3UL;

      if (ctrl_status != 1) {
        LOG(3, F("TIMEOUT: startup"), true);
        Beeper->slowBeep(3);
      }

      ctrl_status = 1;
    
    } else if (
    // TIMEOUT (general).
    // If last read is beyond TIMEOUT, and we've cycled reader at least once in that interval.
      
      reader->msSinceLastTagRead() > reader->tagLastReadTimeoutX1000() &&
      reader->last_reader_power_cycle_ms > 0UL &&
      reader->msSinceLastTagRead() > reader->msSinceLastReaderPowerCycle() &&
      reader->msSinceLastReaderPowerCycle() > 2000UL // TODO: What is this 2000 figure?
      ){

      CT_LOG(6, F("Timeout:general"), true);
      RGB[0]->slowBlink();
      RGB[1]->off();
      
      setProximityState(0);
      reader->reader_power_cycle_high_duration = 3UL;

      if (ctrl_status != 2) {
        LOG(3, F("TIMEOUT: general"), true); // only prints once.
        Beeper->slowBeep(3);
      }

      ctrl_status = 2;

    } else if (
    // AGING phase of controller.
    // If last read is greater than reader-power-cycle-total AND
    // less than final timeout total, we're in the AGING zone.
    
      reader->last_tag_read_ms > 0UL &&
      // FIX: These need upating to use the newly named (and slightly repurposed)
      //      vars or functions that separate soft-timeout from reader-cycle-high.
      reader->msSinceLastTagRead() > reader->ms_reader_cycle_total &&
      reader->msSinceLastTagRead() <= reader->tagLastReadTimeoutX1000()
      ){

      CT_LOG(6, F("Aging"), true);
      if (proximity_state) {
        RGB[0]->off();
        RGB[1]->fastBlink();
      } else {
        RGB[0]->fastBlink();
        RGB[1]->off(); 
      }
      
      //RGB[2]->off();
      Beeper->fastBeep();
      
      setProximityState(1);
      reader->reader_power_cycle_high_duration = 3UL;

      if (ctrl_status != 3) LOG(3, F("AGING"), true); // only prints once.

      ctrl_status = 3;

    } else if (
    // FRESH
    // A valid and recognized tag has recently been read.
    
      reader->last_tag_read_ms > 0UL &&
      reader->msSinceLastTagRead() <= reader->ms_reader_cycle_total
      ){
        
      CT_LOG(6, F("Fresh"), true);
      RGB[0]->off();
      RGB[1]->steady();
      //RGB[2]->off();
      Beeper->off();
      
      setProximityState(1);
      reader->reader_power_cycle_high_duration = 0UL;

      if (ctrl_status != 4) LOG(5, F("FRESH"), true); // only prints once.

      ctrl_status = 4;

    
    } else {
    // PASS phase. Doesn't do anything.
    // This condition is NOT a problem state.
      
      CT_LOG(6, F("Pass"), true); // always prints

      // For debugging only.
      // Must enable both of these to log "PASS".
      //
      // NOTE: This will print too much, unless the following ctrl_status is set to 0.
      //if (ctrl_status != 0) LOG(5, F("PASS"), true); // only prints once.
      // NOTE: This will foul up the timeout and aging beepers, so only use it when debugging requires!
      //if (LogLevel() >= 5U) ctrl_status = 0; 
    }

    if (Menu::RunMode == 0) digitalWrite(OUTPUT_SWITCH_PIN, proximity_state);
    
  } // proximityStateController()


  int Controller::setProximityState(int _state) {
    if (Menu::RunMode == 0) {
      int previous_proximity_state = proximity_state;
      proximity_state = _state;

      if (proximity_state != previous_proximity_state && S.proximity_state_startup == 2) {
        LOG(4, F("Ctrl calling EEPROM.update w/state: "));
        LOG(4, proximity_state, true);
        EEPROM.update(STATE_EEPROM_ADDRESS, proximity_state);
      }
    }
    return proximity_state;
  }

  
