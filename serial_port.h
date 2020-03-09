#ifndef __SERIAL_PORT__
#define __SERIAL_PORT__

  #include <Arduino.h>
  #include "logger.h"

  // See logger.h for master debug controls.
  #ifdef SL_DEBUG
    #define SL_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define SL_LOG(...)
  #endif


  class SerialPort : public Stream {
  public:

    bool is_bt;
    //virtual void begin(unsigned long);
    //virtual void begin(unsigned long, uint8_t);

    bool can_output();

  };

#endif
