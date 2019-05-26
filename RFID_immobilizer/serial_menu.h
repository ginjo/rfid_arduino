// Serial Menu Class

#include <SoftwareSerial.h>

class SerialMenu {
public:

	SoftwareSerial BTserial;
	unsigned long baud_rate;
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
	
	SerialMenu(int rx, int tx, unsigned long baud) :
		BTserial(rx, tx),
		baud_rate(baud)
	{
		// Don't call .begin here, since this is too close to hardware init.
		// For example, "BTserial()" above triggers actual hardware init.
		// So the hardware might not be initialized yet, at this point.
    // Call .begin from setup() function instead.
	}

	void setup() {
    Serial.print("SerialMenu in setup() function, with baud rate: ");
    Serial.println(baud_rate);
    
		BTserial.begin(baud_rate);
	}
	
	void setup(unsigned long baud) {
		BTserial.begin(baud);
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
	        //for (int i = 0; i < 8; i ++) {
	        //  BTserial.write(buff[i]);
	        //}
	        BTserial.println("");

	        for (int i = 0; i < 8; i ++) {
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

	void listTags() {
	  //BTserial.println((char*)tags);
	  for (int i = 0; i < 8; i ++) {
	    if (! char(tags[i][0])) {
	      return;
	    }
	    BTserial.print(i);
	    BTserial.print(". ");
	    for (int j = 0; j < 8; j ++) {
	      BTserial.print(char(tags[i][j]));
	    }
	    BTserial.println("");
	  }
	}
	
	void loop() {
		handleBTserial();
	}

}; // end of serial menu class
