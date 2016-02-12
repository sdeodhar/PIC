/* 
 * File:   newmain.c
 * Author: Admin
 *
 * Created on July 30, 2014, 3:20 PM
 */

#include <stdio.h>
#include <stdlib.h>
//#include <pic.h>
#include "string.h"
#include <pic16f1936.h>
#include <htc.h>
#include "lcd.h"

__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_OFF & MCLRE_OFF & CP_OFF & CPD_OFF & BOREN_ON & CLKOUTEN_OFF & IESO_ON & FCMEN_OFF);
__CONFIG(WRT_OFF & VCAPEN_OFF & PLLEN_ON & STVREN_ON & BORV_LO & LVP_OFF);

//const SEVEN_SEGMENT_TYPE   segment_data[] = {DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9, DIGIT_A, DIGIT_B, DIGIT_C, DIGIT_D, DIGIT_E, DIGIT_F};

//#define device_control_code  0b1010
//#define threshold 19500//17000
UINT32 thold[8];
#define KEYSTATPIN  LATCbits.LATC5
//#define button_pressed_threshold ((reading1*2)/3)+reading1
//#define button_notpressed_threshold ((reading2*1)/3)+reading2

//const UINT8 tab[16] = {0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b000000111,0b01111111,0b01101111,0b01110111,0b01111100,0b00111001,0b01011110,0b01111011,0b01110001};
//UINT32 reading;
UINT16 debounc[8];
//UINT8 dispflag;
//UINT8 count0,count1,count2,count3,count4,count5,count6,count7;
UINT32 curreading0[8],curreading1[8],curreading2[8],curreading3[8],curreading4[8],curreading5[8],curreading6[8],curreading7[8];
UINT32 average[8];
UINT32 raw_reading;//average[8];
UINT8 channelno;//chan[8];
UINT8 count[8];
UINT8 avgflag[8];
UINT8 i2c_rx_buff[8];
UINT8 i2c_tx_buff[8];

void capsensinit(void);
void interrupt isr(void);

/*void Delay(UINT32 ms)
{
    UINT8 i;
    UINT16 j = 0;
    while(ms--)
    {
        i=4;
        while(i--)
        {
            for(j = 0 ; j < 100 ; j++){}
        }
    }
}*/

void scanchannel()
{
    int i = 0;
    if(channelno==0){CPSCON1 = 0b00000000;}
    else if(channelno==1){CPSCON1 = 0b00000001;}
    else if(channelno==2){CPSCON1 = 0b00000010;}
    else if(channelno==3){CPSCON1 = 0b00000011;}
    else if(channelno==4){CPSCON1 = 0b00000100;}
    else if(channelno==5){CPSCON1 = 0b00000101;}
    else if(channelno==6){CPSCON1 = 0b00000110;}
    else if(channelno==7){CPSCON1 = 0b00000111;}
    for(i = 0 ; i < 8 ; i++)
    {
        if(i == channelno)  avgflag[ i ] = 0;
        else  avgflag[ i ] = 1;
    }
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

/*void LCDinit(void)
{
     LCDPS = 0b00110001;
     LCDSE0 = 0x7f;
     //LCDSE1 = 0x7f;
     LCDCON = 0b00001011;//0b10000011;

         LCDDATA0 = 0x00;        //0xF9;  Sets activated waveform between COM0 - SEG0
         /*LCDDATA1 = 0x00;        //0xFF;
         LCDDATA3 = 0x00;        //0xFF;
         LCDDATA4 = 0x00;
         LCDDATA6 = 0x00;
         LCDDATA7 = 0x00;
         LCDDATA9 = 0x00;
         LCDDATA10 = 0x00;*/
   /*      LCDIF = 0;

     LCDREF = 0b10000000;
     LCDCST = 0b00000000;//000;
     LCDRL = 0b11110111;///000;
     LCDEN = 1;
}

 void lcd_display_off(void)
{
    LCDEN = 0;
}

void lcd_display_on(void)
{
    LCDEN = 1;
}

UINT8 getsegval(UINT8 val)
{
 UINT8 comb;
 comb = tab[val];
 return comb;
}*/

/*void dispdata(UINT8 dispval1,UINT8 dispval2,UINT8 dispval3)
{
    LCDDATA0 = dispval1 | (dispval2 & 0x01)<<6;
    LCDDATA1 = (dispval2) << 1;
    LCDDATA2 = dispval3;
}*/

/*void display(UINT32 temp)
{
    UINT8 tempdig[8];
    UINT8 dispval[8];
    //UINT8 i;
    tempdig[0] = temp % 10L ;
    dispval[0] = getsegval( tempdig[ 0 ] ) ;
     /*for ( i=0 ; i<8 ; i++)
     {
         tempdig[i] = temp % 10L ;
         dispval[i] = getsegval( tempdig[ i ] ) ;
         temp /= 10L ;
     }*/
  //for now  LCDDATA0 = dispval[0] ;
    /*LCDDATA1 = dispval[1] ;
    LCDDATA3 = dispval[2] ;
    LCDDATA4 = dispval[3] ;
    LCDDATA6 = dispval[4] ;
    LCDDATA7 = dispval[5] ;
    LCDDATA9 = dispval[6] ;
    LCDDATA10 = dispval[7] ;*/
/*
}
*/
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

/*void I2Cinit(void)
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
}*/

void main(void)
{
    //UINT32 tempval[8];//,disptempval;
    UINT8 i=0,j=0,buttondetect[8],keyno = 0;//,dispbutton,chan,keyno =0;
    OSCCON = 0b01111000;
    OSCTUNE = 0b00000000;
    UINT8 tholdflg = 0,tcount =0;
    ANSELA = 0;
    ANSELB = 0;
    TRISA = 0;
    TRISB = 0;
    TRISC = 0;

    TRISCbits.TRISC5 = 0; // RC6 = TX out
    KEYSTATPIN = 1;
    capsensinit(); //Delay(5);
        //LCDinit();
     usartinit();//Delay(5);
    //Capavginit();
   //I2Cinit();//Delay(5);

        //while(1)
        //{
          //  USARTWriteChar('U');
            /*USARTWriteInt(tempval[0],5);
            USARTWriteChar('\t');
            USARTWriteInt(tempval[1],5);
            USARTWriteChar('\t');
            USARTWriteInt(tempval[2],5);
            USARTWriteChar('\n');
            USARTWriteChar('\r');*/
        //}
        i = 0;
         while(1)
         {
            //tempval= 12345678;
            //display(tempval);
          //if(dispflag)
           //{
                 if(i == 8) i = 0;//if(i==3) i=0;
                 //tempval[i] = reading;
                 channelno  =   i;
                 scanchannel();   
                 //while(tempval[i] >= 100) tempval[i]/=10;
                 //if(i==1)tempval[i]*=100;
                 //if(i==2)tempval[i]*=10000;
                 //disptempval=tempval[0]+tempval[1]+tempval[2];
                 //display(disptempval);
                 //if (tempval[i] > 19) buttondetect[i]= 1;//20)
                 if(avgflag[0]==1)
                 {
                     average[0] = 0 ;
                     for(j=0; j<8 ; j++)
                    {
                         average[0] += curreading0[j];
                    }
                    average[0]/=8; avgflag[0]=0;
                 }
                 if(avgflag[1]==1)
                 {
                     for(j=0; j<8 ; j++)
                    {
                     average[1] += curreading1[j];
                    }
                    average[1]/=8; avgflag[1]=0;
                 }
                 if(avgflag[2]==1)
                 {
                     for(j=0; j<8 ; j++)
                    {
                         average[2] += curreading2[j];
                    }
                    average[2]/=8;avgflag[2]=0;
                 }
                 
                 if(avgflag[3]==1)
                 {
                     for(j=0; j<8 ; j++)
                    {
                         average[3] += curreading3[j];
                    }
                    average[3]/=8;avgflag[3]=0;
                 }
                 
                 if(avgflag[4]==1)
                 {
                     for(j=0; j<8 ; j++)
                    {
                     average[4] += curreading4[j];
                    }
                    average[4]/=8;avgflag[4]=0;
                 }
                 
                 if(avgflag[5]==1)
                 {
                     for(j=0; j<8 ; j++)
                    {
                     average[5] += curreading5[j];
                    }
                    average[5]/=8;avgflag[5]=0;
                 }
                 
                 if(avgflag[6]==1)
                 {
                     for(j=0; j<8 ; j++)
                    {
                     average[6] += curreading6[j];
                    }
                 average[6]/=8;avgflag[6]=0;
                 }
                 
                 if(avgflag[7]==1)
                 {
                     for(j=0; j<8 ; j++)
                    {
                     average[7] += curreading7[j];
                    }
                    average[7]/=8;avgflag[7]=0;
                 }
                    USARTWriteChar('0');
                    USARTWriteChar(':');
                    USARTWriteInt(average[0],5);
                    USARTWriteChar(' ');
                    USARTWriteChar('1');
                    USARTWriteChar(':');
                    USARTWriteInt(average[1],5);
                    USARTWriteChar(' ');
                    USARTWriteChar('2');
                    USARTWriteChar(':');
                    USARTWriteInt(average[2],5);
                    USARTWriteChar(' ');
                    USARTWriteChar('3');
                    USARTWriteChar(':');
                    USARTWriteInt(average[3],5);
                    USARTWriteChar(' ');
                    USARTWriteChar('4');
                    USARTWriteChar(':');
                    USARTWriteInt(average[4],5);
                    USARTWriteChar(' ');
                    USARTWriteChar('5');
                    USARTWriteChar(':');
                    USARTWriteInt(average[5],5);
                    USARTWriteChar(' ');
                    USARTWriteChar('6');
                    USARTWriteChar(':');
                    USARTWriteInt(average[6],5);
                    USARTWriteChar(' ');
                    USARTWriteChar('7');
                    USARTWriteChar(':');
                    USARTWriteInt(average[7],5);
                    USARTWriteChar('\n');
                    USARTWriteChar('\r');

                    i++;//sh

   }
    /*sh      if(tholdflg == 0)
           {
               tcount++;
               thold[i] = average[i] ;//+ (average[i]/2) ;//19900;
               if(tcount > 128) tholdflg = 1 ;
           }
           else
           {     for( keyno = 0 ; keyno < 8 ; keyno ++)
                 {
                     if(average[keyno] > thold[keyno] && avgflag[keyno]==0 && debounc[keyno] == 0)
                     {
                         buttondetect[keyno]= 1;
                         debounc[keyno] = 300;
                     }
                     //else if(average[keyno] < threshold && avgflag[keyno]==0) {buttondetect[keyno]= 0;}
                     else buttondetect[keyno]= 0;

                     if(buttondetect[keyno])
                     {
                          i2c_tx_buff[ 0 ] = keyno;
                          KEYSTATPIN = 0;sh*/
                          /*USARTWriteChar('Y');
                          USARTWriteChar('\t');
                          USARTWriteInt(tempval[keyno],5);
                          USARTWriteChar('\t');
                          USARTWriteInt(keyno,2);
                          USARTWriteChar('\t');
                          USARTWriteChar('\n');
                          USARTWriteChar('\r');*/
                 //sh    }
                     /*else
                     {
                         //KEYSTATPIN = 1;
                         USARTWriteChar('N');
                         USARTWriteChar('\t');
                         USARTWriteInt(tempval[keyno],5);
                         USARTWriteChar('\t');
                         USARTWriteInt(keyno,2);
                         USARTWriteChar('\t');
                         USARTWriteChar('\n');
                         USARTWriteChar('\r');
                     }*/
              //sh   }
           //sh}
               //sh  i++;
                 //dispbutton = buttondetect[0]+ (2*buttondetect[1])+ (4*buttondetect[2]);
                 //display(dispbutton);
                     //uart_xmit(dispbutton);
                     /*USARTWriteInt(tempval[0],5);
                     USARTWriteChar('\t');
                     USARTWriteInt(tempval[1],5);
                     USARTWriteChar('\t');
                     USARTWriteInt(tempval[2],5);
                     USARTWriteChar('\n');
                     USARTWriteChar('\r');*/
                 //}
             //    dispflag=0;
                 
         }
         //sh}
 //sh      return;

//}

void interrupt isr(void)
{
 UINT8 sspstat, buff_idx,no;
 for(no = 0 ; no < 8 ;no++)
 {
     if(debounc[no])
     debounc[no]= debounc[no]-1;
 }
    if (TMR0IE && TMR0IF)				// Overflow interrupt
		TMR0IF = 0;				// Clear T0IF every time

	if (TMR1GIF && TMR1GIE)
        {			// Gate interrupt
		TMR1GIF = 0;
                TMR0 = 0;
                raw_reading = TMR1L + (unsigned int)(TMR1H << 8);

                if(channelno==0)
                {
                curreading0[count[channelno]] =  raw_reading;
                count[channelno]++;
                if(count[channelno]==8)count[channelno]=0;
                }

                if(channelno==1)
                {
                curreading1[count[channelno]] =  raw_reading;
                count[channelno]++;
                if(count[channelno]==8)count[channelno]=0;
                }

                if(channelno==2)
                {
                curreading2[count[channelno]] =  raw_reading;
                count[channelno]++;
                if(count[channelno]==8)count[channelno]=0;
                }

                if(channelno==3)
                {
                curreading3[count[channelno]] =  raw_reading;
                count[channelno]++;
                if(count[channelno]==8)count[channelno]=0;
                }

                if(channelno==4)
                {
                curreading4[count[channelno]] =  raw_reading;
                count[channelno]++;
                if(count[channelno]==8)count[channelno]=0;
                }

                if(channelno==5)
                {
                curreading5[count[channelno]] =  raw_reading;
                count[channelno]++;
                if(count[channelno]==8)count[channelno]=0;
                }

                if(channelno==6)
                {
                curreading6[count[channelno]] =  raw_reading;
                count[channelno]++;
                if(count[channelno]==8)count[channelno]=0;
                }

                if(channelno==7)
                {
                curreading7[count[channelno]] =  raw_reading;
                count[channelno]++;
                if(count[channelno]==8)count[channelno]=0;
                }
                /*
                if(dispflag==0)
                {
                   reading = TMR1L + (unsigned int)(TMR1H << 8);
                   dispflag = 1;
                }*/

                TMR1L   = 0x00;					// Reset Timer1
                TMR1H   = 0x00;					//   "     "
                TMR1ON  = 1;					// Restart Timer1
                TMR0    = 0xFE;
                T1GGO   = 1;
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
                }
            }
            else
            {
                SSPCON1bits.CKP=1;
            }
    }
      
}
/*
 
 */