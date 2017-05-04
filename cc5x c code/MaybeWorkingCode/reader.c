/*
    Use "PICkit2 UART Tool" as a 9600 Baud terminal
    with BitBanging routines.

*/

/* B Knudsen Cc5x C-compiler - not ANSI-C */

#include "16F690.H"
#pragma config |= 0x00D4

/* 'Static' values */
#define RSA_key 17
#define RSA_mod 187
#define MAX_STRING_SIZE 11
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
void incrementPhraseRequestNumber(void);
char modpow(char val, char mod, char pow);
char getAmountFromUART();

/* Borrowed and modified method's headers */
bit check_password( char * input_string, const char * candidate_string );
void OverrunRecover();
void putcharUART( char );
char getcharUART( void );
void string_outUART( const char * string ); 
void string_inUART( char * );
void putcharCARD( char );
char getcharCARD( void );
void string_outCARD( const char * string ); 
void string_inCARD( char * );
void printf(const char * string, char variable); 
void delay10( char );

/* Variables that simulates data from database */
char userID;
char superID;

/* Other variables */
char phraseRequestNumber;
bit refillModeEnabled;
char refillAmount;
char lightFrame;

void main(){
    userID = 42;
    superID = 53;
    
    phraseRequestNumber = 1;
    refillModeEnabled = 0;
    refillAmount = 0;
    
	lightFrame = 0;
	
    initializeSerial();
    while(1){
        run();
    }
}

void initializeSerial(){
    /* UART */
        PORTA.0 = 1; // marking line
        TRISA.0 = 0; // RA0 output
		ANSEL.0 = 0; // No AD on RA0
        TRISA.1 = 1; // RA1 input
		ANSEL.1 = 0; // No AD on RA1
    
    /* CARD */
		TXEN = 1;
		SYNC = 0;
		TX9 = 0;
		SPEN =1;
		
		BRGH = 0;
		BRG16 = 1;
		SPBRG = 25;
		
		CREN = 1;
		RX9 = 0;
		ANSELH.3 = 0;
	
       // PORTB.7 = 1; // marking line
       // TRISB.7 = 0; // RB7 output
       // TRISB.5 = 1; // RB5 input
        TRISC.3 = 1; // RC3 input, Card present bit
	    ANSEL.7 = 0; // RC3 digital input
		TRISC.2 = 0; // LED OUT
		PORTC.2 = 0;
		
}

bit comparePhrase(char* in_phrase, bit userPhrase, char index){
    if(userPhrase){
        switch(index){
            case 0:
                return check_password(in_phrase, "uPhrase0");
                break;
            case 1:
                return check_password(in_phrase, "uPhrase1");
                break;
            case 2:
                return check_password(in_phrase, "uPhrase2");
                break;
        }
    }else{
        switch(index){
            case 0:
                return check_password(in_phrase, "sPhrase0");
                break;
            case 1:
                return check_password(in_phrase, "sPhrase1");
                break;
            case 2:
                return check_password(in_phrase, "sPhrase2");
                break;
        }
    }
	return 0;
}

void flashLight(){
	lightFrame++;
	if(lightFrame>=100){
		lightFrame = 0;
		PORTC.2 = !PORTC.2;
	}
}

void run(){
    if(PORTC.3 == 1){
        string_outUART("Remove card\r\n");
        while(PORTC.3 == 1)flashLight();
        delay10(80);
    }
    if(refillModeEnabled){
        printf("Insert customer card to refill with %d accesses (or super card to change amount)", refillAmount);
    }
    else
        string_outUART("Insert card\r\n");
	OverrunRecover();
    while(PORTC.3 == 0);//nop();// wait for card to be present
	string_outUART("Card detected!\r\n");
	OverrunRecover();
	delay10(150);
	delay10(50);
	char ID = 'E';
    putcharCARD(Cmd_getID);
    ID = getcharCARD();
    if(ID == userID){
            if(!refillModeEnabled){
                string_outUART("User card detected!\r\n");
                putcharCARD(Cmd_getPassPhrase+phraseRequestNumber);
				char phrase[MAX_STRING_SIZE];
                string_inCARD(phrase);
                if(!comparePhrase(phrase, 0, phraseRequestNumber)){
                    putcharCARD(Cmd_accessDenied);
                    string_outUART("ERROR: invalid card!\r\n");
					string_outUART("Phrase gotten: ");
					string_outUART(phrase);	
					string_outUART("\r\n");
                    incrementPhraseRequestNumber();
                    return;
                }
                incrementPhraseRequestNumber();
                putcharCARD(Cmd_getNmbrAccesses);
                char nmbrAccesses = getcharCARD();
                if(nmbrAccesses <= 0){
                    putcharCARD(Cmd_accessDenied);
                    string_outUART("No accesses left!\r\n");
                    return;
                }
                putcharCARD(Cmd_accessGranted);
                putcharCARD(Cmd_decrementAccesses);
                printf("Granted! Accesses left: %d\r\n",nmbrAccesses-1);
                delay10(200);
                }
            else{
                string_outUART("User card detected!\r\n");
                putcharCARD(Cmd_getPassPhrase+phraseRequestNumber);
                char phrase[MAX_STRING_SIZE];
                string_inCARD(phrase);
                if(!comparePhrase(phrase, 0, phraseRequestNumber)){
                    putcharCARD(Cmd_accessDenied);
                    putcharCARD(Cmd_accessDenied);
                    string_outUART("ERROR: invalid card!\r\n");
					string_outUART("Phrase gotten: ");
					string_outUART(phrase);	
					string_outUART("\r\n");
                    refillAmount = 0;
                    refillModeEnabled = 0;
                    return;
                }
                printf("Granted! Adding %d accesses ...\r\n",refillAmount);
                putcharCARD(Cmd_accessGranted);
                delay10(50);
                putcharCARD(Cmd_loadCard + refillAmount);
                delay10(50);
                putcharCARD(Cmd_getNmbrAccesses);
                char nmbrAccesses = getcharCARD();
                printf("Done! Accesses left: %d\r\n",nmbrAccesses);
                delay10(200);
            }
	}else if(ID == superID){
            string_outUART("Super card detected!\r\n");
            putcharCARD(Cmd_getPassPhrase+phraseRequestNumber);
            char phrase[MAX_STRING_SIZE];
                string_inCARD(phrase);
            if(!comparePhrase(phrase, 0, phraseRequestNumber)){
                putcharCARD(Cmd_accessDenied);
                string_outUART("ERROR: invalid card!\r\n");
				string_outUART("Phrase gotten: ");
				string_outUART(phrase);	
				string_outUART("\r\n");
                refillAmount = 0;
                refillModeEnabled = 0;
                incrementPhraseRequestNumber();
                return;
            }
            incrementPhraseRequestNumber();
            putcharCARD(Cmd_accessGranted);
            string_outUART("Granted!\r\n");
            char refillAmount = getAmountFromUART();
            if(refillAmount==0)
                refillModeEnabled=0;
     }else{
            string_outUART("ERROR: invalid card!\r\n");
			string_outUART("ID gotten: ");
			putcharUART(ID);	
			string_outUART("\r\n");
			string_outUART("Valid IDs: ");
			putcharUART(userID);	
			string_outUART(" AND ");
			putcharUART(superID);	
			string_outUART("\r\n");
    }     
}

void incrementPhraseRequestNumber(){
    phraseRequestNumber = modpow(phraseRequestNumber + 1, NMBR_PHRASE, 1);
}

char modpow(char val, char mod, char pow){
    char ret_char = 1;
    for(mod = mod; mod > 0; mod--){
        ret_char = ret_char * val;
        /* Simple and fast char modulo */
        while(ret_char >= mod){
            ret_char -= mod;
        }
    }
    return ret_char;
}

char RSAcryptation(char c){
    return c;//modpow(c, RSA_mod, RSA_key);
}

char getAmountFromUART(){
	char value = 0;
    while(1){
        string_outUART("Insert amount to refill: ");
        char input_string[MAX_STRING_SIZE];
        string_inUART( &input_string[0] );
        string_outUART("\r\n");
        
        int i;
        for(i=0; i < 4; i++){
            if(i == 0 && input_string[i] == '-')
                continue;
            if(input_string[i] == '\0')
                return value;
            value = value * 10;
            char digit = input_string[i] - '0';
            if(digit > 9)
                break;
            if(input_string[0] == '-')
                value -= digit;
            else
                value += digit;
        }
        string_outUART("Must be numeric between -99 and 99! \r\n");
    }
    return value;
}

/* ***********************************************
    ----------------------- NOTE --------------------------------
    Following methods are borrowed and slightly modified!
    Origin: password_bb.c and smartkey.c in Code examples
                                by William Sandqvist
                                
        |       |       |       |       |       |       |       |       |
        V      V      V       V      V      V       V      V       V
    *********************************************** */
    
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
    
void OverrunRecover(){
	char trash;
	trash = RCREG;
	trash = RCREG;
	CREN = 0;
	CREN = 1;
}
	
void putcharUART( char ch )  /* sends one char */
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

char getcharUART( void )  /* recieves one char, blocking */
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

/* *********************************************
    Modification: Encrypted communication with smartcard 
    ********************************************* */
void putcharCARD( char ch )  /* sends one char */
{
  char cipher = RSAcryptation(ch);
  while(!TXIF); // Wait until previous character is transmitted
  TXREG = cipher;
  return;
}

/* *********************************************
    Modification: Encrypted communication with smartcard 
    ********************************************* */
char getcharCARD( void )  /* recieves one char, blocking */
{
	char d_in = 'E';
	while( !RCIF && PORTC.3 );
	if(!RCIF){ 
		string_outUART("ERROR: Card ejected before completion!\r\n");
		return d_in;
	}
	d_in = RCREG;
	return RSAcryptation(d_in);
}

void string_inUART( char * string ) 
{
   char charCount, c;
   for( charCount = 0; ; charCount++ )
       {
         c = getcharUART( );           /* input 1 character     */
         string[charCount] = c;    /* store the character   */
         putcharUART( c );             /* echo the character    */
         if( (charCount == (MAX_STRING_SIZE-1))||(c=='\r' )) /* end of input */
           {
             string[charCount] = '\0'; /* add "end of string"      */
             return;
           }
       }
}

void string_outUART(const char * string)
{
  char i, k;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') return;   /* found end of string */
     putcharUART(k); 
   }
  OverrunRecover();
  return;
}

void string_inCARD( char * string ) 
{
   char charCount, c;
   for( charCount = 0; ; charCount++ )
       {
         c = getcharCARD( );           /* input 1 character     */
         string[charCount] = c;    /* store the character   */
         putcharCARD( c );             /* echo the character    */
         if( (charCount == (MAX_STRING_SIZE-1))||(c=='\r' )) /* end of input */
           {
             string[charCount] = '\0'; /* add "end of string"      */
             return;
           }
       }
}

void string_outCARD(const char * string)
{
  char i, k;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') return;   /* found end of string */
     putcharCARD(k); 
   }
  return;
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
             if( variable.7 ==1) putcharUART('-');
             else putcharUART(' ');
             if( variable > 127) variable = -variable;  // no break!
           case 'u':         // %u unsigned 8bit
             a = variable/100;
             putcharUART('0'+a); // print 100's
             b = variable%100; 
             a = b/10;
             putcharUART('0'+a); // print 10's
             a = b%10;         
             putcharUART('0'+a); // print 1's 
             break;
           case 'b':         // %b BINARY 8bit
             for( m = 0 ; m < 8 ; m++ )
              {
                if (variable.7 == 1) putcharUART('1');
                else putcharUART('0');
                variable = rl(variable);
               }
              break;
           case 'c':         // %c  'char'
             putcharUART(variable); 
             break;
           case '%':
             putcharUART('%');
             break;
           default:          // not implemented 
             putcharUART('!');   
         }   
      }
      else putcharUART(k); 
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
