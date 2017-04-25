
#include "16F84.h"
int* passPhrases;
char ID= 54;
char passPhrase= 43;

/* Function prototype*/
void communicationSequence();
char modPow(char in_value , int pow, char mod);
void putchar( char d_out );
char getchar( void );


void main(){
	while(1){
		communicationSequence();
	}
}

void communicationSequence()
{
  putchar(ID);
  char receivedChar = getchar();
  if(receivedChar==10)
    while(1)
      nop;
  receivedChar-=20;
  putchar(passPhrase);
  receivedChar= getchar();
  if(receivedChar==10)
    while(1)
      nop;
    else if(receivedChar==11)
      while(1)
        nop;


}

//used for encryption and decryption
char modPow(char in_value , int pow, char mod)
{
 char ret_value=0x01;
 int i;
for(i=pow; i>0; i--)
{
  ret_value=(ret_value*in_value);
  while(ret_value>mod && ret_value>0)
  {
    ret_value=ret_value-mod;
  }
  while(ret_value<0)
    ret_value+=mod;

}
return ret_value;
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

/*
* NOTE:
* Code is copied from the example code smartkey.c
*/
char getchar( void )  /* recieves one char */
{
   /* One start bit, one stop bit, 8 data bit, no parity = 10 bit. */
   /* Baudrate: 9600 baud => 104.167 usec. per bit.                */
   TRISB.7 = 1; /* set input mode */
   char d_in, bitCount, ti;
   while( PORTB.7 == 1 )  /* wait for startbit */ ;
   /* delay 1,5 bit is 156 usec                         */ 
   /* 156 usec is 156 op @ 4 MHz ( 5+47*3-1+2+9=156)    */
   ti = 47; do ; while( --ti > 0); nop2();  
   for( bitCount = 8; bitCount > 0 ; bitCount--)
       {
        Carry = PORTB.7;
        d_in = rr( d_in);  /* rotate carry */
        /* delay 1 bit is 104 usec                       */ 
        /* 104 usec is 104 op @ 4 MHz (5+30*3-1+1+9=104) */  
        ti = 30; do ; while( --ti > 0); nop();  
        }
   return modPow(d_in, 7, 143);
}
