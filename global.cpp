  #include "global.h"

  // This is the magic line to have BTserial universally available.
  // Compiler will complain the first time it compiles, but it works fine.
  extern SoftwareSerial *BTserial = new SoftwareSerial(BT_RX_PIN, BT_TX_PIN);

  // Called from main .ino setup().
  extern void globalSetup() {
    // from main .ino
    pinMode(FAILSAFE_PIN, INPUT_PULLUP);
    pinMode(BT_STATUS_PIN, INPUT_PULLUP);
    pinMode(DEBUG_PIN, INPUT_PULLUP);

    // from controller
    pinMode(READER_POWER_CONTROL_PIN, OUTPUT);
    pinMode(OUTPUT_SWITCH_PIN, OUTPUT);
  }

  
