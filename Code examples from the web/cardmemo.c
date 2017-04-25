/* cardmemo.c (for 16F84 GoldCard) stores and retrieves EEPROM-data */

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
   Baudrate:  9600 baud => 104.167 usec. per bit. Simplex.
*/

#include "16F84.h"
#define MAX_STRING 16
#pragma config |= 0x3ff1

/* Function prototypes  */
void putchar( char );
char getchar( void );
void putchar_eedata( char data, char EEPROMadress );
char getchar_eedata( char EEPROMadress ); 
void puts_eedata( char * );
void gets_eedata( char * );
void string_out( const char * string ); 
void string_in( char * );
void delay10( char );


void main( void)
{
   char i;
   char s[MAX_STRING];  /* char-array for temporary strings */
   s[0] = '\0';

   PORTB.7 = 1;  /* ready to send */
   TRISB.7 = 0;  /* Marking line  */
   delay10(100);

   string_out("\r\nWelcome! I recall from my memory: ");

   /* Get EEPROM-data and put it in s[] */
   gets_eedata( &s[0] );   

   /* print out textstring s[] */
   string_out( &s[0] );
  
   string_out("\r\nWhat Do you want me to remember now: ");


   TRISB.7 = 1;  /* ready to recieve */

   /* string input to string s[]     */
   string_in( &s[0] );

   /* Store string s[] in EEPROM-data */
   puts_eedata( &s[0] ); 

   PORTB.7 = 1;  /* ready to send */
   TRISB.7 = 0;  /* Marking line  */

   string_out("\r\nOk! I'll remember that. ");
   string_out("Now you can take out the card.\r\n");
   while(1) nop();
}



/************************
   Functions
   =========
*************************/

void putchar( char d_out )  /* sends one char */
{
   char bitCount, ti;
   TRISB.7 = 0; /* output mode */
   PORTB.7 = 0; /* set startbit */
   for ( bitCount = 10; bitCount > 0 ; bitCount-- )
        {         
         // ti = 27; do ; while( --ti > 0);
         // 104 usec at 3,58 MHz (5+27*3-1+9=104)
         /* 104 usec at 4 MHz (5+30*3-1+1+9=104)                 */
          ti = 30; do ; while( --ti > 0); nop();  
          Carry = 1;           /* stopbit                        */
          d_out = rr( d_out ); /* Rotate Right through Carry     */
          PORTB.7 = Carry;
        }
        nop2(); nop2();
   return; /* all done */
}


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
   return d_in;
}


void string_in( char * s )  /* get input to global string s[]     */
{
   char charCount, c;
   for( charCount = 0; ; charCount++ )
       {
         c = getchar( );     /* input 1 character             */
         s[charCount] = c;   /* store the character           */
         //putchar( c );     /* don't echo the character      */
         if( (charCount == (MAX_STRING-1))||(c=='\r' )) /* end of input   */
           {
             s[charCount] = '\0'; /* add "end of string"      */
             return;
           }
       }
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


void puts_eedata( char * s )
{
/* Put s[] -string in EEPROM-data */
char i, c;
for(i = 0;  ; i++) 
    {
      c = s[i];
      /* Write EEPROM-data sequence                          */
      EEADR = i;          /* EEPROM-data adress 0x00 => 0x40 */ 
      EEDATA = c;         /* data to be written              */
      WREN = 1;           /* write enable                    */
      EECON2 = 0x55;      /* first Byte in comandsequence    */
      EECON2 = 0xAA;      /* second Byte in comandsequence   */
      WR = 1;             /* write begin                     */
      while( EEIF == 0) ; /* wait for done ( EEIF=1 )        */
      WR = 0;             /* write done                      */
      WREN = 0;           /* write disable - safety first    */
      EEIF = 0;           /* Reset EEIF bit in software      */
      /* End of write EEPROM-data sequence                   */
      if(c == '\0') break;  
    }
}


void gets_eedata( char * s )
{
/* Get EEPROM-data and put it in s[] */
char i;
for(i = 0; ;i++)
    {
      /* Start of read EEPROM-data sequence                */
      EEADR = i;       /* EEPROM-data adress 0x00 => 0x40  */ 
      RD = 1;          /* Read                             */
      s[i] = EEDATA;   /* data to be read                  */
      RD = 0;
      /* End of read EEPROM-data sequence                  */  
      if( s[i] == '\0') break;
    }
}


/* MORE USEFUL FUNCTIONS */

char getchar_eedata( char EEPROMadress )
{
  /* Get char from specific EEPROM-adress */
      /* Start of read EEPROM-data sequence                */
      char temp;
      EEADR = EEPROMadress;  /* EEPROM-data adress 0x00 => 0x40  */ 
      RD = 1;                /* Read                             */
      temp = EEDATA;
      RD = 0;
      return temp;           /* data to be read                  */
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


