/* B Knudsen Cc5x C-compiler - not ANSI-C */
#include "16F690.h"
#pragma config |= 0x00D4 

int* passPhrase;
int* customerIDs;
int* ownerIDs;

char customerID = 87;
char ownerID = 54;
char passPhrase = 43;

void initialize();
void communicationSequence();
char modPow(char in_val, int pow, char mod);
void putchar( char d_out );
char getchar( void );



void main(){
	initialize();
	while(1){
		// TODO: Lights off
		communicationSequence();
	}
}

void initialize(){
	TRISB.7 = 0;
	TRISC.2 = 1;
}

void communicationSequence(){
	char ID = getchar();
	putchar(20 + 1);
	if(getchar() != passPhrase){
		putchar(10);
		return;
	}
	if(ID == customerID){
		putchar(2);
		if(getchar() <= 0){
			putchar(10);
			return;
		}else{
			putchar(11);
		}
	} else if(ID == ownerID){
		putchar(11);
		// TODO: Lights on!
		while(ID == ownerID){
			putchar(10);
			ID = getchar();
		}
		if(ID == customerID){
			putchar(20 + 1);
			if(getchar() != passPhrase){
				putchar(10);
				return;
			}
			putchar(12);
		}
		else
			return;
	}
	
	
}

/*
* Used for RSA encryption and decryption
*/
char modPow(char in_val, int pow, char mod){
	char ret_val = 0x01;
	int i;
	for(i = pow; i > 0; i--){
		ret_val = ret_val * in_val;
		while(ret_val >= mod && ret_val >= 0){
			ret_val = ret_val - mod;
		}
		while(ret_val < 0){
			ret_val = ret_val + mod;
		}
	}
	return ret_val;
}

/*
* NOTE:
* Code is copied from the example code smartkey.c and with modifications
*/
void putchar( char d_out )  /* sends one char */
{
   char cypher_char = modPow(d_out, 43, 143);
   char bitCount, ti;
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
* Code is copied from the example code smartkey.c and with modifications
*/
char getchar( void )  /* recieves one char */
{
   /* One start bit, one stop bit, 8 data bit, no parity = 10 bit. */
   /* Baudrate: 9600 baud => 104.167 usec. per bit.                */
   char d_in, bitCount, ti;
   while( PORTC.2 == 1 )  /* wait for startbit */ ;
   /* delay 1,5 bit is 156 usec                         */ 
   /* 156 usec is 156 op @ 4 MHz ( 5+47*3-1+2+9=156)    */
   ti = 47; do ; while( --ti > 0); nop2();  
   for( bitCount = 8; bitCount > 0 ; bitCount--)
       {
        Carry = PORTC.2;
        d_in = rr( d_in);  /* rotate carry */
        /* delay 1 bit is 104 usec                       */ 
        /* 104 usec is 104 op @ 4 MHz (5+30*3-1+1+9=104) */  
        ti = 30; do ; while( --ti > 0); nop();  
        }
   return modPow(d_in, 43, 143);
}