#ifndef __STORAGE_H__
#define __STORAGE_H__

  #include <Arduino.h>
  //#include <EEPROM.h>

	#define STORAGE_CHECKSUM_SIZE 9
  #define STORAGE_NAME_SIZE 16

  /* All calls to static vars from base class must use templates
   *  to qualify the derived class name. See my subclassing & template
   *  cpp examples.
   */

  struct Storage {
    
		static uint16_t GetStoredChecksum(int);
    static Storage *Load(Storage*, int, int);

    char storage_name[STORAGE_NAME_SIZE];

    Storage(const char*);
    int save(int, int);
		uint16_t calculateChecksum();
    
  };

#endif // __STORAGE_H__
