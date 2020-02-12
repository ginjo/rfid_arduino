#ifndef __STORAGE_H__
#define __STORAGE_H__

  #include <Arduino.h>
  #include <string.h>
  #include <EEPROM.h>
  #include "logger.h"

  // See logger.h for master debug controls.
  #ifdef SO_DEBUG
    #define SO_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define SO_LOG(...)
  #endif

  #define STORAGE_NAME_SIZE 16 // should include null-terminator
  
  #define STATE_EEPROM_ADDRESS 0
  #define TAGS_EEPROM_ADDRESS 700
  #define SETTINGS_EEPROM_ADDRESS 800

  /*  Note that all calls to static vars from base class must use templates
   *  to qualify the derived class name. See my subclassing & template
   *  cpp examples. Also note that we are no longer using static vars in this way.
   *  
   *  This is a base class, Storage, that provides EEPROM persistance for any sublclass instance.
   *  All non-static member data will be saved with the instance.
   *  
   *  The key to making this work is to make the base class also a template class, and to
   *  pass in the subclass name when declaring the subclass. This is referred to as the CRTP
   *  pattern "Curiously Recurring Template Pattern". It helps subclasses make effective use
   *  of base instance methods that need to know what subclass is calling them.
   *  
   *  IMPORTANT: templates are compiler features. No code is written until a class template is instanciated.
   *  
   *  IMPORTANT: The CRTP pattern draws a separate base class for each subclass.
   *             Therefore, extra steps need to be taken if you need a polymorphic base (google it).
   *             This Storage class is not used polymorphicly, so not a problem here.
   *  
   *  TODO: Refactor:
   *        √ Store eeprom_address in Storage instance.
   *        √ Make Storage::Load the only function that requires passing eeprom_address,
   *          but allow other functions to optionally receive eeprom_address.
   *        √ Make GetStoredChecksum() an instance method, getStoredChecksum(),
   *          and retrieve the stored-checkum to an instance var, stored_checksum, whenever
   *          the Storage instance is loaded (or re-loaded) from eeprom.
   *          Then save the update and save the stored checksum, whenever instance is stored.
   *        Warning: This gets tricky, because we don't want to store the stored_checksum
   *          in the Storage instance, when we store the Storage instance.
   *          It becomes a chicken/egg problem.
   *        Update: Actually, we do store the checksum in the storage instance,
   *          but we set it to 0 whenever we re-calculate the checksum, so we
   *          avoid the chicken/egg issue.
   */

  template <class T>
  struct Storage {


    /***  Static / Class Vars & Functions  ***/

    /* Constructor */
    Storage(const char *_storage_name, int _eeprom_address = -1) :
      /*
        eeprom_address should not default to 0,
        since that is a legitimate address and may
        be in use.
      */
      eeprom_address(_eeprom_address),
      checksum(0U)
    {
      strlcpy(storage_name, _storage_name, STORAGE_NAME_SIZE);
    }


    static bool Load (T * object_ref, int eeprom_address) {
      SO_LOG(5, F("Strg Load() Begin"), true);

      T temp_obj;// = T(); Does this work without the T()?

      /*
        .get() apparently expects data (a regular object),
        not a pointer to an object.
        
        I think .get coppies the the loaded object,
        instead of just repointing.
      */
      EEPROM.get(eeprom_address, temp_obj);

      //temp_obj.eeprom_address = eeprom_address;

      bool rslt = temp_obj.checksumMatch();

      SO_LOG(6, F("Strg eeprom_address "), false); SO_LOG(6, eeprom_address, true);
      SO_LOG(6, F("Strg object_ref->storage_name '"), false); SO_LOG(6, temp_obj.storage_name, false); SO_LOG(6, "'", true);
      SO_LOG(6, F("Strg sizeof(*object_ref) "), false); SO_LOG(6, sizeof(temp_obj), true);
      SO_LOG(6, F("Strg sizeof(T) "), false); SO_LOG(6, sizeof(T), true);
      
      if (rslt) {
        *object_ref = temp_obj;
      } else {
        LOG(3, F("Strg Load() checksum mismatch"), true);
      }

      object_ref->eeprom_address = eeprom_address;

      SO_LOG(6, F("Strg Load() End"), true);
      
      return rslt;
    }


    /***  Instance Vars & Functions  ***/

    int eeprom_address;
    uint16_t checksum;
    char storage_name[STORAGE_NAME_SIZE];
    
    // Saves this Storage instance to the correct storage address.
    // Sub-classes, like Settings, should carry the info about
    // what address to use.
    int save(int _eeprom_address = -1) {
      SO_LOG(6, F("Strg save() BEGIN"), true);

      // TODO: Gracefully fail if address < 0, or if any of the 3 Storage fields are not correct.
      if (_eeprom_address >= 0) eeprom_address = _eeprom_address;

      SO_LOG(5, F("Strg save() '"), false); SO_LOG(5, storage_name, false);
      SO_LOG(5, F("' to address "), false); SO_LOG(5, eeprom_address, false);
      SO_LOG(5, F(" sizeof(T) "), false); SO_LOG(5, sizeof(T), true);

      // TODO: Reverse the logic here, just cuz it will read better.
      if (checksumMatch()) {

        SO_LOG(5, F("Strg save() EEPROM update not necessary"), true);
        SO_LOG(6, F("Strg save() END"), true);
        LOG(4, F("EEPROM update not necessary"), true);
        return 0;

      } else {

        SO_LOG(5, F("Strg save() chksm mismatch, calling EEPROM.put()"), true);

        checksum = calculateChecksum();
        
        // Must use dereferenced data here, or you store pointer address instead of data.
        // The use of the class template is very important here, as the eepprom
        // functions don't seem to get it when called from a subclass.
        EEPROM.put(eeprom_address, *(T*)this);
        LOG(4, F("EEPROM updated"), true);

        SO_LOG(6, F("Strg save() END"), true);
        
        return 1;
      }
    }
  
    uint16_t calculateChecksum() {
      /*
        The 'checksum' field needs to always be 0 when
        calculating object checksum. Otherwise you have
        an infinite recursion chicken/egg problem.
      */
      uint16_t stored_checksum = checksum;
      checksum = (uint16_t)0;
      
      unsigned char *obj = (unsigned char *) (T*)this;
      //uint16_t len = sizeof(*this); // de-references pointer, so we get size of actual data.
      uint16_t len = sizeof(T);
      uint16_t xxor = 00;
  
      // Converts to 16-bit checksum, and handles odd bytes at end of obj.
      for ( uint16_t i = 0 ; i < len ; i+=2 ) {
        xxor = xxor ^ ((obj[i]<<8) | (i==len-1 ? 0 : obj[i+1]));
      }

      //  #ifdef SO_DEBUG
      //    LOG(4, F("Storage::claculateChecksum() 0x")); LOG(4, xxor, 16);
      //    LOG(4, F(" for storage_name '")); LOG(4, storage_name);
      //    LOG(4, F("' of size ")); LOG(4, len, true);
      //  #endif

      checksum = stored_checksum;
      
      return xxor;    
    }

    bool checksumMatch() {
      uint16_t calculated_checksum = calculateChecksum();
      bool result = (
        checksum == calculated_checksum &&
        checksum != (uint16_t)0 &&
        checksum != 0xFFFF
      );

      #ifdef SO_DEBUG
        // This might need to use LOG cuz it needs to pass integer base.
        SO_LOG(6, "Strg checksumMatch() stor 0x", false); LOG(6, checksum, 16);
        SO_LOG(6, " calc 0x", false); LOG(6, calculated_checksum, 16);
        SO_LOG(6, ", bool-result: ", false); SO_LOG(6, result, true);
      #endif
          
      return result;
    }
    
  };

#endif // __STORAGE_H__


  
