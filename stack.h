/* Provides function-pointer stack features */

/* Note that when passing c++ member function pointers,
 *  it is necessary to use the full-qual name of the object,
 *  and often you will need to precede with ampersand, '&',
 *  but could also be preceded with nothing.
 */

#ifndef __FUNCTION_STACK_H__
#define __FUNCTION_STACK_H__

  // See logger.h for master debug controls.
  #ifdef SK_DEBUG
    #define SK_LOG(level, dat, line) LOG(level, dat, line)
  #else
    #define SK_LOG(...)
  #endif

  #include <Arduino.h>
  #include "settings.h"
  #define FUNCTION_STACK_SIZE 5


  template <class T>
  class Stack {
  public:
    // TODO: Consider returning int instead of void from this type.
    typedef void (T::*CB)(void*);

    CB stack[FUNCTION_STACK_SIZE] = {};
    int stack_index = -1;
    
    virtual void push(CB func) {
      SK_LOG(5, "Stack::push() to index ", false); SK_LOG(5, stack_index+1, true);
      // SK_PRINT(T::instance_name); SK_PRINT(", ");
      //((T*)this->*func)((char*)"push() called with this func");
      if (func) {
        stack_index += 1;
        stack[stack_index] = func;
      } else {
        LOG(3, "Tried to push nullptr to stack.", true);
      }
    }

    virtual CB pop() {
        SK_LOG(5, "Stack::pop() from index ", false); SK_LOG(5, stack_index, true);
        if (stack_index < 0) return nullptr;
        CB func = top();
        stack_index -= 1;
        //((T*)this->*func)((char*)"pop() called, returning this func");
        return func;
    }
    
    virtual CB top() {
        SK_LOG(5, "Stack::top() index ", false); SK_LOG(5, stack_index, true);
        if (stack_index < 0) return nullptr;
        return stack[stack_index];
    }

    virtual void call(void *dat = nullptr, bool _pop = false) {
        SK_LOG(5, "Stack::call() index ", false); SK_LOG(5, stack_index, true);
        CB cback = top();
        if (stack_index < 0 || !cback) return;
        // Need to pop before calling, otherwise pop will happen
        // after called func has ended, which causes problems downstream.
        //((T*)this->*top())(dat);
        if (_pop) pop();
        ((T*)this->*cback)(dat);
    }

    // Optionally receives function pointer of type CB.
    virtual void resetStack(CB func = nullptr) {
      SK_LOG(5, "Stack::resetStack() with index ", false); SK_LOG(5, stack_index, true);
      stack_index = -1;
      
      for (int i=0; i < FUNCTION_STACK_SIZE; i++) {
        stack[i] = nullptr;
      }
      
      if (func) {
        push(func);
      }
    }
    
  }; // end Stack

#endif


  
