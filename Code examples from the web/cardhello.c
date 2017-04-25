/* crdhello.c (for GoldCard 16F84) Send "hello, world" to PC Hyperterminal-program */

/*
SmartCard contact

             __ __ __
       +5V  |C1|   C5| Gnd
            |__|   __|
   MCLR/PGM |C2|  |C6| 
            |__|  |__|
    OSC/PGC |C3|  |C7| RB7/PGD I/O 
            |__|  |__|
            |C4|  |C8|
            |__|__|__|

*/


/*
   SERIAL COMMUNICATION (RS232)
   ============================
   One start bit, one stop bit, 8 data bit, no parity = 10 bit.
   Baudrate: 9600 baud => 104.167 usec. per bit.
   ser_out PORTB.7
*/

#include "16F84.h"
/*  Circuitprogrammer Configuration:  */
#pragma config |= 0x3ff1

void putchar( char );
void stringout(const char *string); 
void delay10( char );

void main( void )
{
   while(1)
    {
      char i;
      PORTB.7 = 1;
      TRISB.7 = 0;  /* Marking line */
      delay10(100);
      stringout("\r\nhello, world!");  
    }
}



/************************
   Functions
   =========
*************************/

void putchar( char d_out )  /* sends one char */
{
   char bitCount, ti;
   TRISB.7 = 0; /* output mode  */
   PORTB.7 = 0; /* set startbit */
   for ( bitCount = 10; bitCount > 0 ; bitCount-- )
        {
          
         // ti = 27; do ; while( --ti > 0);      /* 104 usec at 3,58 MHz (5+27*3-1+9=104) */ 
          ti = 30; do ; while( --ti > 0); nop(); /* 104 usec at 4 MHz (5+30*3-1+1+9=104)  */  
          Carry = 1;           /* stopbit                        */
          d_out = rr( d_out ); /* Rotate Right through Carry     */
          PORTB.7 = Carry;
        }
        nop2(); nop2();
    return; /* all done */
}


void stringout(const char *string)
{
  char i, k;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') return;   // at end of string
     putchar(k); 
   }
  return;
}

void delay10( char n)
/*
  Delays a multiple of 10 milliseconds using the TMR0 timer
  Clock : 4 MHz   => period T = 0.25 microseconds
  1 IS = 1 Instruction Cycle = 1 microsecond
  error: 0.16 percent
*/
{
    char i;

    OPTION = 7;
    do  {
        i = TMR0 + 39; /* 256 microsec * 39 = 10 ms */
        while ( i != TMR0)
            ;
    } while ( --n > 0);
}


