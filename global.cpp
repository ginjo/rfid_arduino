  #include "global.h"


  bool TempDebug = false;

  // Defines & initializes BTserial. This works fine too.
  //SoftwareSerial *BTserial = new SoftwareSerial(BT_RX_PIN, BT_TX_PIN);
  
  // Just defines BTserial. See global.h for declaration and main .ino for initialization.
  SoftwareSerial *BTserial;

  // Defines RGB LED -- an array of 3 Led instances.
  Led *RGB[3];

  // Defines pointer to beeper, which is also handled by Led class.
  Led *Beeper;
  

  // Called from main .ino setup().
  void GlobalSetup() {
    // from main .ino
    pinMode(BT_STATUS_PIN, INPUT_PULLUP);
    pinMode(FAILSAFE_PIN, INPUT_PULLUP);
    pinMode(DEBUG_PIN, INPUT_PULLUP);

    // from controller
    pinMode(READER_POWER_CONTROL_PIN, OUTPUT);
    pinMode(OUTPUT_SWITCH_PIN, OUTPUT);

    TempDebug = (digitalRead(DEBUG_PIN) == LOW);

    //delay(100); // Allows voltages to stabilize... maybe?
  }

  
