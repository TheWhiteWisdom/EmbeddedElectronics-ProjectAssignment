
/* B Knudsen Cc5x C-compiler - not ANSI-C */

#include "16F84A.H"
#pragma config |= 0x3ff1

/* 'Static' values */
#define RSA_key 11
#define RSA_mod 187
#define MAX_STRING 11
#define NMBR_PHRASE 3
#define Cmd_accessGranted 0x01
#define Cmd_accessDenied 0x02
#define Cmd_getID 0x03
#define Cmd_getPassPhrase 0x10 // + Pass phrase number
#define Cmd_getNmbrAccesses 0x04
#define Cmd_decrementAccesses 0x05
#define Cmd_loadCard 0x100  // + amount of accesses

/* Method headers */
void initializePhrases(void);
void initializeSerial(void);
void run(void);

/* Borrowed and modified method's headers */
void putchar( char );
char getchar( void );
void string_out( const char * string ); 
void string_in( char * );

char id;
char*** passPhrasesPointer;
char accessesAddress;

void main(){
    char** passPhrases[MAX_STRING][NMBR_PHRASE];
    *passPhrasesPointer = passPhrases;
    
    id = 42;
    accessesAddress = 14;
    
    initializePhrases();
    initializeSerial();
    while(1){
        run();
    }
}


void initializePhrases(){
    (*passPhrasesPointer)[0] = "uPhrase0";
    (*passPhrasesPointer)[1] = "uPhrase1";
    (*passPhrasesPointer)[2] = "uPhrase2";
}

void initializeSerial(){
    /* READER */
        PORTB.7 = 1; // marking line
        // RB7 I/O port. TRISB.7 set at putchar and getchar
}

void run(){    
    while(1){
        char command = getchar();
        if(command > Cmd_loadCard){
            char value = command - Cmd_loadCard;
            char accesses = gets_eedata(accessesAddress);
            accesses += value;
            puts_eedata(accesses, accessesAddress);
            continue;
        }
        if(command > Cmd_getPassPhrase){
            if(command-Cmd_getPassPhrase > NMBR_PHRASE || command-Cmd_getPassPhrase < 0)
                return;
            putchar((*passPhrasesPointer)[command-Cmd_getPassPhrase]);
            continue;
        }
        switch(command){
            case Cmd_accessGranted:
                continue;
            case Cmd_accessDenied:
                return;
            case Cmd_getID:
                putchar(id);
                break;
            case Cmd_decrementAccesses:
                char accesses = gets_eedata(accessesAddress);
                accesses--;
                puts_eedata(accesses, accessesAddress);
                break;
            case Cmd_getNmbrAccesses:
                putchar(gets_eedata(accessesAddress));
                break;
            default:
                putchar(0);
                break;
        }
    }
}

modpow(char val, char mod, char pow){
    char ret_char = 1;
    for(mod; mod > 0; mod--){
        ret_char = ret_char * val;
        /* Simple and fast char modulo */
        while(ret_char >= mod){
            ret_char -= mod;
        }
    }
    return ret_char;
}

char RSAcryptation(char c){
    return modpow(c, RSA_mod, RSA_key);
}

/* ***********************************************
    ----------------------- NOTE --------------------------------
    Following methods are borrowed and slightly modified!
    Origin: password_bb.c and smartkey.c in Code examples
                                by William Sandqvist
                                
        |       |       |       |       |       |       |       |       |
        V      V      V       V      V      V       V      V       V
    *********************************************** */

/* *********************************************
    Modification: Encrypted communication with smartcard 
    ********************************************* */
void putchar( char ch )  /* sends one char */
{
  TRISB.7 = 0;
  char cipher = RSAcryptation(ch);
  char bitCount, ti;
  PORTB.7 = 0; /* set startbit */
  for ( bitCount = 10; bitCount > 0 ; bitCount-- )
   {
     /* delay one bit 104 usec at 4 MHz       */
     /* 5+18*5-1+1+9=104 without optimization */ 
     ti = 18; do ; while( --ti > 0); nop(); 
     Carry = 1;     /* stopbit                    */
     cipher = rr( cipher ); /* Rotate Right through Carry */
     PORTB.7 = Carry;
   }
  return;
}

/* *********************************************
    Modification: Encrypted communication with smartcard 
    ********************************************* */
char getchar( void )  /* recieves one char, blocking */
{
   /* One start bit, one stop bit, 8 data bit, no parity = 10 bit. */
   /* Baudrate: 9600 baud => 104.167 usec. per bit.                */
   char d_in, bitCount, ti;
   TRISB.7 = 1; /* stops the feed to RB7 and tells that its ready for stream */
   while( PORTB.7 == 1 ) /* wait for startbit */ ;
      /* delay 1,5 bit 156 usec at 4 MHz         */
      /* 5+28*5-1+1+2+9=156 without optimization */
      ti = 28; do ; while( --ti > 0); nop(); nop2();
   for( bitCount = 8; bitCount > 0 ; bitCount--)
       {
        Carry = PORTB.7;
        d_in = rr( d_in);  /* rotate carry */
         /* delay one bit 104 usec at 4 MHz       */
         /* 5+18*5-1+1+9=104 without optimization */ 
         ti = 18; do ; while( --ti > 0); nop(); 
        }
    TRISB.7 = 0;
    PORTB.7 = 1; /* This side not ready for stream */
   return RSAcryptation(d_in);
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

void string_out(const char * string)
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

void puts_eedata( char c, char address ){
/* Put char in EEPROM-data on specified address */
      /* Write EEPROM-data sequence                          */
      EEADR = address;          /* EEPROM-data adress 0x00 => 0x40 */ 
      EEDATA = c;         /* data to be written              */
      WREN = 1;           /* write enable                    */
      EECON2 = 0x55;      /* first Byte in comandsequence    */
      EECON2 = 0xAA;      /* second Byte in comandsequence   */
      WR = 1;             /* write begin                     */
      while( EEIF == 0) ; /* wait for done ( EEIF=1 )        */
      WR = 0;             /* write done                      */
      WREN = 0;           /* write disable - safety first    */
      EEIF = 0;           /* Reset EEIF bit in software      */  
}

char gets_eedata(char address){
	/* Start of read EEPROM-data sequence                */
      EEADR = address;       /* EEPROM-data adress 0x00 => 0x40  */ 
      RD = 1;          /* Read                             */
      char ret_char = EEDATA;   /* data to be read                  */
      RD = 0;
	  return ret_char;
}