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
#define COUNTDOWNSTART 9

volatile uint16_t g_running = 1;

//set numbers table 0-->9
 const uint8_t numbers[]=
    {
        0b00111111, 0b00000110, 0b01011011, 0b01001111,
        0b01100110, 0b01101101, 0b01111101, 0b00000111,
        0b01111111, 0b01101111
    };




int main(void) 
{
   

    //PINA4 as red wire
    PORTA.DIRCLR= PIN4_bm;
    //PINA? triggers interrupt when wire is pulled
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    
    
    //Set port C pins as outs
    PORTC.DIRSET = PIN7_bm |PIN6_bm | PIN5_bm | PIN4_bm |PIN3_bm |PIN2_bm |PIN1_bm |PIN0_bm;
    //create undefined variables for increment and index
    
    uint8_t count = COUNTDOWNSTART;
    
    //enable global interrupts
    sei();
    
    
    
    while(1)
    {
       //Starts from 9 (index 0) and continues to count indexes upwards (numbers downwards)
       PORTC.OUT = numbers[count];
       if(count < 1)
       {
        //When reaches number 0 (index 10), continues to blink empty and 0
           while(1)
           {
              //blinks empty or 0 with 1 second delay
              PORTC.OUT = numbers[0];
              _delay_ms(500);
              PORTC.OUT = 0x00;
              _delay_ms(500);
           }
           
       }
       // 1s delay between numbers
       _delay_ms(1000);
       
       //Halt
       while(!g_running);
       
       count--;
       
    }
   
}
//implement interrupt
ISR(PORTA_PORT_vect)
{
    //Clear specific flags
    VPORTA.INTFLAGS = PIN4_bm;
    //Halt timer with timer
    g_running = 0;
}
