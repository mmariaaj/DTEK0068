/*
 * File:   main.c
 * Author: mariajokinen
 *
 * Created on November 29, 2021, 9:00 PM
 */

/*
* Program for displaying points taken as input through serial terminal.
* Program requires numeric values to display on 7-segment display. If value not
* numeric, program displays E and warns user that value was not accepted. If
* value is acceptable, program let's user know. Program uses FreeRTOS and USART.
*/

#define F_CPU 3333333
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "FreeRTOS.h"  
#include "clock_config.h"
#include "task.h"
#include "queue.h"

//defining baud rate and queue delay
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 *(float)BAUD_RATE)) + 0.5)

//length variables for both message and point queues
static const uint8_t queue_len = 5;
static const uint8_t msg_len = 5;
//initialize the two queues with QueueHandle_t
static QueueHandle_t point_queue;
static QueueHandle_t message_queue;

//table for showing point values through 7-segment display
const uint8_t points[]=
 {
    0b00111111, 0b00000110, 0b01011011, 0b01001111,
    0b01100110, 0b01101101, 0b01111101, 0b00000111,
    0b01111111, 0b01101111, 0b01111001
 };

//function for initializing USART
void USART0_init(void)
{ 
    PORTA.DIR &= ~PIN1_bm;
    PORTA.DIR |= PIN0_bm;
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);
    USART0.CTRLB |= USART_TXEN_bm;
    USART0.CTRLB |= USART_RXEN_bm;
}

//function for reading USART input
char USART0_read()
{
    while (!(USART0.STATUS & USART_RXCIF_bm))
    {
    ;
    }
    return USART0.RXDATAL;
}

//function for sending single char through USART
void USART0_sendChar(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
    {
    ;
    }
   
    USART0.TXDATAL = c; 
}

//function for sending string message through USART
void USART0_sendString(char *str)
{
    for(size_t i = 0; i < strlen(str); i++)
{
        USART0_sendChar(str[i]);
    }
}

//combined task for sending input to both queues
void char_to_queues(void* parameter)
{
   char point;
   
   for(;;)
   {
       //read char through USART
       point = USART0_read();
       //send read char to both message and point queue
       xQueueSend(message_queue, (void *)&point, 10);
       xQueueSend(point_queue, (void *)&point, 10);
   }
   
  // loop doesn't end but task should have vTaskDelete() call just in case
  vTaskDelete(NULL); 
}

//task for displaying message through USART
void display_message(void* parameter){
    
    char c;
    
    for(;;)
    {
        //check that queue has a value
        if(xQueueReceive(message_queue, (void *)&c, 0) ==pdTRUE)
        {
            //check that input is number between 0-9
            if(((uint8_t)c - 48 >= 0) && ((uint8_t)c - 48 < 10)){
                //send message that input was valid
                USART0_sendString(" is a valid digit\r\n");
            }
            else
            {
                //Send error message
                USART0_sendString("\r\n Error! Not a valid digit\r\n");
            }    
        }
   
    }
    
    vTaskDelete(NULL);
}

//task for displaying points on display
void display_points(void* parameter)
{
    char c;
    
    // This task will run indefinitely
    for (;;)
    {
        
        if(xQueueReceive(point_queue, (void *)&c, 0) ==pdTRUE)
        {
            //make sure digit is between 0-9, uint8_t format requires a deduction of 48
            if((uint8_t)c - 48 >= 0 && (uint8_t)c - 48 < 10)
            {
                //show digit in question from points-table
                PORTC.OUT = points[(uint8_t)c - 48];
            }
            else
            {
                //show E incase not digit
                PORTC.OUT = points[10];
            }
        }
        
    }

vTaskDelete(NULL);
}


int main(void)
{
    //set pins for 7-segment display control
    PORTC.DIR = 0xFF;
    PORTF.DIRSET = PIN5_bm;
    PORTF.OUTSET = PIN5_bm;

    //Initialize USART
    USART0_init();
    
    //create task for sending received char to different queues 
    xTaskCreate(
      char_to_queues,
      "char to queues",
      configMINIMAL_STACK_SIZE,
      NULL,
      tskIDLE_PRIORITY,
      NULL
     );
    
    
    // Create task for displaying points on 7-segment display
    xTaskCreate(
      display_points,
      "points",
      configMINIMAL_STACK_SIZE,
      NULL,
      tskIDLE_PRIORITY,
      NULL
     );
    
    //create task for sending messages through USART
    xTaskCreate(
      display_message,
      "message",
      configMINIMAL_STACK_SIZE,
      NULL,
      tskIDLE_PRIORITY,
      NULL
     );
    
    //create queues defined in the beginning
    point_queue = xQueueCreate(queue_len, sizeof(char));
    message_queue = xQueueCreate(msg_len, sizeof(char));

    
    //Start task scheduler
    vTaskStartScheduler();
    
    
//return an integer value as defined
return 0;
}

