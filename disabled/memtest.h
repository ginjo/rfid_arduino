#ifndef __MEMTEST_H__
#define __MEMTEST_H__

  //==============================================================================
  // sys mem test
  //==============================================================================

  #include <Arduino.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <malloc.h>
  
  extern char _end;
  extern "C" char *sbrk(int i);
  char *ramstart=(char *)0x20070000;
  char *ramend=(char *)0x20088000;
  
  //-------------------------------------------------------------------------------------
  void memtest() {
    char sbuf[64]; // output string
    
    char *heapend=sbrk(0);
    register char * stack_ptr asm ("sp");
    struct mallinfo mi=mallinfo();
    sprintf(sbuf, "Dyn.RAM used: %-10ld ", mi.uordblks);
    Serial.println(sbuf);
    
    sprintf(sbuf, "Prg.stat.RAM used %-10ld ", & _end - ramstart);
    Serial.println(sbuf);
    
    sprintf(sbuf, "Stack RAM used %-10ld ", ramend - stack_ptr);
    Serial.println(sbuf);
    
    sprintf(sbuf, "Free mem: %-10ld ", stack_ptr - heapend + mi.fordblks);
    Serial.println(sbuf);
  }

#endif
