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

  // See logger.h for master debug controls.
  #ifdef INO_DEBUG
    #define INO_PRINT(...) DPRINT(__VA_ARGS__)
    #define INO_PRINTLN(...) DPRINTLN(__VA_ARGS__)
  #else
    #define INO_PRINT(...)
    #define INO_PRINTLN(...)
  #endif


  /***  Declarations  ***/

  // Defines RGB LED -- an array of 3 Led instances.
  // Declaration is in led_blinker.h (extern and static var declaration/definitions are very confusing).
  Led *RGB[3] = {};

  // Declares beeper, which is also handled by Led class.
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
    
    delay(15);

    LOG(F("Booting RFID proximity sensor, "));
    LOG(VERSION);
    LOG(F(", "));
    LOG(TIMESTAMP, true);

    LOG(F("Debug pin status: "));
    LOG(TempDebug, true);
    
    INO_PRINTLN(F("Initialized default serial port @ 57600 baud"));

    FREERAM("setup() before Settings::Load()");
    
    Settings::Load(); // (*settings_instance, eeprom_address)

    //SoftwareSerial *BTserial = new SoftwareSerial(BT_RX_PIN, BT_TX_PIN);
    //BTserial->flush();
    //BTserial->end();
    BTserial->begin(S.bt_baud);
    
    Serial.flush(); // I think flushes only outbound data. See Serial class docs.
    Serial.begin(S.hw_serial_baud);
    while (! Serial) delay(10);
    
    delay(15);

    #ifdef INO_DEBUG
      LOG(F("Loaded Settings '"));
      LOG(S.settings_name);
      LOG(F("' with checksum '0x"));
      LOG(S.calculateChecksum(), 16);
      LOG(F("' of size "));
      LOG(sizeof(S), true);
    #endif
    
    LOG(F("Initialized serial port with loaded setting: "));
    LOG(S.hw_serial_baud, true);

    // Displays current settings and readers.
    LOG("", true);
    S.printSettings(&Serial);
    Serial.println("");
    //Reader::PrintReaders(&Serial);
    //Serial.println("");
    //
    if (canLogToBT()) {
      S.printSettings(BTserial);
      BTserial->println("");
      //Reader::PrintReaders(BTserial);
      //BTserial->println("");
    }

    FREERAM("setup() pre obj new");


    /*  Initialize main objects  */

    RGB[0] = new Led(LED_RED_PIN, "Rd");
    RGB[1] = new Led(LED_GREEN_PIN, "Gr");
    RGB[2] = new Led(LED_BLUE_PIN, "Bl");

    Beeper  = new Led(BEEPER_PIN, "au", S.tone_frequency);

    RfidSerial = new SoftwareSerial(RFID_RX_PIN, RFID_TX_PIN);

    RfidReader = Reader::GetReader((int)S.default_reader);
    RfidReader->serial_port = RfidSerial;

    // moved here from below
    OutputControl = new Controller(RfidReader, RGB, Beeper);
    
    Menu::HW = new Menu(&Serial, RfidReader, "HW");
    Menu::SW = new Menu(BTserial, RfidReader, "SW");

    //OutputControl = new Controller(RfidReader, RGB, Beeper);

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

    FREERAM("end setup()");

    // Add empty line before beginning loop.
    LOG("", true);

  } // setup()


  void loop() {

    RGB[0]->loop();
    RGB[1]->loop();
    RGB[2]->loop();
    Beeper->loop();

    if (Menu::run_mode > 0) {
      Menu::Loop();
    } else if (Menu::run_mode == 0) {
      //RfidReader->loop(); // Is this for debugging, or is it just old code?
      OutputControl->loop();      
    }

    //FREERAM(); // Only for memory debugging.
  } // end loop()

  
