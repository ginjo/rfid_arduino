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
  #include "menu.h"
  #include "controller.h"


  /***  Declarations  ***/

  // Defines RGB LED -- an array of 3 Led instances.
  // Declaration is in led_blinker.h (extern and static var declaration/definitions are very confusing).
  //Led *RGB[3] = {};
  //Led Led::RGB[] = {};
  //Led RGB[] = {Led(LED_RED_PIN, "Rd"), Led(LED_GREEN_PIN, "Gr"), Led(LED_BLUE_PIN, "Bl")};
  Led *RGB[3];

  // Declares beeper, which is also handled by Led class.
  //Led *Beeper;
  Led *Beeper;

  // Declares a software-serial port for admin console.
  // Was moved to global.h and global.cpp, since it needed to be an extern.
  //SoftwareSerial *BTserial;

  // Declares serial port for RFID reader.
  SoftwareSerial *RfidSerial;

  // Declares rfid reader instance.
  Reader *RfidReader;
  
  // Declares instance of Controller handler.
  Controller *OutputControl;


  /*** Setup  ***/

  void setup() {
    globalSetup();
    
    // Opens default hardware serial port.
    // Requirement for Settings operations logging.
    Serial.begin(57600);
    while (! Serial) delay(10);

    //BTserial->flush();
    BTserial->begin(S.bt_baud);
    
    delay(25);

    LOG(4, F("RFID proximity sensor pre-boot"), true);

    LOG(4, F("Initialized HW serial port @ 57600"), true);

    FREERAM("Main setup() before Settings::Load()");
    
    Settings::Load();

    BTserial->begin(S.bt_baud);
    
    Serial.flush(); // I think flushes only outbound data. See Serial class docs.
    Serial.begin(S.hw_serial_baud);
    while (! Serial) delay(10);
    
    delay(25);

    LOG(4, F("Booting RFID proximity sensor, "));
    LOG(4, VERSION);
    LOG(4, F(", "));
    LOG(4, TIMESTAMP, true);

    #ifdef INO_DEBUG
      LOG(5, F("Loaded Settings '"));
      LOG(5, S.settings_name);
      LOG(5, F("' with checksum '0x"));
      LOG(5, S.calculateChecksum(), 16);
      LOG(5, F("' of size "));
      LOG(5, sizeof(S), true);
    #endif

    LOG(4, F("Initialized HW serial port @ "));
    LOG(4, S.hw_serial_baud, true);

    LOG(4, F("Debug pin status: "));
    LOG(4, TempDebug, true);

    LOG(4, F("LogLevel(): "));
    LOG(4, LogLevel(), true);
    


    // Displays current settings and readers.
    if (LogLevel() >=4) {
      LOG(4, "", true);
      S.printSettings(&Serial);
      Serial.println("");
      Reader::PrintReaders(&Serial);
      Serial.println("");
      //
      if (canLogToBT()) {
        S.printSettings(BTserial);
        BTserial->println("");
        Reader::PrintReaders(BTserial);
        BTserial->println("");
      }
    }

    FREERAM("setup() pre obj new");


    /*  Initialize main objects  */

    RGB[0] = new Led(LED_RED_PIN, "Rd");
    RGB[1] = new Led(LED_GREEN_PIN, "Gr");
    RGB[2] = new Led(LED_BLUE_PIN, "Bl");
    
    Beeper = new Led(BEEPER_PIN, "au", S.tone_frequency);

    if (LogLevel() >= 5) Led::PrintStaticIntervals();

    RfidSerial = new SoftwareSerial(RFID_RX_PIN, RFID_TX_PIN);

    RfidReader = Reader::GetReader((int)S.default_reader);
    RfidReader->serial_port = RfidSerial;

    //OutputControl = new Controller(RfidReader, RGB, Beeper);
    OutputControl = new Controller(RfidReader);
    
    Menu::HW = new Menu(&Serial, RfidReader, "HW");
    Menu::SW = new Menu(BTserial, RfidReader, "SW");

    FREERAM("setup() pre obj stp");
    
    
    /*  Run setup/begin/init functions  */

    // Activates the Controller handler.
    OutputControl->begin();

    // Initializes output controller, including switch relay and LEDs.
    // Disabled, experimenting with original call in Controller::begin().
    //OutputControl->initializeOutput();

    // Activates the serial port for the Controller handler.
    RfidSerial->begin(S.rfid_baud);

    // Loads tags to default location (Tags::TagSet).
    Tags::Load();

    // Activates the admin console.
    Menu::Begin();

    //// Activates the serial port for the Controller handler.
    //RfidSerial->begin(S.rfid_baud);
    //
    //// Activates the Controller handler.
    //OutputControl->begin();

    FREERAM("setup() end");

    // Add empty line before beginning loop.
    LOG(4, "", true);

  } // setup()


  void loop() {

    RGB[0]->loop();
    RGB[1]->loop();
    RGB[2]->loop();
    Beeper->loop();

    if (Menu::run_mode > 0) {
      Menu::Loop();
    } else if (Menu::run_mode == 0) {
      OutputControl->loop();      
    }

    //FREERAM(); // Only for memory debugging.
  } // end loop()

  
