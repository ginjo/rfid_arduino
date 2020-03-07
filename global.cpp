  #include "global.h"


  bool TempDebug = false;

  // Defines & initializes SWserial. This works fine too.
  //SoftwareSerial *SWserial = new SoftwareSerial(BT_RX_PIN, BT_TX_PIN);
  
  // Just defines SWserial. See global.h for declaration and main .ino for initialization.
  //SoftwareSerial *SWserial
  // Attempting initialization here to maybe avoid mystery UB.
  //SoftwareSerial *SWserial = nullptr;
  SerialPort *HWserial = nullptr;
  SerialPort *SWserial = nullptr;

  // Defines RGB LED -- an array of 3 Led instances.
  Led *RGB[3] = {};

  // Defines pointer to beeper, which is also handled by Led class.
  Led *Beeper = nullptr;
  

  // Called from main .ino setup().
  void GlobalSetup() {
    pinMode(BT_STATUS_PIN, INPUT_PULLUP);
    //pinMode(FAILSAFE_PIN, INPUT_PULLUP);
    pinMode(DEBUG_PIN, INPUT_PULLUP);
    pinMode(READER_POWER_CONTROL_PIN, OUTPUT);
    pinMode(OUTPUT_SWITCH_PIN, OUTPUT);
    
    //digitalWrite(OUTPUT_SWITCH_PIN, HIGH);

    TempDebug = (digitalRead(DEBUG_PIN) == LOW);
  }

  
