/*
 * File:   main.c
 * Author: mariajokinen
 *
 * Created on November 6, 2021, 2:58 PM
 */

#define F_CPU 3333333
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint16_t g_wire;

//implement interrupt
ISR(PORTA_PORT_vect)
{
    g_wire = PORTA.DIRCLR;
}



void main(void) 
{
   
//set numbers table 9-->0 including blank at index 11
    uint8_t numbers[]=
    {
        0b01101111, 0b01111111, 0b00000111, 0b01111101,
        0b01101101, 0b01100110, 0b01001111, 0b01011011,
        0b00000110, 0b00111111, 0b00000000
    };
    
    //Set port C pins as outs
    PORTC.DIRSET = PIN7_bm |PIN6_bm | PIN5_bm | PIN4_bm |PIN3_bm |PIN2_bm |PIN1_bm |PIN0_bm;
    //create undefined variables for increment and index
    uint8_t increment = 1;
    uint8_t index = 0;
    
    //interrupt
    PORTA.PIN4CTRL= PORT_ISC_RISING_gc;
    //enable global interrupts
    sei();
    
    while(1)
    {
       //Starts from 9 (index 0) and continues to count indexes upwards (numbers downwards)
       PORTC.OUT = numbers[index];
       if(index < 10)
       {
           //index rises, 1 second delay between numbers
           index += increment;
           _delay_ms(1000);
       }
       else
       {
           //When reaches number 0 (index 10), continues to blink empty and 0
           while(1)
           {
              //blinks empty or 0 with 1 second delay
              PORTC.OUT = numbers[10];
              _delay_ms(1000);
              PORTC.OUT = numbers[11];
              _delay_ms(1000);
           }
       
       }
       
    }
   
}
