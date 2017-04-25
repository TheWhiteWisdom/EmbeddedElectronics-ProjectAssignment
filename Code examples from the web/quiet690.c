/* quiet690.c  Does nothing. */
/* Use when you want to be sure to only communicate with the SmartCard */

/* B Knudsen Cc5x C-compiler - not ANSI-C */

#include "16F690.h"
#pragma config |= 0x00D4

void main( void)
{
   TRISA.0 = 1;   /* not to disturb PK2 UART Tool */
   TRISA.1 = 1;   /* not to disturb PK2 UART Tool */

   ANSELH.3 = 0;  /* RB5 not analog input     */
   TRISB.5  = 1;  /* RB5 input                */ 
   TRISB.7  = 0;  /* serial_out output        */
   PORTB.7  = 1;  /* serial_out marking line  */
 
   while(1) 
    {  
      nop();
    }
}






/********************
      HARDWARE
      ========
*********************/

/*
           ___________  ___________ 
          |           \/           |
    +5V---|Vdd      16F690      Vss|---GND
          |RA5        RA0/AN0/(PGD)|x ->- (PK2Rx)
          |RA4            RA1/(PGC)|x -<- (PK2Tx)
    SW1---|RA3/!MCLR/(Vpp)  RA2/INT|
          |RC5/CCP              RC0|
          |RC4                  RC1|
  CrdIn->-|RC3                  RC2|->-LED (lock)
          |RC6                  RB4|
          |RC7               RB5/Rx|-<-SerIn
 SerOut-<-|RB7/Tx               RB6|
          |________________________|

*/

