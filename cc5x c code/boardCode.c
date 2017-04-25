int* passPhrase = {11, 13, 14, 15, 16, 21, 24, 27, 28, 29};
int* customerIDs = {95};
int* ownerIDs = {13};

void main(){
	initialize();
	while(1){
		communicationSequence();
	}
}

void initialize(){

}

void communicationSequence(){

}


/*
* NOTE:
* Code is copied from the example code smartkey.c
*/
void putchar( char d_out )  /* sends one char */
{
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
          d_out = rr( d_out ); /* Rotate Right through Carry     */
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
   return d_in;
}

/*
* NOTE:
* Code is copied from the example code smartkey.c
*/
void string_in( char * input_string )
{
   char charCount, c;
   for( charCount = 0; ; charCount++ )
       {
         c = getchar( );     /* input 1 character             */
         input_string[charCount] = c;  /* store the character */
         //putchar( c );     /* don't echo the character      */
         if( (charCount == (MAX_STRING-1))||(c=='\r' )) /* end of input   */
           {
             input_string[charCount] = '\0';  /* add "end of string"      */
             return;
           }
       }
}

/*
* NOTE:
* Code is copied from the example code smartkey.c
*/
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

/*
* NOTE:
* Code is copied from the example code smartkey.c
*/
bit check_candidate( char * input_string, const char * candidate_string )
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