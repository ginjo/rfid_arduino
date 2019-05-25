/* Blink without Delay

 Copied from the BlinkWithoutDelay starter,
 then modularized and somewhat "evented"
 for easier management of blink phases.
 
 */

  #include <SoftwareSerial.h>

  // Sets the pin number
  const int led_pin =  9; //LED_BUILTIN;// the number of the LED pin
  
  // Sets duration of led phases (milliseconds) - should always be even num of elements.
  const long interval[] = {50,20,50,20,50,1700};
  
  // Sets initial state of led (phase 0)
  int led_state = LOW;

  // Sets starting phase
  int phase = 0;

  // Tells blinker to start
  int start_blinker = LOW;
  
  // Generally, you should use "unsigned long" for variables that hold time
  // The value will quickly become too large for an int to store
  unsigned long previous_ms = 0; // will store last time LED was updated
  unsigned long previous_ms_saved = 0; // just for reporting

  // BT module
  SoftwareSerial BTserial(2, 3); // RX | TX
  char bt_state = '0';

  // Byte buffer for incoming BTserial strings
  uint8_t buff[8];
  int buff_index = 0;

  char tags[8][8] = {
    {'1','2','3','4','a','b','c','d'},
    {'z','x','c','v','9','8','7','6'},
    {'A','1','B','2','C','3','D','4'}
  };
  int tag_index = 3;


  void setup() {
    // set the digital pin as output:
    pinMode(led_pin, OUTPUT);
    Serial.begin(9600);
    BTserial.begin(9600);
    //startPhase(0);
    start_blinker = HIGH;
    Serial.println("Enter AT commands for HC-06:");
  }
  
  void loop() {
    // here is where you'd put code that needs to be running all the time.
    
    handleBTserial();
    handleBlinker();
  }
  
  // Starts a new blinker phase, given int
  void startPhase(int phz) {
    phase = phz;
    digitalWrite(led_pin, ledStateChange());
    previous_ms = millis();
  }

  // Toggles and returns led state
  int ledStateChange() {
    if (led_state == HIGH) {
      led_state = LOW;
    } else {
      led_state = HIGH;
    }
    return led_state;
  }

  // Handles start-stop blinker and blinker cycling
  void handleBlinker() {
    if (start_blinker == HIGH) {
      start_blinker = LOW;
      led_state = LOW;
      startPhase(0);
    }

    // Checks to see if it's time to toggle the led
    unsigned long current_ms = millis();
    if (current_ms - previous_ms >= interval[phase]) {
      // save the last time you started a new phase
      previous_ms_saved = previous_ms;
      previous_ms = current_ms;
      
      // Sends loggerging code to Serial output
      //logger(phase);
      //logger(interval[phase]);
      //logger(current_ms);
      //logger(previous_ms_saved);
      //logger(led_state[phase]);
      //logger(" ");

      // Increments the led phase, or resets it to zero,
      // then calls startPhase()
      int ary_size = sizeof(interval)/sizeof(*interval);
      if (phase >= ary_size - 1) {
        startPhase(0);
      } else {
        startPhase(phase + 1);
      }
    }
  }

  // Handle BTserial
  void handleBTserial() {
    // Keep reading from HC-06 and send to Arduino Serial Monitor
    if (BTserial.available()) {  
      uint8_t byt = BTserial.read();
      
      // For debugging
      //BTserial.println("");
      //BTserial.print("BYTE ");
      //BTserial.println(byt);
      //BTserial.print("BYTE_CHAR ");
      //BTserial.println(char(byt));
      //BTserial.print("STATE_CHAR ");
      //BTserial.println(bt_state);
      
      if (bt_state == '0') {
        // Draws or selects menu
        switch (char(byt)) {
          case '1':
            BTserial.println("Menu > List tags");
            listTags();
            BTserial.println("");
            bt_state = '0';
            break;
          case '2':
            BTserial.println("Menu > Add tag");
            BTserial.print("Enter a tag number to store: ");
            bt_state = '2';
            break;
          case '3':
            BTserial.println("Menu > Delete tag");
            BTserial.println("");
            bt_state = '0';
            break;
          default:
            BTserial.println("Menu");
            BTserial.println("1. List tags");
            BTserial.println("2. Add tag");
            BTserial.println("3. Delete tag");
            BTserial.println("");
            bt_state = '0';
            break;
        }
      } else if (bt_state == '2') {
        buff[buff_index] = byt;
        buff_index ++;
        BTserial.write(byt);
        
        if (int(byt) == 13 || buff_index > 7) {
          buff_index = 0;
          BTserial.println("");

          // Need to discard bogus tags... this kinda works
          if (sizeof(buff)/sizeof(*buff) != 8 || buff[0] == 13) {
            bt_state = '0';
            BTserial.println("");
            return;
          }
          
          //BTserial.print("Tag entered: ");
          //BTserial.println((char*)buff);
          //for (int i = 0; i < 8; i++) {
          //  BTserial.write(buff[i]);
          //}
          BTserial.println("");

          for (int i = 0; i < 8; i++) {
            tags[tag_index][i] = buff[i];
          }
          tag_index ++;
          
          bt_state = '0';
          listTags();
          BTserial.println("");
        }
      // Are either of these last two conditions used?
      } else if (int(byt) == 13) {
        // User hit Return
        BTserial.println("");
        bt_state = '0';
      } else {
        BTserial.write(byt);
      }
      
    } // done with available BTserial input
  }

  // Generic menu selection response
  //void youSelected(char item) {
  //  BTserial.print("\r\nYou selected menu item '");
  //  BTserial.print(item);
  //  BTserial.println("'");
  //}

  // Log to serial port(s)
  void logger(char str) {
    Serial.println(str);
    BTserial.println(str);    
  }

  void listTags() {
    //BTserial.println((char*)tags);
    for (int i = 0; i < 8; i++) {
      if (! char(tags[i][0])) {
        return;
      }
      BTserial.print(i);
      BTserial.print(". ");
      for (int j = 0; j < 8; j++) {
        BTserial.print(char(tags[i][j]));
      }
      BTserial.println("");
    }
  }
  
