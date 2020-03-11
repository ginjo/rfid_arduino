/* 
 * Passive RFID-activated switch with bluetooth serial admin console.
 * For use in any switching application where RFID tag proximity is required for operation.
 * 
 */

  //#include "Stream.h"
  #include <Arduino.h>
  #include <SoftwareSerial.h>

  #include "global.h"
  #include "logger.h"
  #include "settings.h"
  #include "led_blinker.h"
  #include "reader.h"
  #include "menu.h"
  #include "controller.h"
  #include "serial_port.h"



  /***  Global Definitions (some declared as extern in global.h)
        and Local Declarations and Definitions  ***/

  bool TempDebug = false;

  Led *Led::RGB[3] = {
    new Led(LED_RED_PIN, "Rd"),
    new Led(LED_GREEN_PIN, "Gr"),
    new Led(LED_BLUE_PIN, "Bl")
  };

  //Led *Led::Beeper = new Led(BEEPER_PIN, "au", S.tone_frequency);

  Led *Led::Beeper;



  // Defines standard and custom serial port objects and adds them to SerialPort::List[].
  //
  HardwareSerial *hw_serial = &Serial;
  //SoftwareSerial *sw_serial = new SoftwareSerial(SW_RX_PIN, SW_TX_PIN);
  //
  SerialPort *HWserial = SerialPort::Add(hw_serial, true);
  //SerialPort *SWserial = SerialPort::Add(sw_serial);
  SerialPort *SWserial = nullptr;


  // Declares local serial port for RFID reader.
  SoftwareSerial *RfidSerial;

  // Declares local rfid reader instance.
  Reader *RfidReader;
  
  // Declares local instance of Controller handler.
  Controller *OutputControl;



  /***  Setup   ***/

  void setup() {
    //hw_serial->println(F("global setup bgn"));
    
    GlobalSetup();

    //hw_serial->println(F("global setup fin"));


    //hw_serial->println(F("hw_serial bgn"));
    
    hw_serial->begin(S.hw_baud);
    while (! hw_serial) delay(1);
    
    //hw_serial->println(F("hw_serial fin"));
    
    // sw_serial->begin(S.sw_baud);
    delay(25);


    #ifdef INO_DEBUG
      LOG(4, F("RFID proximity sensor pre-boot"), true);
      LOG(4, F("HW, SW serial @ "), false);
      LOG(4, S.hw_baud, false);
      LOG(4, F(", "), false);
      LOG(4, S.sw_baud, true);
      LOG(5, F("FreeRam b4 settings-load "));
      LOG(5, FreeRam(), true);
    #endif

    
    Settings::Load();

    // sw_serial->begin(S.sw_baud);
    hw_serial->flush(); // I think flushes only outbound data. See Serial class docs.
    hw_serial->begin(S.hw_baud);
    while (! hw_serial) delay(10);
    delay(25);

    //delete Led::Beeper;
    Led::Beeper = new Led(BEEPER_PIN, "au", S.tone_frequency);

    #ifdef INO_DEBUG
      LOG(4, F("Booting RFID proximity control, "));
      LOG(4, VERSION);
      LOG(4, F(", c++ "));
      LOG(4, __cplusplus);
      LOG(4, F(", "));
      LOG(4, TIMESTAMP, true);
  
      LOG(5, F("Loaded '"));
      LOG(5, S.settings_name);
      LOG(5, F("' with checksum '0x"));
      LOG(5, S.calculateChecksum(), 16);
      LOG(5, F("' of size "));
      LOG(5, sizeof(S), true);
  
      LOG(4, F("HW, SW serial @ "), false);
      LOG(4, S.hw_baud, false);
      LOG(4, F(", "), false);
      LOG(4, S.sw_baud, true);
  
      LOG(4, F("S.debugMode() "));
      LOG(4, S.debugMode(), true);
  
      LOG(4, F("LogLevel() "));
      LOG(4, LogLevel(), true);
  
  
      // Displays current settings and readers.
      // TODO: Maybe make this part of Logger or SerialPort.
      if (LogLevel() >= 4U) {
        for (uint8_t n = 0; n < SerialPort::Count; n++) {
          SerialPort *sp = SerialPort::List[n];
          if (sp->can_output()) {
            LOG(4, "", true);
            S.printSettings(sp);
            sp->println("");
            Reader::PrintReaders(sp);
            sp->println("");
          }
        }
      }

      LOG(5, F("FreeRam at setup() b4 new objcts "));
      LOG(5, FreeRam(), true);
    #endif


    #ifdef BK_DEBUG
      if (LogLevel() >= 5U ) Led::PrintStaticIntervals();
    #endif


    RfidReader = Reader::GetReader((int)S.default_reader);
    RfidSerial = new SoftwareSerial(RFID_RX_PIN, RFID_TX_PIN);
    RfidReader->serial_port = RfidSerial;

    OutputControl = new Controller(RfidReader);

    // TODO: Refactor this, when you get Menu::List[] implemented.
    Menu::M1 = new Menu(HWserial, RfidReader, "HW");
    //Menu::M2 = new Menu(SWserial, RfidReader, "SW");

    LOG(5, F("FreeRam at setup() pre obj stp "));
    LOG(5, FreeRam(), true);
    
    
    /*  Run setup/begin/init functions  */

    // Activates the Controller instance.
    OutputControl->begin();

    // Activates the serial port for the RFID reader.
    RfidSerial->begin(S.rfid_baud);

    // Loads tags to default location (Tags::TagSet).
    Tags::Load();

    // Activates the admin console.
    Menu::Begin();

   #ifdef INO_DEBUG
    LOG(5, F("FreeRam at setup() end "));
    LOG(5, FreeRam(), true);

    // Add empty line before beginning loop.
    LOG(4, "", true);
  #endif

  } // setup()



  void loop() {

    Led::RGB[0]->loop();
    Led::RGB[1]->loop();
    Led::RGB[2]->loop();
    Led::Beeper->loop();

    if (Menu::RunMode > 0) {
      Menu::Loop();
    } else if (Menu::RunMode == 0) {
      OutputControl->loop();      
    }

    //FREERAM(); // Only for memory debugging.
  } // end loop()



  void GlobalSetup() {
    pinMode(BT_STATUS_PIN, INPUT_PULLUP);
    //pinMode(FAILSAFE_PIN, INPUT_PULLUP);
    pinMode(DEBUG_PIN, INPUT_PULLUP);
    pinMode(READER_POWER_CONTROL_PIN, OUTPUT);
    pinMode(OUTPUT_SWITCH_PIN, OUTPUT);
    
    //digitalWrite(OUTPUT_SWITCH_PIN, HIGH);

    TempDebug = (digitalRead(DEBUG_PIN) == LOW);
  }

  
