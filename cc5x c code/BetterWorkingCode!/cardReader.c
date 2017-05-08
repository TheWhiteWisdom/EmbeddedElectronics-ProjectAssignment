/* Code is based on smartlock.c */
/* smartlock.c  question and answer, compare strings and unlock */
/* B Knudsen Cc5x C-compiler - not ANSI-C                       */

#include "16F690.h"
#pragma config |= 0x00D4

#define MAX_STRING 31 /* string input max 30 characters */

/* Function prototypes */
void communicationSequence( void );
void initserial( void );
void  putchar( char );
char getchar( void );     /* Special! nonblocking when card removed  */
void printf(const char * string, char);
void putchar_eedata( char, char );
char getchar_eedata( char );            
void OverrunRecover(void);
void string_in( char * ); /* no extra echo, local echo by connection */
void string_out( const char * string );
bit check_candidate( char * input_string, const char * candidate_string );
void delay( char );

char address;

void main( void)
{
	delay(250); // Extra time to start the display
	delay(250);
	delay(250);
	
   TRISA.0 = 1; /* RA0 not to disturb PK2 UART Tool */
   ANSEL.0 = 0; /* RA0 digital input */
   TRISA.1 = 1; /* RA1 not to disturb PK2 UART Tool */
   ANSEL.1 = 0; /* RA1 digital input */
   initserial();

   TRISC.3 = 1;  /* RC3 card contact is input       */
   ANSEL.7 = 0;  /* RC3 digital input               */
   TRISC.2 = 0;  /* RC2 lock (lightdiode) is output */
   PORTC.2 = 0;  /* RC2 initially locked            */
	address = 0x52;
	putchar_eedata( 0, address );
	
	
	communicationSequence();
}


/********************
	Functions
	=======
********************/
char rand( void ){
	bit EXOR_out;
	static char rand_hi, rand_lo;
	if( !rand_hi && !rand_lo ) rand_lo = 0x01;
	EXOR_out = rand_lo.0;
	EXOR_out ^= rand_lo.2;
	EXOR_out ^= rand_lo.3;
	EXOR_out ^= rand_lo.5;
	Carry = EXOR_out;
	rand_hi = rr( rand_hi );
	rand_lo = rr( rand_lo );
	return rand_lo;
}

char sendPhrase(){
	char index = rand();
	index = index % 3;
	switch(index){
		case 0:
			string_out( "1. who is it?\r\n");
			break;
		case 1:
			string_out( "2. who is it?\r\n");
			break;
		default:
			string_out( "3. who is it?\r\n");
	}
	return index;
}

char getAndComparePhrase(char index){
	char input_string[MAX_STRING];
	bit compare;
	/* get the answer string from the card */
      string_in( &input_string[0] );
         
       /* Compare the answer string with the correct answer for master card */
	   switch(index){
			case 0:
				compare = check_candidate( &input_string[0], "1. me, the master card" );
				break;
			case 1:
				compare = check_candidate( &input_string[0], "2. me, the master card" );
				break;
			default:
				compare = check_candidate( &input_string[0], "3. me, the master card" );
	   }
	   if(compare == 1){
			return 'M';
	   }
	   /* Compare the answer string with the correct answer for user card */
	   switch(index){
			case 0:
				compare = check_candidate( &input_string[0], "1. me, the customer card" );
				break;
			case 1:
				compare = check_candidate( &input_string[0], "2. me, the customer card" );
				break;
			default:
				compare = check_candidate( &input_string[0], "3. me, the customer card" );
	   }
	   if(compare == 1){
			return 'C';
	   }
       return 0;
}

void communicationSequence(){
	char input_string[MAX_STRING];
	bit compare, refillMode;

	while(1){
		if( PORTC.3 == 0)
			string_out("Insert card!\r\n");
		while( PORTC.3 == 0) ; /* wait for card insertion */
       delay(100); /* card debounce */
       delay(50);  /* extra delay   */       

       /* ask the question */
	   char requestedPhrase = sendPhrase();
       

       delay(100); /* USART is buffered, so wait until all chars sent  */

       /* empty the reciever FIFO, it's now full with garbage */
       OverrunRecover();

	   char user = getAndComparePhrase(requestedPhrase);
	   
	   OverrunRecover();

	   if(user == 'M' || user == 'C'){
			string_out("Enter PIN:\r\n");
			delay(100); /* USART is buffered, so wait until all chars sent  */
			OverrunRecover();
			string_in( &input_string[0] ); // Read PIN (card does stuff with it)
			delay(100);
			OverrunRecover();
			string_in( &input_string[0] ); // Read response from card
			compare = check_candidate( &input_string[0], "PIN OK" );
			OverrunRecover();
			if(compare == 0 ){
				if(refillMode){
					refillMode = 0;
					OverrunRecover();
					delay(100);  /* extra delay */ 
					string_out("Refill mode disabled\r\n");
					OverrunRecover();
					delay(200);  /* extra delay */ 
				}
			}
			else if(compare == 1 && user == 'M'){
				refillMode = !refillMode;
				delay(100);
				if(refillMode == 1){
					string_out("Refill mode enabled\r\n");
				}
				else
					string_out("Refill mode disabled\r\n");
			}
			else if(compare == 1 && user == 'C'){
				char accesses = getchar_eedata( address );
				if(refillMode == 1){
					putchar_eedata( accesses+2, address );
					string_out("Refill done! \r");
					delay(50);
					OverrunRecover();
					printf("Number of accesses: %d\n", accesses+2);
					delay(50);
					OverrunRecover();
				}
				else if(accesses > 0){
					putchar_eedata( accesses-1, address );
					string_out("Welcome! \r");
					delay(50);
					OverrunRecover();
					printf("Number of accesses: %d\n", accesses-1);
					delay(50);
					OverrunRecover();
					PORTC.2 = 1;
				}
				else{
						delay(50);
						OverrunRecover();
						string_out("No accesses left!\r\n");
					}
				refillMode = 0;
			}
	   }
	   else
			string_out("Invalid card!\r\n");
			
        OverrunRecover();
       delay(100);  /* extra delay */ 
		if(PORTC.3 == 1)
			string_out("Remove card!\r\n");
       while( PORTC.3 == 1); /* wait for card removal */

       delay(10);

       PORTC.2 = 0;  /* lock again now when card is out  */
       delay(150);   /* card debounce */
	}
}
/********************
     Borrowed FUNCTIONS
     =========
*********************/

void initserial( void )  /* initialise PIC16F690 serialcom port */
{
   /* One start bit, one stop bit, 8 data bit, no parity. 9600 Baud. */

   TXEN = 1;      /* transmit enable                   */
   SYNC = 0;      /* asynchronous operation            */
   TX9  = 0;      /* 8 bit transmission                */
   SPEN = 1;

   BRGH  = 0;     /* settings for 6800 Baud            */
   BRG16 = 1;     /* @ 4 MHz-clock frequency           */
   SPBRG = 25;

   CREN = 1;      /* Continuous receive                */
   RX9  = 0;      /* 8 bit reception                   */
   ANSELH.3 = 0;  /* RB5 digital input for serial_in   */
}


void OverrunRecover(void)
{  
   char trash;
   trash = RCREG;
   trash = RCREG;
   CREN = 0;
   CREN = 1;
}

void putchar( char d_out )  /* sends one char */
{
   while (!TXIF) ;   /* wait until previus character transmitted   */
   TXREG = d_out;
   return; /* done */
}

char getchar( void )  /* recieves one char */
{
   char d_in = '\r';
   while ( !RCIF && PORTC.3 ) ;  /* wait for character or card removal */
   if(!RCIF) return d_in;
   d_in = RCREG;
   return d_in;
}

void string_in( char * string ) 
{
   char charCount, c;
   for( charCount = 0; ; charCount++ )
       {
         c = getchar( );        /* input 1 character         */
         string[charCount] = c; /* store the character       */
         // putchar( c );       /* don't echo the character  */
         if( (charCount == (MAX_STRING-1))||(c=='\r' )) /* end of input */
           {
             string[charCount] = '\0'; /* add "end of string" */
             return;
           }
       }
}


void string_out(const char * string )
{
  char i, k;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') return;   /* found end of string */
     putchar(k);
   }
  return;
}

bit check_candidate( char * input_string, const char * candidate_string )
{
   /* compares input with the candidate string */
   char i, c, d;
   for(i=0; ; i++)
     {
       c = input_string[i];
       d = candidate_string[i];
       if(d != c ) return 0;       /* no match    */
         if( d == '\0' ) return 1; /* exact match */
     }
}


void putchar_eedata( char data, char adress )
{
/* Put char in specific EEPROM-adress */
      /* Write EEPROM-data sequence                          */
      EEADR = adress;     /* EEPROM-data adress 0x00 => 0x40 */
      EEPGD = 0;          /* Data, not Program memory        */  
      EEDATA = data;      /* data to be written              */
      WREN = 1;           /* write enable                    */
      EECON2 = 0x55;      /* first Byte in comandsequence    */
      EECON2 = 0xAA;      /* second Byte in comandsequence   */
      WR = 1;             /* write                           */
      while( EEIF == 0) ; /* wait for done (EEIF=1)          */
      WR = 0;
      WREN = 0;           /* write disable - safety first    */
      EEIF = 0;           /* Reset EEIF bit in software      */
      /* End of write EEPROM-data sequence                   */
}


char getchar_eedata( char adress )
{
/* Get char from specific EEPROM-adress */
      /* Start of read EEPROM-data sequence                */
      char temp;
      EEADR = adress;  /* EEPROM-data adress 0x00 => 0x40  */ 
      EEPGD = 0;       /* Data not Program -memory         */      
      RD = 1;          /* Read                             */
      temp = EEDATA;
      RD = 0;
      return temp;     /* data to be read                  */
      /* End of read EEPROM-data sequence                  */  
}

void printf(const char * string, uns8 variable)
{
  char i, k, m, a, b;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') break;   // at end of string
     if( k == '%')           // insert variable in string
      { 
        i++;
        k = string[i];
        switch(k)
         {
           case 'd':         // %d  signed 8bit
             if( variable.7 ==1) putchar('-');
             else putchar(' ');
             if( variable > 127) variable = -variable;  // no break!
           case 'u':         // %u unsigned 8bit
             a = variable/100;
             putchar('0'+a); // print 100's
             b = variable%100; 
             a = b/10;
             putchar('0'+a); // print 10's
             a = b%10;         
             putchar('0'+a); // print 1's 
             break;
           case 'b':         // %b BINARY 8bit
             for( m = 0 ; m < 8 ; m++ )
              {
                if (variable.7 == 1) putchar('1');
                else putchar('0');
                variable = rl(variable);
               }
              break;
           case 'c':         // %c  'char'
             putchar(variable); 
             break;
           case '%':
             putchar('%');
             break;
           default:          // not implemented 
             putchar('!');   
         }   
      }
      else putchar(k); 
   }
}

void delay( char millisec)
/* 
  Delays a multiple of 1 milliseconds at 4 MHz
  using the TMR0 timer by B. Knudsen
*/
{
    OPTION = 2;  /* prescaler divide by 8        */
    do  {
        TMR0 = 0;
        while ( TMR0 < 125)   /* 125 * 8 = 1000  */
            ;
    } while ( -- millisec > 0);
}



/********************
      HARDWARE
      ========
*********************/

/*
           ___________  ___________ 
          |           \/           |
    +5V---|Vdd      16F690      Vss|---GND
          |RA5        RA0/AN0/(PGD)|-x ->- (PK2Rx)
          |RA4            RA1/(PGC)|-x -<- (PK2Tx)
    SW1---|RA3/!MCLR/(Vpp)  RA2/INT|
          |RC5/CCP              RC0|
          |RC4                  RC1|
  CrdIn->-|RC3                  RC2|->-LED (lock)
          |RC6                  RB4|
          |RC7               RB5/Rx|-<-SerIn
 SerOut-<-|RB7/Tx               RB6|
          |________________________|

Card Oscillator OSC 4 MHz ELFA 74-560-07

Card contact
                   __ __ __
              +5V |C1|   C5| GND
                  |__|   __|
        !MCLR/PGM |C2|  |C6| 
                  |__|  |__|
          OSC/PGC |C3|  |C7| IO/PGD 
                  |__|  |__|
                  |C4|  |C8|
                  |__|__|__|
                Contact CrdIn
				
All communications are tied together:

                  +5
                   |    --K<A-- = diodes
                  10k
                   |
           CrdIO---+--1k---(PK2Rx)
                   +--A>K--(PK2Tx)
      SerOut--K<A--+-------SerIn
*/
