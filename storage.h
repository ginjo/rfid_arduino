#ifndef __STORAGE_H__
#define __STORAGE_H__

  #include <Arduino.h>
  #include <string.h>
  #include <EEPROM.h>
  #include "logger.h"

	//#define STORAGE_CHECKSUM_SIZE 9
  #define STORAGE_NAME_SIZE 16

  /*  Note that all calls to static vars from base class must use templates
   *  to qualify the derived class name. See my subclassing & template
   *  cpp examples. Also note that we are no longer using static vars in this way.
   *  
   *  This is a base class, Storage, that provides EEPROM persistance for any sublclass instance.
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
        
    static T* Load (T * object_ref, int eeprom_address) {
      Serial.println(F("Storage::Load() BEGIN"));
      
      EEPROM.get(eeprom_address, *object_ref); // .get() expects data, not pointer.

      object_ref->eeprom_address = eeprom_address;
      
      if (! object_ref->checksumMatch()) {
        Serial.println(F("Storage::Load() checksum mismatch"));
        //object_ref->checksum = (uint16_t)0;
      }

      #ifdef DEBUG
        Serial.print(F("eeprom_address ")); Serial.println(eeprom_address);
        Serial.print(F("object_ref->storage_name '")); Serial.print(object_ref->storage_name); Serial.println("'");
        Serial.print(F("sizeof(*object_ref) ")); Serial.println(sizeof(*object_ref));
        Serial.print(F("sizeof(T) ")); Serial.println(sizeof(T));
      #endif
  
      Serial.println(F("Storage::Load() END"));
      return object_ref;
    }


    /***  Instance Vars & Functions  ***/

    int eeprom_address;
    uint16_t checksum;
    char storage_name[STORAGE_NAME_SIZE];

    /* Constructor */
    Storage(const char *_storage_name, int _eeprom_address = -1) {
      strlcpy(storage_name, _storage_name, STORAGE_NAME_SIZE);
      if (_eeprom_address >= 0) eeprom_address = _eeprom_address;
    }
    
    // Saves this Storage instance to the correct storage address.
    // Sub-classes, like Settings, should carry the info about
    // what address to use.
    int save(int _eeprom_address = -1) {
      Serial.println(F("Storage::save() BEGIN"));

      if (_eeprom_address >= 0) eeprom_address = _eeprom_address;

      #ifdef DEBUG
        Serial.print(F("Storage::save() '")); Serial.print(storage_name);
        Serial.print(F("' to address ")); Serial.print(eeprom_address);
        Serial.print(F(" sizeof(T) ")); Serial.println(sizeof(T));
      #endif
      
      if (! checksumMatch()) {
        Serial.println(F("Storage::save() chksm mismatch, calling EEPROM.put()"));

        checksum = calculateChecksum();
        
        // Must use dereferenced data here, or you store pointer address instead of data.
        // The use of the class template is very important here, as the eepprom
        // functions don't seem to get it when called from a subclass.
        EEPROM.put(eeprom_address, *(T*)this);
  
        Serial.println(F("Storage::save() END"));
        return 0;
      } else {
        Serial.println(F("Storage::save() EEPROM update not necessary"));
        Serial.println(F("Storage::save() END"));
        return 1;
      }
    }
  
    uint16_t calculateChecksum() {
      // The 'checksum' field needs to always be 0 when
      // calculating object checksum.
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

      //  #ifdef DEBUG
      //    Serial.print(F("Storage::claculateChecksum() 0x")); Serial.print(xxor, 16);
      //    Serial.print(F(" for storage_name '")); Serial.print(storage_name);
      //    Serial.print(F("' of size ")); Serial.println(len);
      //  #endif

      checksum = stored_checksum;
      
      return xxor;    
    }

    bool checksumMatch() {
      //  uint16_t stored_checksum = checksum;
      //  checksum = (uint16_t)0;
      uint16_t calculated_checksum = calculateChecksum();
      bool result = (
        checksum == calculated_checksum &&
        checksum != (uint16_t)0 &&
        checksum != 0xFFFF
      );
      
      Serial.print("Storage::checksumMatch() 0x"); Serial.print(checksum, 16);
      Serial.print(" 0x"); Serial.print(calculated_checksum, 16);
      Serial.print(", bool-result: "); Serial.println(result);
      
      //  checksum = stored_checksum;
      return result;
    }
    
  };

#endif // __STORAGE_H__
