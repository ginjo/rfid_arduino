#ifndef __SERIAL_PORT__
#define __SERIAL_PORT__

  #include <Arduino.h>
  #include <SoftwareSerial.h>

  // See logger.h for master debug controls.
  #ifdef SL_DEBUG
    #define SL_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define SL_LOG(...)
  #endif

  #define SERIAL_PORT_LIST_SIZE 1
  

  extern bool CanLogToBT(); // forward declaration, see serial_port.cpp.

  class SerialPort : public Stream {
  public:


    /*  Static  */

    static SerialPort *List[SERIAL_PORT_LIST_SIZE]; // Is the size even recognized here?
    static uint8_t Count;

    static SerialPort* Add(HardwareSerial*, bool = false);
    static SerialPort* Add(SoftwareSerial*, bool = false);
    

    /*  Constructor  */
    
    SerialPort();


    /*  Instance vars & functions  */
    
    bool is_bt;
    bool is_sw_serial;

    bool can_output();

    bool isListening();
    bool listen();

  };

#endif
