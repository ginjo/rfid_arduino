#include "settings.h"

  extern const Storage Settings = {

    // RFID class constants
    //  14,  // RDM63000
    //  10,    // 7941E
    
    20,    // TAG_LAST_READ_TIMEOUT       seconds
    1000,  // TAG_READ_INTERVAL           ms
    150,   // READER_CYCLE_LOW_DURATION   ms
    5000,  // READER_CYCLE_HIGH_DURATION  ms
    6,     // READER_POWER_CONTROL_PIN    ms

    // Led class constants
    //  10,    //

    // SerlialMenu class constants
    //  16,    //
    //  8,     //
    //  20,    //
    //  5      //
  };

  extern const Storage& S = Settings;

  extern const int TAG_LAST_READ_TIMEOUT = S.TAG_LAST_READ_TIMEOUT;
  extern const int TAG_READ_INTERVAL = S.TAG_READ_INTERVAL;
  extern const int READER_CYCLE_LOW_DURATION = S.READER_CYCLE_LOW_DURATION;
  extern const int READER_CYCLE_HIGH_DURATION = S.READER_CYCLE_HIGH_DURATION;
  extern const int READER_POWER_CONTROL_PIN = S.READER_POWER_CONTROL_PIN;

  
