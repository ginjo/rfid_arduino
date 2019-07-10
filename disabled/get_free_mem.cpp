//  // Gets free-memory, see https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
//  // This should really go in a Utility class.
//  // It is only here as a quick fix.    
//  #ifdef __arm__
//  // should use uinstd.h to define sbrk but Due causes a conflict
//  extern "C" char* sbrk(int incr);
//  #else  // __ARM__
//  extern char *__brkval;
//  #endif  // __arm__
//  
//  static int freeMemory() {
//    char top;
//  #ifdef __arm__
//    return &top - reinterpret_cast<char*>(sbrk(0));
//  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
//    return &top - __brkval;
//  #else  // __arm__
//    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
//  #endif  // __arm__
//  }

