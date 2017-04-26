
#include "16F84.h"
#pragma config |= 0x3ff1
char ID;
char passPhrase;

/* Function prototype*/
void initialize();
char modPow(char in_value , int pow, char mod);
void putchar( char d_out );
char getchar( void );

void main(){
	initialize();
	while(1){
        putchar('t');
	}
}
void initialize(){	
	PORTB.7 = 1;
	TRISB.7 = 1;
}

/*
* NOTE:
* Code is copied from the example code smartkey.c
*/
void putchar( char d_out )  /* sends one char */
{
   char cypher_char= modPow( d_out, 7, 143);
   char bitCount, ti;
   TRISB.7 = 0; /* output mode */
   PORTB.7 = 0; /* set startbit */
   for ( bitCount = 10; bitCount > 0 ; bitCount-- )
        {
         /* 104 usec at 3,58 MHz (5+27*3-1+9=104) */  
         // ti = 27; do ; while( --ti > 0);
         /* 104 usec at 4 MHz (5+30*3-1+1+9=104)  */         
          ti = 30; do ; while( --ti > 0); nop();   
          Carry = 1;           /* stopbit                        */
          cypher_char = rr( cypher_char ); /* Rotate Right through Carry     */
          PORTB.7 = Carry;
        }
        nop2(); nop2();
   return; /* all done */
}