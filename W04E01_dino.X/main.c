/*
 * File:   main.c
 * Author: mariajokinen
 *
 * Created on November 19, 2021, 1:41 PM
 */

/*general description: */

#include <avr/io.h>
#include <avr/interrupt.h>
//include the necessary header that defines ccp_write_io()
#include <avr/cpufunc.h>



//define threshold value for jumping (iOS darkmode)
#define THRESHOLD (700)

//set servo spacebar press/no press values
#define SERVO_PRESS (0x00FF)
//#define SERVO_NOPRESS (0x00DC)

#define SERVO_PWM_PERIOD        (0x1046)
//#define SERVO_PWM_DUTY_NEUTRAL  (0x0138)

/*PF4 LDR input, potentiometer input PE0*/
//variable for value conversion
volatile uint16_t comp_value = 0;
//variable for showing number
volatile uint16_t div = 0;
//variable for countdown running
volatile uint16_t g_running = 1;
//variable for counter replacing delay-function
volatile uint16_t g_clockticks = 0;


//set numbers table 0-->10 (A represents 10)
 const uint8_t numbers[]=
 {
    0b00111111, 0b00000110, 0b01011011, 0b01001111,
    0b01100110, 0b01101101, 0b01111101, 0b00000111,
    0b01111111, 0b01101111, 0b01110111
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

 void ADC0_init(void)
 {
    //Set PF4 as input (ADC/LDR)
    PORTF.DIRCLR = PIN4_bm;
    //No pull-up, no invert, disable input buffer
    PORTF.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
    // Use Vdd as reference voltage and set prescaler of 16
    ADC0.CTRLC |= ADC_PRESC_DIV16_gc | ADC_REFSEL_VDDREF_gc;
    // Enable (power up) ADC (10-bit resolution is default)
    ADC0.CTRLA |= ADC_ENABLE_bm;
    
    //ADC channel selection
    ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc;
    //2,5V reference
    VREF.CTRLA |= VREF_ADC0REFSEL_2V5_gc;
    //interrupts enabled
    ADC0.INTCTRL |= ADC_RESRDY_bm;
    
 }
 
 void TCA_init(void)
 {
    //
    PORTMUX.TCAROUTEA |= PORTMUX_TCA0_PORTB_gc;
    // Set 0-WO2 (PB2) as digital output
    PORTB.DIRSET = PIN2_bm;
    // Set TCA0 prescaler value to 16 (~208 kHz)
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc;
    // Set single-slop PWM generation mode
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    // Using double-buffering, set PWM period (20 ms)
    TCA0.SINGLE.PERBUF = SERVO_PWM_PERIOD;
    // Set initial servo arm position as 0 degrees
    TCA0.SINGLE.CMP2BUF = 312;
    // Enable Compare Channel 2
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;
    // Enable TCA0 peripheral
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
 }

int main(void) 
{
    ADC0_init();
    TCA_init();
    
    //Set port C pins (display) as outs
    PORTC.DIRSET = 0xFF;
    
    //PINF5 as out for display blinking
    //PORTF.DIRSET = PIN5_bm;
    //PORTF.OUTSET = PIN5_bm;
    
    //set LDR port
    PORTF.IN = PORTF.IN | PIN4_bm;
    
    
    //initialize RTC timer's PIT interrupt
    //rtc_init();
    
    //enable global interrupts
    sei();
    
    
    while(1)
    {
       // Start conversion (bit cleared when conversion is done)
       ADC0.COMMAND = ADC_STCONV_bm;
       // When the conversion is done, the RESRDY bit in the ADC0.INTFLAGS
       // gets set by the hardware. Without interrupt handler, the program
       // must wait for that bit to get set before reading the ADC result.
       while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
       {
        //ADC value to variable
        comp_value = ADC0.RES;
        
        div = comp_value/100;
        
        if(comp_value >= THRESHOLD)
        {
          PORTC.OUT = numbers[11];
          TCA0.SINGLE.CMP2BUF = SERVO_PRESS;
           
        }
        else
        {
           PORTC.OUT = numbers[div];
           TCA0.SINGLE.CMP2BUF = 312;
        }
       }
       // RESRDY bit must be cleared before another conversion.
       // Either by writing '1' over it or reading value from ADC0.RES.
       //ADC0.INTFLAGS = ADC_RESRDY_bm;
       
       //Show threshold hundreds
       //PORTC.OUT = numbers[div];
       
       //variable for checking even seconds
       //uint8_t div = g_clockticks%8;
       ADC0.INTFLAGS = ADC_RESRDY_bm;
       
    }
   
}
//implement interrupt
ISR(ADC0_RESRDY_vect)
{
    comp_value = ADC0.RES;
    //Clear interrupt flags
    ADC0.INTFLAGS = ADC_RESRDY_bm;
}

ISR(RTC_PIT_vect)
{
 //Clear interrupt flags 
 RTC.PITINTFLAGS = RTC_PI_bm;
 //increase variables count
 g_clockticks += 10;
}

