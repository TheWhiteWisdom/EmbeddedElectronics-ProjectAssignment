/* countdown_bb.c (for 16F690) countdowns EEPROM-data */
/* B Knudsen Cc5x C-compiler - not ANSI-C */

#include "16F690.h"
#pragma config |= 0x00D4

void initserial( void );
void putchar( char );
void printf(const char * string, char);
void putchar_eedata( char, char );
char getchar_eedata( char );            
void delay10( char );

void main( void)
{
   char CountDownValue;

   initserial();
   delay10(100);

   /* Get EEPROM-data stored in adress 0 */
   CountDownValue = getchar_eedata( 0 ); 
   printf("\r\nWelcome! Countdown is now: %u", CountDownValue );

   CountDownValue-- ;

   /* Store new EEPROM-data at adress 0 */
   putchar_eedata( CountDownValue, 0 ); 
   printf("\r\nNow you can turn off the power.\r\n", 0 );

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


void initserial( void )  /* initialise PIC16F690 serialcom port */
{
   ANSEL.0 = 0; /* No AD on RA0             */
   ANSEL.1 = 0; /* No AD on RA1             */
   PORTA.0 = 1; /* marking line             */
   TRISA.0 = 0; /* output to PK2 UART-tool  */
   TRISA.1 = 1; /* input from PK2 UART-tool */
   return;      
}

void putchar( char ch )  /* sends one char */
{
  char bitCount, ti;
  PORTA.0 = 0; /* set startbit */
  for ( bitCount = 10; bitCount > 0 ; bitCount-- )
   {
     /* delay one bit 104 usec at 4 MHz       */
     /* 5+18*5-1+1+9=104 without optimization */ 
     ti = 18; do ; while( --ti > 0); nop(); 
     Carry = 1;     /* stopbit                    */
     ch = rr( ch ); /* Rotate Right through Carry */
     PORTA.0 = Carry;
   }
  return;
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



/* *********************************** */
/*            HARDWARE                 */
/* *********************************** */



/*
   Use "PICkit2 UART Tool" as a 9600 Baud terminal.
   Uncheck "Echo On".
   PIC internal USART is not used. BitBanging routines.
   No extra connections are needed.
           ___________  ___________ 
          |           \/           | 
    +5V---|Vdd      16F690      Vss|---GND
          |RA5        RA0/AN0/(PGD)|bbTx ->- PK2Rx
          |RA4            RA1/(PGC)|bbRx -<- PK2Tx
          |RA3/!MCLR/(Vpp)  RA2/INT|
          |RC5/CCP              RC0|
          |RC4                  RC1|
          |RC3                  RC2|
          |RC6                  RB4|
          |RC7               RB5/Rx|not used
  not used|RB7/Tx               RB6|
          |________________________|
                                    
*/

