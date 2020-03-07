/*
 * This is a trick to wrap (customized) an existing header, but I don't know if it works correctly.
 * See https://stackoverflow.com/questions/18804402/add-a-method-to-existing-c-class-in-other-file
 */

#ifndef __STREAM_EXT__
#define __STREAM_EXT__

  #include <stdint.h>
  #include <inttypes.h>
  #include <Print.h>
  
  namespace evil {
    #include_next <Stream.h>
  }

  class Stream : public evil::Stream {
  public:
    using evil::Stream::Stream;
    
    virtual void begin(unsigned long);// {}
    virtual void begin(unsigned long, uint8_t);// {}
  };

#endif
