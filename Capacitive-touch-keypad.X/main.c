/* 
 * File:   main.c
 * Author: Admin
 *
 * Created on November 17, 2014, 1:03 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
//#include <pic.h>
#include "string.h"
#include <pic16f1936.h>
#include <htc.h>

typedef unsigned int        UINT;
typedef unsigned char       UINT8;
typedef unsigned short int  UINT16;
typedef unsigned long int   UINT32;

__CONFIG(FOSC_INTOSC & WDTE_ON & PWRTE_OFF & MCLRE_OFF & CP_OFF & CPD_OFF & BOREN_ON & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
__CONFIG(WRT_OFF & VCAPEN_OFF & PLLEN_ON & STVREN_ON & BORV_LO & LVP_OFF);

#define KEYSTATPIN  LATCbits.LATC5
#define capsense_avg_cnt 8

persistent UINT32 threshold[8];
persistent UINT8 tholdflg ,pwrstat;

UINT16 debounc[8];

UINT32 raw_reading,cur_read[8],average[8];//max;
signed long int curdiff[8],olddiff[8],diff[8],max;
UINT8 channelno;
UINT8 count,pos;
UINT8 avgflag;
UINT8 i2c_rx_buff[8];
UINT8 i2c_tx_buff[8];
UINT8 keystat[8],keycnt[8];
void capsensinit(void);
void interrupt isr(void);

void setscanchannel()
{
    if(channelno==0){CPSCON1 = 0b00000000;}
    else if(channelno==1){CPSCON1 = 0b00000001;}
    else if(channelno==2){CPSCON1 = 0b00000010;}
    else if(channelno==3){CPSCON1 = 0b00000011;}
    else if(channelno==4){CPSCON1 = 0b00000100;}
    else if(channelno==5){CPSCON1 = 0b00000101;}
    else if(channelno==6){CPSCON1 = 0b00000110;}
    else if(channelno==7){CPSCON1 = 0b00000111;}
}

void capsensinit(void)
{
    LATB = 0;
    TRISB  |= 0b00111111;//0b00001110;
    ANSELB |= 0b00111111;//0b00001110;                     // Analog inputs - all CPS pins  RB123

    TRISA  |= 0b00110000;//0b00001110;
    ANSELA |= 0b00110000;//0b00001110;

    //OPTION_REG = 0x20;			     // setup the timer0 prescaler (3 LSb's)

    ADCON0     = 0;//0b00111101;						// Select 1.2V reference, A/D on
    ADCON1     = 0;//0b01010000;						// Select Clock / 16 for A/D clock
    CPSCON0    = 0b10001101;						// Turn Cap Sense on, high range (18 uA), Timer 0 is incremented for period measurement
    CPSCON1    = 0;//0b00000001;						// Start with channel
    //OPTION_REG = 0xC3; // fosc/4, hi-lo edge transition, 1:16 prescaler
// EXAMPLE TIMER INITIALIZATION
    // Only an 8-bit timer may be used as the mTouch framework timer.
    // TMR1/3/5 are not currently able to be used for this purpose.
    OPTION_REG = 0b11101000;        //OPTION_REG  = 0b10000111;//00;   // TMR0 Prescaler  = 1:2
    TMR0IF = 0; // clear TMR0 interrupt flag
    TMR0IE = 1; // enable TMR0 interrupt
    T1CON = 0b01000001;//0xC5; // Timer1 initialization
    TMR1IF =0;
    TMR1IE =1;
    T1GCON = 0xE1; // Timer1 gate init /Toggle Mode/TMR0 time base
    TMR1GIF = 0; // Clear Gate Interrupt Flag
    TMR1GIE = 1; // Enable Gate Interrupt
    GIE = 1;
}


void usartinit(void)
{
    TRISCbits.TRISC6 = 0; // RC6 = TX out
    TRISCbits.TRISC7 = 1; // RC7 = RX in

    TXSTAbits.BRGH=0;       // select low speed Baud Rate (see baud rate calcs below)
    TXSTAbits.TX9=0;        // select 8 data bits
    TXSTAbits.TXEN = 1;     // enable transmit


    RCSTAbits.SPEN=1;       // serial port is enabled
    RCSTAbits.RX9=0;        // select 8 data bits
    RCSTAbits.CREN=1;       // receive enabled

    // calculate values of SPBRGL and SPBRGH based on the desired baud rate
    //
    // For 8 bit Async mode with BRGH=0: Desired Baud rate = Fosc/64([SPBRGH:SPBRGL]+1)
    // For 8 bit Async mode with BRGH=1: Desired Baud rate = Fosc/16([SPBRGH:SPBRGL]+1)
    // For our example,we will use BRGH=0,Fosc=16Mhz and we want baud rate=9600
    //
    //  9600 = Fosc/64([SPBRGH:SPBRGL]+1)
    //  9600 = Fosc/64(X+1)
    //  9600 = Fosc/64X + 64
    //  9600(64X + 64) = Fosc
    //  X = [Fosc/(9600)(64)]-1
    //  X = [16000000/(9600)(64)] -1
    //  X = SPBRGH:SPBRGL = 25.01 (round to 25)

    SPBRGL=25;  // here is calculated value of SPBRGH and SPBRGL
    SPBRGH=0;

    PIR1bits.RCIF=0;        // make sure receive interrupt flag is clear
    PIE1bits.RCIE=0;//1;        // enable UART Receive interrupt
   // INTCONbits.PEIE = 0;//1;    // Enable peripheral interrupt
   //INTCONbits.GIE = 1;     // enable global interrupt
}

void USARTWriteChar(char ch)
{
  //while(!PIR1bits.TXIF);
  while(!TXSTAbits.TRMT);
  TXREG=ch;
}

void USARTWriteString(const char *str)
{
  while(*str!='\0')
  {
      USARTWriteChar(*str);
      str++;
  }
}

void USARTWriteInt(int val, int field_length)
{
    char str[5]={0,0,0,0,0};
    int i=4,j=0;
    //Handle negative integers
    if(val<0)
    {
        USARTWriteChar('-');   //Write Negative sign
        val=val*-1;     //convert to positive
    }
    else
    {
        USARTWriteChar(' ');
    }

    if(val==0 && field_length<1)
    {
        USARTWriteChar('0');
        return;
    }
    while(val)
    {
        str[i]=val%10;
        val=val/10;
        i--;
    }

    if(field_length==-1)
        while(str[j]==0) j++;
    else
        j=5-field_length;

    for(i=j;i<5;i++)
    {
        USARTWriteChar('0'+str[i]);
    }
}

void I2Cinit(void)
{
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;

    SSPSTAT |= 0xc0;
    SSPCON1 |= 0x36;
    SSPCON2 |= 0x80;
    SSPCON3 |= 0x38;
    SSPMSK = 0xfe;
    SSPADD = 0x0e;

    SSPCON1bits.SSPEN = 1;
    SSPIF = 0;
    SSPIE = 1;
    PEIE = 1;
    //GIE = 1;
}

void main(void)
{
    UINT8 i=0,j=0,k,t;//togflg = 1;

    OSCCON = 0b01111000;
    OSCTUNE = 0b00000000;
    SWDTEN = 0b01;
    UINT8 tholdcount;
    ANSELA = 0;
    ANSELB = 0;
    TRISA = 0;
    TRISB = 0;
    TRISC = 0;
    WDTCONbits.WDTPS = 0b01010;

    for ( j = 0 ; j < 8 ; j++ )
    {
        debounc[j] = 0;
        threshold[j] = 0;
        cur_read[j] = 0;
        average[j] = 0;
        keystat[j]= 0;keycnt[j]= 0;
    }
    TRISCbits.TRISC5 = 0; // RC6 = TX out
    KEYSTATPIN = 1;
    capsensinit(); //Delay(5);
    setscanchannel();
    channelno = 0;avgflag = 0;
    count = 0;
    usartinit();
    I2Cinit();
    if(STATUSbits.nTO == 0){ USARTWriteString(" reset ");}
    //else {tholdflg = 0;}
    //tholdflg = 0;
    tholdcount=0;
    max = 500; pos = 9;
  //  TRISAbits.TRISA2 = 0;
  //  LATAbits.LATA2 = 1;
/*       while(1)
        {
          USARTWriteChar('U');
        }
*/
        while(1)
        {
            CLRWDT();
    /*        if(togflg)
            {
                LATAbits.LATA2 = 0;//togflg;
                togflg = 0;
            }
            else {
                LATAbits.LATA2 = 1;//togflg;
                togflg = 1;
            }
*/
		if(avgflag)
		{
			for( i = 0 ; i < 8 ; i++ )
			{
				average[i] /= capsense_avg_cnt;
				/*USARTWriteInt(average[i],5);
                    		USARTWriteChar('\t');
				if(i == 7) { USARTWriteChar('\n');USARTWriteChar('\r');}*/

                                if(tholdflg == 0)
                                {
                                    if(tholdcount == 8) threshold[i] = 0;
                                    threshold[i] += average[i];
                                    if ( i == 7 ) tholdcount++ ;
                                    if( tholdcount == 16 )
                                    {
                                        for ( j = 0 ; j < 8 ; j++ )
                                            threshold[ j ] >>= 3 ;
                                        tholdflg = 1;
                                    }
                                }
                                else
                                {
                                    olddiff[i] = curdiff[i];
                                    curdiff[i] = threshold[i] - average[i];
                                    if ( curdiff[i] > olddiff[i] )
                                        diff[i]= curdiff[i] - olddiff[i];
                                    else
                                        diff[i]= olddiff[i]- curdiff[i];
                                    if( diff[i] > max ) { max = diff[i]; pos = i; }
                                    //if( curdiff[i] > max ){ max = curdiff[i]; pos = i;}

                                    //USARTWriteInt(diff[i],5);
                    		    //USARTWriteChar('\t');
				    if(i == 7) 
                                    {
                                        //USARTWriteInt(pos,2);
                                        //USARTWriteChar(' ');
                                        //USARTWriteInt(diff[pos],5);
                                        //USARTWriteChar('\n');USARTWriteChar('\r');

                                        for(t = 0; t < 8 ; t++)
                                        {
                                            if(t != pos) {if(keycnt[t])keycnt[t]--;keystat[t] =0;}
                                            else if( max > 600 )
                                            {
                                                keycnt[t]++;
                                                if( keycnt[t] > 7 )
                                                {
                                                    for(k = 0 ; k < 8 ; k++) keycnt[k] = 0;
                                                    keystat[t] = 1;
                                                }
                                            }
                                            else{ if(keycnt[t])keycnt[t]-- ; keystat[t] = 0;}
                                            USARTWriteInt(keycnt[t],1);
                                        }
                                        
                                        if( pos != 9 && debounc[pos] == 0 )
                                        {
                                            debounc[pos] = 400;
                                            USARTWriteChar('\n');
                                            USARTWriteString("Key Detected = ");
                                            i2c_tx_buff[ 0 ] = pos;
                                            KEYSTATPIN = 0;
                                        }
                                            USARTWriteInt(pos,2);
                                            //i2c_tx_buff[ 0 ] = pos;
                                            USARTWriteChar('\n');USARTWriteChar('\r');
                                            
                                            max = 500; pos = 9;

                                    }
                                    //USARTWriteInt(keycnt[i],1);
                                    //USARTWriteChar('\t');
                                    //if(i == 7){USARTWriteChar('\n');USARTWriteChar('\r');}
                                    //pos = i;
                                    /*USARTWriteInt(curdiff[i],5);
                    		    USARTWriteChar('\t');
				    if(i == 7) { USARTWriteChar('\n');USARTWriteChar('\r');}
                                    if(curdiff[i] > 800){keycnt[i]++;}
                                    else if(curdiff[i] < 800){ keycnt[i]=0; keystat[i] = 0;}
                                    if(keycnt[i] == 8) keystat[i] = 1;*/
                                    /*USARTWriteInt(keystat[i],1);
                    		    USARTWriteChar('\t');
				    if(i == 7) { USARTWriteChar('\n');USARTWriteChar('\r');}*/
                                                                      
                                }

			}

		        avgflag = 0 ;
		}


	}


}


void interrupt isr(void)
{
  UINT8 j;
  UINT8 sspstat, buff_idx,no;
  
   for(no = 0 ; no < 8 ;no++)
   {
       if(debounc[no])
       debounc[no]= debounc[no]-1;
   }
  if (TMR0IE && TMR0IF)			// Overflow interrupt
      TMR0IF = 0;			// Clear T0IF every time

  if (TMR1GIF && TMR1GIE)
  {			// Gate interrupt
	TMR1GIF = 0;
        TMR0 = 0;
        TMR1ON = 0;
        raw_reading = TMR1L + (unsigned int)(TMR1H << 8);
        
        cur_read[channelno] = cur_read[channelno] + raw_reading;
        channelno++;

        if ( channelno == 8 ) { channelno = 0; count++; }
        setscanchannel();
        TMR1L   = 0x00;					// Reset Timer1
        TMR1H   = 0x00;					//   "     "
        TMR1ON  = 1;					// Restart Timer1
        //TMR0    = 0xFE;
        TMR0    = 0;
        T1GGO   = 1;
        if (count == capsense_avg_cnt)
        {
            count = 0;
            for(j = 0 ; j < 8; j ++)
            {
                average[j] = cur_read[j] ;
                cur_read[j] = 0;
            }
            avgflag = 1;
        }
  }

    if (SSPIF && SSPIE)
    {
            SSPIF = 0;
            sspstat = SSPSTAT & 0b00100101;
            if (sspstat == 0b00000001) //Last Byte was an Address + write
            {
                buff_idx = 0;
                SSPCON1bits.CKP = 1;
            }
            else if ( sspstat == 0b00100001 ) // Last Byte was Data
            {
                i2c_rx_buff[buff_idx] = SSPBUF;
                SSPCON1bits.CKP = 1;
                if (++buff_idx == sizeof(i2c_rx_buff))
                {
                    buff_idx--;
                }
            }
            else if ( sspstat == 0b00000101 ) //  Last Byte was an Address + read
            {
                //SSPBUF;
                KEYSTATPIN = 1 ;
                SSPBUF = i2c_tx_buff[0];
                SSPCON1bits.CKP=1;
                buff_idx = 1;
            }
            else if (( sspstat == 0b00100100 )&&(!SSPCON2bits.ACKSTAT)) // Last Byte was Data
            {
                SSPBUF = i2c_tx_buff[buff_idx];
                SSPCON1bits.CKP = 1;
                if (++buff_idx == sizeof(i2c_tx_buff))
                {
                    buff_idx = 0;
                    //KEYSTATPIN = 1 ;
                }
            }
            else
            {
                SSPCON1bits.CKP=1;
            }
   }
}
