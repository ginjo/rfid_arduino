#ifndef __STORAGE_H__
#define __STORAGE_H__

  #include <Arduino.h>
  #include <EEPROM.h>

	#define CHECKSUM_SIZE 4
  #define STORAGE_NAME_SIZE 16

  struct Storage {

    //char storage_name[STORAGE_NAME_SIZE];
		static constexpr char *storage_name = "base-class";
		static uint16_t GetStoredChecksum();

		template <class Subclass>
		static Subclass Load(Subclass*);

    Storage();

    int save();
		uint16_t calculateChecksum();
    
  };

#endif // __STORAGE_H__
