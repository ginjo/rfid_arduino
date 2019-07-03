#ifndef __STORAGE_H__
#define __STORAGE_H__

  #include <Arduino.h>
  #include <EEPROM.h>
  #include "logger.h"

  #define storage_name_size 16

  struct Storage {
    /*  Private Implementation Details
     *  storage_name is a string representation
     *  of the subclass name, since we can't
     *  instrospect the name at runtime.
     *  TODO: Should this be a 'const'?
     */
    char storage_name[storage_name_size];

    /*  Constructors  */

    Storage(const char *);
    

    /*  Functions  */

    virtual void save();
    virtual void save(int eeprom_address);

    //static Storage load(int);
    
  };

#endif  //  end __STORAGE_H__
