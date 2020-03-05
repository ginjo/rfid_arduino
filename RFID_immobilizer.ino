  /* 
 * Passive RFID-activated switch with bluetooth serial admin console.
 * For use in any switching application where RFID tag proximity is required for operation.
 * 
 */
 
  #include <Arduino.h>
  #include <SoftwareSerial.h>

  #include "global.h"
  #include "logger.h"
  #include "settings.h"
  #include "led_blinker.h"
  #include "reader.h"
  #include "menu.h"
  #include "controller.h"


  /***  Local Declarations  ***/

  // See global files for extern declarations & definitions used in this file.

  // Declares local serial port for RFID reader.
  SoftwareSerial *RfidSerial;

  // Declares local rfid reader instance.
  Reader *RfidReader;
  
  // Declares local instance of Controller handler.
  Controller *OutputControl;


  /*** Setup  ***/

  void setup() {
    GlobalSetup();

    // Initialize (static) SWserial first cuz we need it for logging.
    // See global files for declaration/definition of SWserial.
    SWserial = new SoftwareSerial(BT_RX_PIN, BT_TX_PIN);
    
    // Opens default hardware serial port.
    // Requirement for Settings operations logging.
    Serial.begin(S.hw_baud);
    while (! Serial) delay(10);
    SWserial->begin(S.sw_baud);
    delay(25);

    #ifdef INO_DEBUG
      LOG(4, F("RFID proximity sensor pre-boot"), true);
      LOG(4, F("HW, SW serial @ "), false);
      LOG(4, S.hw_baud, false);
      LOG(4, F(", "), false);
      LOG(4, S.sw_baud, true);
      FREERAM("Before Settings::Load()");
    #endif
    
    Settings::Load();

    SWserial->begin(S.sw_baud);
    Serial.flush(); // I think flushes only outbound data. See Serial class docs.
    Serial.begin(S.hw_baud);
    while (! Serial) delay(10);
    delay(25);


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
  
      LOG(4, F("Debug mode: "));
      LOG(4, S.debugMode(), true);
  
      LOG(4, F("LogLevel(): "));
      LOG(4, LogLevel(), true);
  
  
      // Displays current settings and readers.
      if (LogLevel() >= 4U) {
        LOG(4, "", true);
        S.printSettings(&Serial);
        Serial.println("");
        Reader::PrintReaders(&Serial);
        Serial.println("");
        //
        if (CanLogToBT()) {
          S.printSettings(SWserial);
          SWserial->println("");
          Reader::PrintReaders(SWserial);
          SWserial->println("");
        }
      }
  
      FREERAM("setup() pre new objcts");
    #endif


    /*  Initialize main objects. See global files for declarations/definitions.  */

    #ifdef BK_DEBUG
      if (LogLevel() >= 5U ) Led::PrintStaticIntervals();
    #endif

    RGB[0] = new Led(LED_RED_PIN, "Rd");
    RGB[1] = new Led(LED_GREEN_PIN, "Gr");
    RGB[2] = new Led(LED_BLUE_PIN, "Bl");
    
    Beeper = new Led(BEEPER_PIN, "au", S.tone_frequency);

    RfidSerial = new SoftwareSerial(RFID_RX_PIN, RFID_TX_PIN);

    RfidReader = Reader::GetReader((int)S.default_reader);
    RfidReader->serial_port = RfidSerial;

    OutputControl = new Controller(RfidReader);
    
    Menu::M1 = new Menu(&Serial, RfidReader, "HW");
    Menu::M2 = new Menu(SWserial, RfidReader, "SW");

    FREERAM("setup() pre obj stp");
    
    
    /*  Run setup/begin/init functions  */

    // Activates the Controller instance.
    OutputControl->begin();

    // Activates the serial port for the RFID reader.
    RfidSerial->begin(S.rfid_baud);

    // Loads tags to default location (Tags::TagSet).
    Tags::Load();

    // Activates the admin console.
    Menu::Begin();

   #ifdef BK_DEBUG
    FREERAM("setup() end");

    // Add empty line before beginning loop.
    LOG(4, "", true);
  #endif

  } // setup()


  void loop() {

    RGB[0]->loop();
    RGB[1]->loop();
    RGB[2]->loop();
    Beeper->loop();

    if (Menu::RunMode > 0) {
      Menu::Loop();
    } else if (Menu::RunMode == 0) {
      OutputControl->loop();      
    }

    //FREERAM(); // Only for memory debugging.
  } // end loop()

  
