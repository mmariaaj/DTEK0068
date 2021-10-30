/*
 * File:   main.c
 * Author: mariajokinen
 *
 * Created on October 30, 2021, 4:26 PM
 */


#include <xc.h>

void main(void) {
 
    // Set PF5 (LED) as out, PF6 as in
    PORTF.DIRSET = PIN5_bm;
    PORTF.DIRCLR = PIN6_bm;
  
    // The superloop
    while (1)
    {
          if(PORTF.IN & PIN6_bm){
             PORTF.OUT |= PIN5_bm;    
          }
          else{
             PORTF.OUT &= ~PIN5_bm;;
          }
    }
    return;
}
