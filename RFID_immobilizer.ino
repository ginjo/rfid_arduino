/* 
 * Passive RFID-activated switch with bluetooth serial admin console.
 * For use in generic engine kill modules.
 * 
 */
 
  #include <Arduino.h>
  #include <SoftwareSerial.h>
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

  // Declares blinker LED.
  //Led *led_red, *led_green, *led_blue;

  // Declares RGB LED.
  Led *RGB[3];

  // Declares beeper.
  Led *Beeper;

  // Declares a software-serial port for admin console.
  SoftwareSerial *BTserial;

  // Declares serial port for Controller reader.
  SoftwareSerial *RfidSerial;//(91,90);

  // Declares rfid reader instance.
  Reader *RfidReader;
  
  // Declares instance of Controller handler.
  Controller *OutputControl;



  /*** Setup  ***/

  void setup() {
    pinMode(FAILSAFE_PIN, INPUT_PULLUP);
    
    // Opens default hardware serial port.
    // Requirement for Settings operations logging.
    Serial.begin(57600);
    while (! Serial) delay(10);
    delay(15);
    Serial.println(F("Initialized default serial port @ 57600 baud"));

    FreeRam("setup() pre load-setngs");
    
    Settings::Load(); // (*settings_instance, eeprom_address)


    /* For normal mode, when debugging not needed */
    
    Serial.flush(); // I think flushes only outbound data. See Serial class docs.
    Serial.begin(S.HW_SERIAL_BAUD);
    while (! Serial) delay(10);
    delay(15);
    Serial.print(F("Re-initialized serial port with loaded settings: "));
    Serial.println(S.HW_SERIAL_BAUD);

    Serial.print(F("Loaded Settings '"));
    Serial.print((char *)S.settings_name);
    //Serial.print(F("' with checksum 0x"));
    //Serial.print(S.calculateChecksum(), 16);
    Serial.print(F("' of size "));
    Serial.println(sizeof(S));

    Serial.print(F("Booting Controller Immobilizer, "));
    Serial.print(VERSION);
    Serial.print(F(", "));
    Serial.println(TIMESTAMP);


    /* For manual debug/log mode */
    
    pinMode(S.DEBUG_PIN, INPUT_PULLUP);

    int debug_pin_status = digitalRead(S.DEBUG_PIN);

    Serial.print(F("Debug pin status: "));
    Serial.println(debug_pin_status);

    if (debug_pin_status == LOW) {
      Serial.println(F("Debug pin LOW ... enabling debug"));
      S.enable_debug = 1;
    }

    // Displays current settings.
    Serial.println();
    S.printSettings(&Serial);
    Serial.println();

    FREERAM("setup() pre obj new");


    /*  Initialize main objects  */

    RGB[0]  = new Led(LED_RED_PIN, "Rd");
    RGB[1]  = new Led(LED_GREEN_PIN, "Gr");
    RGB[2]  = new Led(LED_BLUE_PIN, "Bl");

    Beeper  = new Led(BEEPER_PIN, "au");
    
    BTserial = new SoftwareSerial(S.BT_RXTX[0], S.BT_RXTX[1]); // RX | TX

    RfidSerial = new SoftwareSerial(S.RFID_SERIAL_RX, A6); // Assumes A6 is unused pin.

    RfidReader = Reader::GetReader(S.DEFAULT_READER);
    RfidReader->serial_port = RfidSerial;
    
    Menu::HW = new Menu(&Serial, RfidReader, "HW");
    Menu::SW = new Menu(BTserial, RfidReader, "SW");

    OutputControl = new Controller(RfidReader, RGB, Beeper);

    FREERAM("setup() pre obj stp");
    
    
    /*  Run setup/begin/init functions  */

    // Initializes output controller, including switch relay and LEDs.
    OutputControl->initializeOutput();

    // Activates the software-serial port for admin console.
    BTserial->begin(S.BT_BAUD);

    // Loads tags to default location (Tags::TagSet).
    Tags::Load();

    // Activates the admin console.
    Menu::Begin();

    // Activates the serial port for the Controller handler.
    RfidSerial->begin(S.RFID_BAUD);

    // Activates the Controller handler.
    OutputControl->begin();

    FreeRam("end setup()");

    // Add empty line before beginning loop.
    Serial.println();

  } // setup()


  void loop() {

    RGB[0]->loop();
    RGB[1]->loop();
    RGB[2]->loop();
    Beeper->loop();

    if (Menu::run_mode > 0) {
      Menu::Loop();
    } else if (Menu::run_mode == 0) {
      //RfidReader->loop();
      OutputControl->loop();      
    }
    
  } // end loop()

  
