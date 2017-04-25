/* password_bb.c  Check multiple passwords with PK2 UartTool */

/*
   Use "PICkit2 UART Tool" as a 9600 Baud terminal
   with BitBanging routines.
*/

/* B Knudsen Cc5x C-compiler - not ANSI-C */
#include "16F690.h"
#pragma config |= 0x00D4
#define MAX_STRING 11

void initserial( void );
void putchar( char );
char getchar( void );
void string_in( char * ); 
void printf(const char * string, char variable); 
bit check_password( char * input_string, const char * candidate_string );
char check_all_passwords( char * input_string );
void delay10( char );


void main( void)
{
   char num;
   char input_string[MAX_STRING]; /* buffer for input string */
   initserial();

   while(1)
    {
      delay10(250);  /* 2,5 sek pause */
      printf("Your password: ", 0);

      string_in( &input_string[0] );

      num = check_all_passwords( &input_string[0] );

      if(num) printf("\r\nNumber %d you may pass!\r\n", num);
      else printf("\r\nYou unknown, stay where you are!\r\n",0);
    }
}






/* *********************************** */
/*            FUNCTIONS                */
/* *********************************** */


bit check_password( char * input_string, const char * candidate_string )
{
   /* compares input buffer with the candidate string */
   char i, c, d;
   for(i=0; ; i++)
     {
       c = input_string[i];
       d = candidate_string[i];
       if(d != c ) return 0;       /* no match    */
         if( d == '\0' ) return 1; /* exact match */
     }
}

char check_all_passwords( char * input_string )
{
   if( check_password(input_string, "kalle") )      return 1;
   else if( check_password(input_string, "olle") )  return 2;
   else if( check_password(input_string, "nisse") ) return 3;
   else return 0;
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

char getchar( void )  /* recieves one char, blocking */
{
   /* One start bit, one stop bit, 8 data bit, no parity = 10 bit. */
   /* Baudrate: 9600 baud => 104.167 usec. per bit.                */
   char d_in, bitCount, ti;
   while( PORTA.1 == 1 ) /* wait for startbit */ ;
      /* delay 1,5 bit 156 usec at 4 MHz         */
      /* 5+28*5-1+1+2+9=156 without optimization */
      ti = 28; do ; while( --ti > 0); nop(); nop2();
   for( bitCount = 8; bitCount > 0 ; bitCount--)
       {
        Carry = PORTA.1;
        d_in = rr( d_in);  /* rotate carry */
         /* delay one bit 104 usec at 4 MHz       */
         /* 5+18*5-1+1+9=104 without optimization */ 
         ti = 18; do ; while( --ti > 0); nop(); 
        }
   return d_in;
}

void string_in( char * string ) 
{
   char charCount, c;
   for( charCount = 0; ; charCount++ )
       {
         c = getchar( );           /* input 1 character     */
         string[charCount] = c;    /* store the character   */
         putchar( c );             /* echo the character    */
         if( (charCount == (MAX_STRING-1))||(c=='\r' )) /* end of input */
           {
             string[charCount] = '\0'; /* add "end of string"      */
             return;
           }
       }
}

void printf(const char *string, char variable)
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

void delay10( char n)
{
    char i;  OPTION = 7;
    do  {
        i = TMR0 + 39; /* 256 microsec * 39 = 10 ms */
        while ( i != TMR0)  ;
    } while ( --n > 0);
}



/* *********************************** */
/*            HARDWARE                 */
/* *********************************** */


/*
             _____________  _____________ 
            |             \/             |
      +5V---|Vdd        16F690        Vss|---Gnd
            |RA5            RA0/AN0/(PGD)|bbTx->- PK2 UART-tool
            |RA4/AN3            RA1/(PGC)|bbRx-<- PK2 UART-tool
            |RA3/!MCLR/(Vpp)  RA2/AN2/INT|
            |RC5/CCP                  RC0|
            |RC4                      RC1|
            |RC3/AN7                  RC2|
            |RC6/AN8             AN10/RB4|
            |RC7/AN9               RB5/Rx|
            |RB7/Tx                   RB6|
            |____________________________|
                                          
*/

