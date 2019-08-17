  #include "controller.h"
  #include <EEPROM.h>

  // Constructors
  // Receives a Reader and an array of Led objecs (*RGB[] from .ino file).
  Controller::Controller(Reader *_reader, Led *_blinker[]) :
    proximity_state(0),
    reader(_reader),
    blinker(_blinker)
  { ; }


  // TODO: Some (all?) of this needs to be moved to Reader setup().
  void Controller::begin() {
    // Make sure to run this somewhere, if not here.
    //initializeOutput();
    
    pinMode(S.READER_POWER_CONTROL_PIN, OUTPUT);

    // Sets local 'reader' to instance of Reader.
    //reader = GetReader(S.DEFAULT_READER); // Moving to main .ino, to be passed into Controller::Controller().

    Serial.print(F("Starting Controller with reader "));
    Serial.print(reader->reader_name);
    Serial.print(F(", with EEPROM proximity state "));
    Serial.println(proximity_state);
    //  Serial.print(F(", and output switch pin: "));
    //  Serial.println(S.OUTPUT_SWITCH_PIN);
        
    // Initializes the reader power/reset control.
    //digitalWrite(S.READER_POWER_CONTROL_PIN, S.READER_POWER_CONTROL_POLARITY ? HIGH : LOW);
    // This first one is to clear any built-up charge, as something is holding the reader low at startup.
    digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? LOW : HIGH);
    delay(50);
    digitalWrite(S.READER_POWER_CONTROL_PIN, reader->power_control_logic ? HIGH : LOW);
    
  } // setup()


  void Controller::loop() {
    reader->loop();
    proximityStateController();
  }


  // Initializes output switch.
  void Controller::initializeOutput() {
    pinMode(S.OUTPUT_SWITCH_PIN, OUTPUT);    
    
    // Starts up with whatever state we left off in.
    // Protects against thief using 'admin' to move
    // in fits and starts, since a failed proximity
    // timeout will set S.proximity_state to 0.
    // This also tells the proximityStateController
    // where we left off at power-down (or reset).
    proximity_state = S.proximity_state;

    Serial.print(F("Setting output switch per saved proximity_state: "));
    Serial.println(proximity_state);
    // Switches the main load according to current proximity_state.
    // This turns on the load if saved prox-state was "on".
    // This begins the courtesey grace period until the system can
    // start processing tags (at which time, it will immediately
    // shut down output until a successful tag read).
    digitalWrite(S.OUTPUT_SWITCH_PIN, proximity_state);
    if (proximity_state) {
      blinker[1]->StartupBlink();
    } else {
      blinker[0]->StartupBlink();
    }
  }


  // The proximity_state var determines physical switch state
  // while this function is actively looping.
  // 
  void Controller::proximityStateController() {
    CT_PRINTLN(F("*** PROXIMITY ***"));
    CT_PRINT(F("last_tag_read_ms: ")); CT_PRINTLN(reader->last_tag_read_ms);
    CT_PRINT(F("last_reader_power_cycle_ms: ")); CT_PRINTLN(reader->last_reader_power_cycle_ms);
    CT_PRINT(F("msSinceLastTagRead(): ")); CT_PRINTLN(reader->msSinceLastTagRead());
    CT_PRINT(F("msSinceLastReaderPowerCycle(): ")); CT_PRINTLN(reader->msSinceLastReaderPowerCycle());
    CT_PRINT(F("ms_reader_cycle_total: ")); CT_PRINTLN(reader->ms_reader_cycle_total);
    CT_PRINT(F("tagLastReadTimeoutX1000(): ")); CT_PRINTLN(reader->tagLastReadTimeoutX1000());
    CT_PRINTLN(F("***"));
    
    // If NO TAG READ YET and reader has recently power cycled
    // This should probably calculate or use global setting for appropriate time-to-wait since last power cycle.
    if (
      reader->last_tag_read_ms == 0UL &&
      reader->last_reader_power_cycle_ms > 0UL &&
      reader->msSinceLastReaderPowerCycle() > 2000UL
      ){
      
      CT_PRINTLN(F("proximityStateController() startup GRACE period timeout, no tag found"));
      blinker[0]->SlowBlink();
      blinker[1]->Off();
      setProximityState(0);
    
    // If last read is beyond TIMEOUT, and we've cycled reader at least once in that interval.
    } else if (
      reader->msSinceLastTagRead() > reader->tagLastReadTimeoutX1000() &&
      reader->last_reader_power_cycle_ms > 0UL &&
      reader->msSinceLastTagRead() > reader->msSinceLastReaderPowerCycle() &&
      reader->msSinceLastReaderPowerCycle() > 2000UL
      ){
      
      CT_PRINTLN(F("proximityStateController() TIMEOUT"));
      blinker[0]->SlowBlink();
      blinker[1]->Off();
      setProximityState(0);

    // If last read is greater than reader-power-cycle-total AND
    // less than final timeout total, we're in the AGING zone.
    } else if (
      reader->last_tag_read_ms > 0UL &&
      reader->msSinceLastTagRead() > reader->ms_reader_cycle_total &&
      reader->msSinceLastTagRead() <= reader->tagLastReadTimeoutX1000()
      ){

      //CT_PRINTLN(F("### AGING ###"));
      //CT_PRINT(F("last_tag_read_ms: ")); CT_PRINTLN(last_tag_read_ms);
      //CT_PRINT(F("msSinceLastTagRead(): ")); CT_PRINTLN(msSinceLastTagRead());
      //CT_PRINT(F("ms_reader_cycle_total: ")); CT_PRINTLN(ms_reader_cycle_total);
      //CT_PRINT(F("tagLastReadTimeoutX1000(): ")); CT_PRINTLN(tagLastReadTimeoutX1000());
      //CT_PRINTLN(F("###  ###"));

      CT_PRINTLN(F("proximityStateController() AGING"));
      if (proximity_state) {
        blinker[1]->FastBlink();
        blinker[0]->Off();
      } else {
        blinker[0]->FastBlink();
        blinker[1]->Off();        
      }
      
      setProximityState(1);

    // If we're STILL YOUNG.
    } else if (
      reader->last_tag_read_ms > 0UL &&
      reader->msSinceLastTagRead() <= reader->ms_reader_cycle_total
      ){
        
      CT_PRINTLN(F("proximityStateController() still YOUNG"));
      blinker[1]->Steady();
      blinker[0]->Off();
      setProximityState(1);

    // No expected condition was met (not sure what to do here yet).
    } else {
      CT_PRINTLN(F("proximityStateController() no condition was met (not necessarily a problem)"));
    }

    // TODO: Is there a better place for this? UPDATE: I don't think so.
    if (Menu::run_mode == 0) digitalWrite(S.OUTPUT_SWITCH_PIN, proximity_state);
    
  } // proximityStateController()

  void Controller::setProximityState(int _state) {
    if (Menu::run_mode == 0) {
      proximity_state = _state;
      
      if (proximity_state == 0) {
        reader->reader_power_cycle_high_duration = 3UL;
      } else {
        reader->reader_power_cycle_high_duration = 0UL;
      }
      
      S.updateProximityState(_state);
    }
  }


  /*  Static Vars & Functions  */

  // This is for aliasing the original Tags list to the new Tags list.
  // See beginning of .h file for typedef.
  // This is not currently used, but leaving here as example
  // of c++ variable aliasing (referencing).
  //TagArray& Controller::Tags = Tags::TagSet.tag_array;

  
