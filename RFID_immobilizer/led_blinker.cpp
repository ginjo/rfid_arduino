// LED Blinker Class

class Led {
public:
  // Sets the pin number
  int led_pin; // = 9; //LED_BUILTIN;// the number of the LED pin
  
  // Sets duration of led phases (milliseconds).
  // Should always be even num of elements.
  int intervals[];
	int intervals_length;
  
  // Sets initial state of led (phase 0)
  int led_state = LOW;

  // Sets starting phase
  int phase = 0;

  // Tells blinker to start if HIGH
  int start_blinker = LOW;
  
  // Generally, you should use "unsigned long" for variables that hold time.
  // The value will quickly become too large for an int to store.
  unsigned long previous_ms = 0; // will store last time LED was updated
  unsigned long previous_ms_saved = 0; // just for reporting


  
  Led(int byt, int len, int inv[]) : 
    led_pin(byt),
		intervals_length(len)
		//intervals(inv) // this doesn't seem to work here,
		// so use the itterative pattern below:
	{
		//Serial.println("Beginning LED constructor\r\n");
    //for (int n = 0; n < len; n ++) {
    //  intervals[n] = inv[n];
    //	// Serial.print("n: ");
    //	// Serial.println(n);
    //	// Serial.print("inv[n]: ");
    //	// Serial.println(inv[n]);
    //	// Serial.println("\r\n");
    //}

		// Serial.println("Finished LED contructor");
  }


	void setup(int inv[]) {
		pinMode(led_pin, OUTPUT);
		start_blinker = HIGH;
    intervals_length = 4;
    
    for (int n = 0; n < 4; n ++) {
      intervals[n] = inv[n];
      Serial.print("n: ");
      Serial.println(n);
      Serial.print("inv[n]: ");
      Serial.println(inv[n]);
      Serial.println("\r\n");
    }

    Serial.print("intervals: ");
    Serial.print(intervals_length);
    Serial.print(",");
    for (int n = 0; n < intervals_length; n ++) {
      Serial.print(" ");
      Serial.print(intervals[n]);
    }
    Serial.println("");
	}
	

  void loop() {
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

    //Serial.println(phase);
    //Serial.println(intervals[phase]);
    //Serial.println(led_state);
    //Serial.println(current_ms);
    //Serial.println(previous_ms);
    //Serial.println(previous_ms_saved);
    //Serial.println(" ");

    if (current_ms - previous_ms >= intervals[phase]) {
      // save the last time you started a new phase
      previous_ms_saved = previous_ms;
      previous_ms = current_ms;
      
      Serial.println(phase);
      Serial.println(intervals[phase]);
      Serial.println(current_ms);
      Serial.println(previous_ms_saved);
      Serial.println(led_state);
      Serial.println(" ");

      // Increments the led phase, or resets it to zero,
      // then calls startPhase()
      //int ary_size = sizeof(intervals)/sizeof(*intervals);
      if (phase >= intervals_length - 1) {
        startPhase(0);
      } else {
        startPhase(phase + 1);
      }
    }
  }

};
