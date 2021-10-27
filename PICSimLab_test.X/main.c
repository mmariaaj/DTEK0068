/*
 * File:   main.c
 * Author: dtek
 *
 */
#define F_CPU   2000000

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
     /*
      * Set pin PB5 (LED) as output
      */
     DDRB |= (1 << PIN5);

    while (1)
    {
		/*
         * Reverse state of led
		 */
        PORTB ^= (1 << PIN5);
        _delay_ms(500);
    }
}
