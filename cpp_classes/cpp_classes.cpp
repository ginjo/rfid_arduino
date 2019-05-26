/******************************************************************************

This example shows an LED class with constructor that takes
an array of on/off intervals.

This was designed to run on https://www.onlinegdb.com.

*******************************************************************************/
#include <stdio.h>

class Led {
public:
    int pin;
    int interval_count;
    unsigned long interval[];

    //Led(int pin, unsigned long interval[]);
    Led(int p, int c, unsigned long i[]) {
        pin = p;
        interval_count = c;
        printf("Initializing Led instance on pin %i with %i intervals\n", p, c); 
        for(int n = 0; n < c; n++) {
            printf("%i. %u\n", n, i[n]);
            interval[n] = i[n];
        }
        printf("\n");

    }
};

int main()
{
    printf("Hello world, running main()\n");
    
    unsigned long interval[] = {50,20,50,2880};
    Led red(9, 4, interval);

    printf("pin: %i\n", red.pin);
    printf("intervals (%i): ", red.interval_count);
    
    int n; // so we can preserve the final n after the loop
    for (n = 0; n < red.interval_count - 1; n++) {
        printf("%u, ", red.interval[n]);
    }
    printf("%u", red.interval[n]);
    
    return 0;
}
