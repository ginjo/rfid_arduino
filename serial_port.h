#ifndef __SERIAL_PORT__
#define __SERIAL_PORT__

  #include <Arduino.h>

  // See logger.h for master debug controls.
  #ifdef SL_DEBUG
    #define SL_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define SL_LOG(...)
  #endif

  extern bool CanLogToBT(); // forward declaration, see serial_port.cpp.

  class SerialPort : public Stream {
  public:

    // Constructor
    SerialPort();

    // Instance vars & functions
    bool is_bt;
    bool is_sw_serial;

    bool can_output();

    bool isListening();
    bool listen();

  };

#endif
