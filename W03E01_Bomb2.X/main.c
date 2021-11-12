/*
 * File:   main.c
 * Author: mariajokinen
 *
 * Created on November 12, 2021, 5:59 PM
 */

/*General description: Program implementing a "bomb" for ATMega4809 board.
 *With the course specific build on breadboard the program will count down from
 *9 on 7-segment display. It will flash 0 after reaching it and it will stop
 *counting if red wire (wire from PA4 to GRN) is pulled mid count.*/

#include <avr/io.h>
#include <avr/interrupt.h>
//include the necessary header that defines ccp_write_io()
#include <avr/cpufunc.h>
//header needed for idle sleep implementation
#include <avr/sleep.h>
#define COUNTDOWNSTART 9

//variable for countdown running
volatile uint16_t g_running = 1;
//variable for counter replacing delay-function
volatile uint16_t g_clockticks = 0;

//set numbers table 0-->9
 const uint8_t numbers[]=
 {
    0b00111111, 0b00000110, 0b01011011, 0b01001111,
    0b01100110, 0b01101101, 0b01111101, 0b00000111,
    0b01111111, 0b01101111
 };

 //Periodic interrupt timer, from material provided
 void rtc_init(void)
 {
    //variable for register value manipulation
    uint8_t timer;
    //disable oscillator
    timer = CLKCTRL.XOSC32KCTRLA;
    timer &= ~CLKCTRL_ENABLE_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, timer);
    //Wait for clock to be released
    while(CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm);
    
    //Select external crystal
    timer = CLKCTRL.XOSC32KCTRLA;
    timer &= ~CLKCTRL_SEL_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, timer);
    
    //enable oscillator
    timer = CLKCTRL.XOSC32KCTRLA;
    timer |= CLKCTRL_ENABLE_bm;
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, timer);
    //wait for clock to stabilize
    while(RTC.STATUS > 0);
    
    //RTC module configuration
    //Select external oscillator
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;
    //enable periodic interrupt
    RTC.PITINTCTRL = RTC_PI_bm;
    //set period to 1/8 second cycle and enable PIT function
    RTC.PITCTRLA = RTC_PERIOD_CYC4096_gc | RTC_PITEN_bm;
 }


int main(void) 
{
   

    //PINA4 as red wire
    PORTA.DIRCLR= PIN4_bm;
    //trigger interrupt when wire is pulled from PINA4
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    
    //Set port C pins as outs
    PORTC.DIRSET = PIN7_bm |PIN6_bm | PIN5_bm | PIN4_bm |PIN3_bm |PIN2_bm |PIN1_bm |PIN0_bm;
    //PINF5 as out for display blinking
    PORTF.DIRSET = PIN5_bm;
    PORTF.OUTSET = PIN5_bm;
    
    //idle sleep mode
    set_sleep_mode(SLPCTRL_SMODE_IDLE_gc);
    
    //initialize RTC timer's PIT interrupt
    rtc_init();
    
    //enable global interrupts
    sei();
    
    //create undefined variable for counting down
    uint8_t count = COUNTDOWNSTART;
    
    //superloop
    while(1)
    {
       //Starts from 9 (index 10) and continues to count downwards
       PORTC.OUT = numbers[count];
       //variable for checking even seconds
       uint8_t div = g_clockticks%8;
       //When reaches number 0 (index 10) toggles PINF5
       if((count == 0) & (div == 0))
       {
        //Toggle PF5
        PORTF.OUTTGL = PIN5_bm;   
       }
       //if countdown is running and g_clockticks is larger than 0
       //and g_clockticks is dividable by 8, count decreases
       //if g_clockticks was not defined as >0 number 9 would only flash quickly
       else if((g_running) & (g_clockticks != 0) & (div == 0))
       {
          //decrease table index number / number shown in display
           count--;
       }
       
       //idle sleep
       sleep_mode();
       
    }
   
}
//implement interrupt
ISR(PORTA_PORT_vect)
{
 //Clear interrupt flags
 VPORTA.INTFLAGS = PIN4_bm;
 //Halt timer
 g_running = 0;
}

ISR(RTC_PIT_vect)
{
 //Clear interrupt flags 
 RTC.PITINTFLAGS = RTC_PI_bm;
 //increase variables count
 g_clockticks ++;
}
