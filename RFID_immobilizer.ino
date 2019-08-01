/* 
 * Passive RFID activated switch, user-controllable with
 * activation delay, for use in generic engine kill module.
 * 
 */
 
 
  #include <Arduino.h>
  #include <SoftwareSerial.h>
  #include "settings.h"
  #include "led_blinker.h"
  #include "serial_menu.h"
  #include "rfid.h"

  // Comment this line to disable debug code for this class.
  //#define INO_DEBUG
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

  // Declares serial port for RFID reader.
  SoftwareSerial *RfidSerial;//(91,90);

  // Declares rfid reader instance.
  Reader *RfidReader;
  
  // Declares instance of RFID handler.
  RFID *Rfid;


  void setup() {
    
    // For debugging, so Settings operations can be logged.
    Serial.begin(57600);
    while (! Serial) delay(10);
    delay(15);
    Serial.println(F("Initialized default serial port @ 57600 baud"));

    FreeRam("setup() pre load-setngs");
    
    //Settings::Load(SETTINGS_EEPROM_ADDRESS);
    //Settings::Load(&Settings::Current);
    Settings::Load();

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

    Serial.print(F("Booting RFID Immobilizer, "));
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

    // Prints out all settings in tabular format.
    Serial.println();
    for (int n=1; n <= SETTINGS_SIZE; n++) {
      char output[SETTINGS_NAME_SIZE + SETTINGS_VALUE_SIZE] = {};
      S.displaySetting(n, output);
      Serial.println(output);
    }
    Serial.println();

    FREERAM("setup() pre obj new");

    /*  Initialize main objects  */

    Blinker = new Led(S.LED_PIN);

    BTserial = new SoftwareSerial(S.BT_RXTX[0], S.BT_RXTX[1]); // RX | TX

    RfidSerial = new SoftwareSerial(S.RFID_SERIAL_RX, S.LED_PIN);

    RfidReader = Reader::GetReader(S.DEFAULT_READER);
    RfidReader->serial_port = RfidSerial;
    
    SerialMenu::HW = new SerialMenu(&Serial, RfidReader, Blinker, "HW");
    SerialMenu::SW = new SerialMenu(BTserial, RfidReader, Blinker, "SW");

    Rfid = new RFID(RfidReader, Blinker);

    FREERAM("setup() pre obj stp");
    
    
    /*  Run setups  */

    Rfid->initializeOutput();

    Blinker->StartupBlink();

    // Activates the software-serial port for admin console.
    BTserial->begin(S.BT_BAUD);

    // Loads tags to default location (Tags::TagSet).
    Tags::Load();

    // Activates the admin console.
    SerialMenu::Begin();

    // Activates the serial port for the RFID handler.
    RfidSerial->begin(S.RFID_BAUD);

    // Activates the RFID handler.
    Rfid->begin();

    FreeRam("end setup()");
    

  } // setup()


  void loop() {
    
    Blinker->loop();

    if (SerialMenu::run_mode > 0) {
      SerialMenu::Loop();
    } else if (SerialMenu::run_mode == 0) {
      RfidReader->loop();
      Rfid->loop();      
    }
    
  } // end loop()

  
