#include <SoftwareSerial.h>

class Test
{
  private:
    Stream *_streamRef;
  
  public:
    setSerial(Stream *streamObject)
    {
      _streamRef=streamObject;
    }
    
    sendText(char *someText)
    {
      _streamRef->println(someText);
    }

    Test(Stream *_ref) : _streamRef(_ref) {}
};


SoftwareSerial softSerial(2, 3);

Test wrapper1(&Serial);
Test wrapper2(&softSerial);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  softSerial.begin(9600);
  
  //wrapper1.setSerial(&Serial); // tell the class which serial object to use
  //wrapper2.setSerial(&softSerial);
}

void loop() {
  // put your main code here, to run repeatedly:
  String txt1 = "HELLO, WORLD!!!"
                "\n"
                "Wassup?";
  char txt1char[64];
  txt1.toCharArray(txt1char, 64);
  Serial.println(txt1char);

  char txt3[] = "helloworld"
                "\n"
                "hiya\n";
  Serial.println(txt3);
                
  wrapper1.sendText("Hello, world!");
  //wrapper2.sendText("Hey hey hey kids!");
  delay(1000);
}
