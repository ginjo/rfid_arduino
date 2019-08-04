/* 
 * Passive Controller activated switch, user-controllable with
 * activation delay, for use in generic engine kill module.
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


  // Declares blinker LED.
  Led *Blinker;

  // Declares a software-serial port for admin console.
  SoftwareSerial *BTserial;

  // Declares serial port for Controller reader.
  SoftwareSerial *RfidSerial;//(91,90);

  // Declares rfid reader instance.
  Reader *RfidReader;
  
  // Declares instance of Controller handler.
  Controller *Rfid;


  void setup() {
    pinMode(FAILSAFE_PIN, INPUT_PULLUP);
    
    // For debugging, so Settings operations can be logged.
    Serial.begin(57600);
    while (! Serial) delay(10);
    delay(15);
    Serial.println(F("Initialized default serial port @ 57600 baud"));

    FreeRam("setup() pre load-setngs");
    
    Settings::Load(); // (*settings_instance, eeprom_address)

    // Normal, when debugging not needed.
    Serial.flush();
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

    // For manual debug/log mode.
    pinMode(S.DEBUG_PIN, INPUT_PULLUP);

    int debug_pin_status = digitalRead(S.DEBUG_PIN);

    Serial.print(F("Debug pin status: "));
    Serial.println(debug_pin_status);

    if (debug_pin_status == LOW) {
      Serial.println(F("Debug pin LOW ... enabling debug"));
      S.enable_debug = 1;
    }

    // TODO: Move this to Settings class, and call it from here.
    // Prints out all settings in tabular format.
    Serial.println();
    //for (int n=1; n <= SETTINGS_SIZE; n++) {
    //  char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
    //  S.displaySetting(n, output);
    //  Serial.println(output);
    //}
    S.printSettings(&Serial);
    Serial.println();

    FREERAM("setup() pre obj new");


    /*  Initialize main objects  */

    Blinker = new Led(S.LED_PIN);

    BTserial = new SoftwareSerial(S.BT_RXTX[0], S.BT_RXTX[1]); // RX | TX

    RfidSerial = new SoftwareSerial(S.RFID_SERIAL_RX, S.LED_PIN);

    RfidReader = Reader::GetReader(S.DEFAULT_READER);
    RfidReader->serial_port = RfidSerial;
    
    Menu::HW = new Menu(&Serial, RfidReader, Blinker, "HW");
    Menu::SW = new Menu(BTserial, RfidReader, Blinker, "SW");

    Rfid = new Controller(RfidReader, Blinker);

    FREERAM("setup() pre obj stp");
    
    
    /*  Run setups  */

    Rfid->initializeOutput();

    Blinker->StartupBlink();

    // Activates the software-serial port for admin console.
    BTserial->begin(S.BT_BAUD);

    // Loads tags to default location (Tags::TagSet).
    Tags::Load();

    // Activates the admin console.
    Menu::Begin();

    // Activates the serial port for the Controller handler.
    RfidSerial->begin(S.RFID_BAUD);

    // Activates the Controller handler.
    Rfid->begin();

    FreeRam("end setup()");

    // Add empty line before beginning loop.
    Serial.println();

  } // setup()


  void loop() {
    
    Blinker->loop();

    if (Menu::run_mode > 0) {
      Menu::Loop();
    } else if (Menu::run_mode == 0) {
      RfidReader->loop();
      Rfid->loop();      
    }
    
  } // end loop()

  
