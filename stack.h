/* Provides function-pointer stack features */

#ifndef __FUNCTION_STACK_H__
#define __FUNCTION_STACK_H__

  #include <Arduino.h>
  #define FUNCTION_STACK_SIZE 5

  template <class T>
  class Stack {
  public:
    typedef int (T::*CB)(void*);

    CB stack[FUNCTION_STACK_SIZE] = {};
    int stack_index = -1;
    
    virtual void push(CB func) {
        printf("push: %i -> %i\n", stack_index, stack_index+1);
        //((T*)this->*func)((char*)"push() called with this func");
        stack_index += 1;
        stack[stack_index] = func;
    }

    virtual CB pop() {
        printf("pop: %i -> %i\n", stack_index, stack_index-1);
        if (stack_index < 0) return NULL;
        CB func = top();
        stack_index -= 1;
        //((T*)this->*func)((char*)"pop() called, returning this func");
        return func;
    }
    
    virtual CB top() {
        printf("top: %i\n", stack_index);
        if (stack_index < 0) return NULL;
        return stack[stack_index];
    }

    virtual void call(void *dat = NULL, bool _pop = false) {
        printf("call: %i\n", stack_index);
        if (stack_index < 0) return;
        ((T*)this->*top())(dat);
        if (_pop) pop();
    }

    // TODO: Consider receiving a function to add to position 0 of the stack.
    virtual void resetStack(CB func = NULL) {
      stack_index = -1;
      for (int i=0; i < FUNCTION_STACK_SIZE; i++) {
        stack[i] = NULL;
      }
      push(func);
    }
    
  };

#endif
