/* Provides function-pointer stack features */

#ifndef __FUNCTION_STACK_H__
#define __FUNCTION_STACK_H__

  template <class T>
  class Stack {
  public:
    typedef int (T::*CB)(void*);

    CB stack[5] = {};
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
    
  };

#endif
