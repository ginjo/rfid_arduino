/******************************************************************************

Custom Stream, HardwareSerial, and SoftwareSerial classes.

This will allow polymorphic custom-stream pointer to access methods
in both serial classes that are not declared in the Stream class.

*******************************************************************************/
#ifndef __SERIAL_CUSTOM_H__
#define __SERIAL_CUSTOM_H__

  #include <Arduino.h>
  #include <SoftwareSerial.h>
  #include <stream.h>
  
  
  class StreamCustom : public virtual Stream, public virtual Print {
  public:
      //virtual void print(char*);
      virtual void begin(unsigned long *);
      
      virtual void listen() {;}
      virtual bool isListening() { return true; }
      virtual bool available() {;}
      
  };
  
  class SoftSerial : public SoftwareSerial, public virtual StreamCustom {
  public:
      //void print(char *data, uint8_t format = 10) { va_list args; SoftwareSerial::print(args, data); }
      void listen() { SoftwareSerial::listen(); }
      bool isListening() { return SoftwareSerial::isListening(); }
  };
  
  class HardSerial : public HardwareSerial, public virtual StreamCustom {
  public:
      void print(char* str) { HardwareSerial::print(str); }
  };
  
  
  //  int main()
  //  {
  //      StreamCustom *soft = new SoftSerial;
  //      StreamCustom *hard = new HardSerial;
  //      
  //      soft->print("soft");
  //      soft->listen();
  //      soft->softly();
  //      soft->hardly();
  //      
  //      hard->print("hard");
  //      hard->listen();
  //      hard->softly();
  //      hard->hardly();
  //  
  //      return 0;
  //  };

#endif
