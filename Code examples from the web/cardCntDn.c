/* cardCntDn.c (for 16F84 SmartCard) countdowns EEPROM-data */

/* B Knudsen Cc5x C-compiler - not ANSI-C */

#include "16F84.h"
#pragma config |= 0x3ff1


#define MAX_STRING 11

void putchar( char );
void chartoa(char, char * string );
void string_out( const char * string ); 
void putchar_eedata( char data, char EEPROMadress );
char getchar_eedata( char EEPROMadress );            
void delay10( char );

void main( void)
{
   char CountDownValue;
   char string[MAX_STRING];  /* char-array for temporary strings */
   string[0] = '\0';

   PORTB.7 = 1;  /* ready to send */
   TRISB.7 = 0;  /* Marking line  */
   delay10(100);

   /* Get EEPROM-data stored in adress 0 */
   CountDownValue = getchar_eedata( 0 ); 

   /* print out char variable                                             */
   chartoa(CountDownValue, &string[0] );  /* convert number to textstring */ 
   string_out("\r\nWelcome! Countdown is now: ");
   string_out( &string[0] ); 

   CountDownValue-- ;

   /* Store new EEPROM-data at adress 0 */
   putchar_eedata( CountDownValue, 0 ); 

   string_out("\r\nNow you can remove the card.\r\n");
   while(1) nop();
}





/* *********************************** */
/*            FUNCTIONS                */
/* *********************************** */


void delay10( char n)
/*
  Delays a multiple of 10 milliseconds using the TMR0 timer
  Clock : 4 MHz   => period T = 0.25 microseconds
  1 IS = 1 Instruction Cycle = 1 microsecond
  error: 0.16 percent. B Knudsen.
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


void putchar( char d_out )  /* sends one char */
{
   char bitCount, ti;
   TRISB.7 = 0; /* output mode */
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

void string_out(const char * string)
{
  char i, k;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') return;   // found end of string
     putchar(k); 
   }
  return;
}

char getchar_eedata( char EEPROMadress )
{
/* Get char from specific EEPROM-adress */
      /* Start of read EEPROM-data sequence                */
      char temp;
      EEADR = EEPROMadress;  /* EEPROM-data adress 0x00 => 0x40  */ 
      RD = 1;                /* Read                             */
      temp = EEDATA;
      RD = 0;
      return temp;          /* data to be read                   */
      /* End of read EEPROM-data sequence                        */  
} 



void putchar_eedata( char data, char EEPROMadress )
{
/* Put char in specific EEPROM-adress */
      /* Write EEPROM-data sequence                          */
      EEADR = EEPROMadress; /* EEPROM-data adress 0x00 => 0x40 */ 
      EEDATA = data;        /* data to be written              */
      WREN = 1;             /* write enable                    */
      EECON2 = 0x55;        /* first Byte in comandsequence    */
      EECON2 = 0xAA;        /* second Byte in comandsequence   */
      WR = 1;               /* write                           */
      while( EEIF == 0) ;   /* wait for done (EEIF=1)          */
      WR = 0;
      WREN = 0;             /* write disable - safety first    */
      EEIF = 0;             /* Reset EEIF bit in software      */
      /* End of write EEPROM-data sequence                     */
}


/* chartoa:  convert "char" unsigned 8 bit variable c 
   to ASCII-characters in string s[]                  */
void chartoa(char c, char * string )
{
char i,temp;
string[3]='\0'; /* end of string */
for (i = 2; ;i--)
   {
    temp = c % 10; /* the remainder               */
    temp += '0';   /* ASCII-code of the remainder */
    string[i]=temp;     /* store  */
    if (i==0) break;
    c /= 10;       /* the whole number quotient   */
   }
}



/* *********************************** */
/*            HARDWARE                 */
/* *********************************** */


/*
SmartCard contact

             __ __ __
       +5V  |C1|   C5| Gnd
            |__|   __|
   MCLR/PGM |C2|  |C6| 
            |__|  |__|
    OSC/PGC |C3|  |C7| RB7/PGD I/O -><- Txd/Rxd half duplex 
            |__|  |__|
            |C4|  |C8|
            |__|__|__|

*/



/*
   SERIAL COMMUNICATION (RS232)
   ============================
   One start bit, one stop bit, 8 data bit, no parity = 10 bit.
   Baudrate:  9600 baud => 104.167 usec. per bit. Simplex.
*/

