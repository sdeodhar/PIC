/* 
 * File:   main.c
 * Author: Admin
 *
 * Created on September 24, 2014, 10:53 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "pic16f1829.h"
#include <htc.h>

#define int8u      unsigned char
#define int16u     unsigned int
#define int32u	   unsigned long
#define int8s      signed   char
#define int16s	   signed   int
#define int32s     signed   long

#define SLAVESEL    LATBbits.LATB5  //LATBbits.LATB6
#define SPISCK      LATCbits.LATC3
#define SPISDI      PORTCbits.RC7
#define SPISDO      LATCbits.LATC6
#define DATARDY     PORTCbits.RC0
//#define START       LATBbits.LATB7
#define NOOP        0xff

#define ADCSTATPIN  LATBbits.LATB7

void interrupt i2cddata(void);

int8u i2c_rx_buff[8];
int8u i2c_tx_buff[9];

__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_OFF & MCLRE_OFF & CP_OFF & CPD_OFF & BOREN_ON & CLKOUTEN_OFF & IESO_ON & FCMEN_OFF);
__CONFIG(WRT_OFF & PLLEN_OFF & STVREN_ON & BORV_LO & LVP_OFF);

void Delay(int32u ms)
{
    int8u i;
    int32u j = 0;
    while(ms--)
    {
        i=4;
        while(i--)
        {
            for(j = 0 ; j < 100 ; j++){}
        }
    }
}

void USARTinit(void)
{
    APFCON0 |= 0b10000100;
    TRISCbits.TRISC5 = 1;//B5 = 1; // RC5 = RX out
    TRISCbits.TRISC4 = 0;//B7 = 0; // RC4 = TX in

    TXSTAbits.BRGH = 1 ;//0;
    BRG16 = 1 ;
    SYNC = 0 ;
    TXSTAbits.TX9 = 0;        // select 8 data bits
    TXSTAbits.TXEN = 1;     // enable transmit

    RCSTAbits.SPEN=1;       // serial port is enabled
    //RCSTAbits.RX9=0;        // select 8 data bits
    //RCSTAbits.CREN=1;       // receive enabled
    SPBRG = 69 ;
    //SPBRGL= 69;
    SPBRGH= 0;
}

void USARTWriteChar(char ch)
{
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

void USARTWriteInt(int32u val)
{
    char str[10]={'0','0','0','0','0','0','0','0','0','0'};
    int i=9;
    //Handle negative integers
    if(val<0)
    {
        USARTWriteChar('-');   //Write Negative sign
        val=val*-1;     //convert to positive
    }
    else
    {
        USARTWriteChar('+');
    }

    while(val)
    {
        str[i]=(val%10)+'0';
        val=val/10;
        i--;
        if (i==0) break ;
    }

    for(i=0;i<10;i++)
    {
        USARTWriteChar(str[i]);
    }
}

/*void SPIinit( void )
{
    SSP1STATbits.SMP = 1;
    SSP1STATbits.CKE = 0;
    SSP1CON1bits.SSPEN = 1;
    SSP1CON1bits.CKP = 0;
    SSP1CON1bits.SSPM = 0b0000;
    SSP1CON3bits.BOEN = 1;
}*/

/*
void Send_I2C_ACK(void)
{
   SSP1IF=0;
   SSP1CON2bits.ACKDT=0;
   SSP1CON2bits.ACKEN=1;
   while(!SSP1IF);
}
void Send_I2C_NAK(void)
{
    SSP1IF=0;
    SSP1CON2bits.ACKDT=1;
    SSP1CON2bits.ACKEN=1;
    while(!SSP1IF);
}
void Send_I2C(unsigned int databyte)
{
    SSP1IF = 0;          
    SSPBUF = databyte;            
    while( !SSP1IF );    
}

int8u Read_I2C(void)
{
    SSP1IF = 0;
    SSP1CON2bits.RCEN=1;
    while(!SSP1IF);
    return (SSPBUF);
}
*/

void I2Cinit(void)
{
    ANSELB = 0 ;
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB6 = 1;
    SSP1STAT |= 0xc0;
    SSP1CON1 |= 0x36;
    SSP1CON2 |= 0x80;
    SSP1CON3 |= 0x38;
    SSP1MSK = 0xfe;
    //SSP1ADD = 0xf0;
    SSP1ADD = 0x2c;
    
    SSP1CON1bits.SSPEN = 1;
    SSP1IF = 0;
    SSP1IE = 1;
    PEIE = 1;
    GIE = 1;
}

SPIinit()
{
    ANSELC = 0 ;
    TRISCbits.TRISC3 = 0;
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 0;
    TRISBbits.TRISB5 = 0;   //TRISBbits.TRISB6 = 0;
    SLAVESEL = 1 ;
    SPISCK = 0 ;
}

int8u sendSPIdata(int8u spidata_out)
{
    int8u spidata_in = 0 ,i = 0;
    int8u mask = 0x80;
    for( i = 0; i < 8 ; i++ )
    {
        //SPISCK = 1;
        if (spidata_out & mask) SPISDO = 1 ; else SPISDO = 0;
        SPISCK = 1 ;
        spidata_in <<= 1;
        if ( SPISDI ) spidata_in |= 1 ;
        SPISCK = 0;
        mask >>= 1 ;
    }

    return spidata_in;
}

int8u spi( int8u data_out )
{
    //SLAVESEL = 0 ;
    int8u data_in;
    data_in = sendSPIdata(data_out);
    //SSP1BUF = data_out;
    //while( !SSP1STATbits.BF );
    //data_in = SSP1BUF ;
    //SLAVESEL = 1 ;
    return ( data_in );
    
}

void adcinit(void)
{
    //int32u data ;

    TRISCbits.TRISC1 = 1;
    TRISBbits.TRISB7 = 0;
    TRISAbits.TRISA5 = 0;
    LATAbits.LATA5 = 1 ;
    //LATCbits.LATC7 = 1 ;
    //LATCbits.LATC1 = 1 ;
    //while ( 1 )
    //{
    Delay(10);
    LATAbits.LATA5 = 0 ;
    Delay(10);
    LATAbits.LATA5 = 1 ;

    //START = 1;
    SLAVESEL = 0 ;
    spi(0x00);

    spi(0x40);
    spi(0x00);
    spi(0x01);

    spi(0x42);
    spi(0x00);
    spi(0x50);

    spi(0x43);
    spi(0x00);
    spi(0x05);

    //while ( 1 )
    //{
    //        spi( 0x20 ) ;
    //        spi( 0x00 ) ;
    //        data = spi( NOOP ) ;
            //data <<= 8 ;
            //data |= spi( NOOP ) ;
    //        USARTWriteInt( data);
    //        USARTWriteChar('\n');
    //        USARTWriteChar('\r');
    //}
    SLAVESEL = 1 ;
}
//spi(writeData);

//readData = spi(0x00);

int main(void)
{
    int32u data = 0;
    int32u accum = 0;
    int8u  n = 0 ,databyte1,databyte2,databyte3;
    OSCCON = 0b01111000;//16MHz
    //OSCCON = 0b01100010;//2MHz
    OSCTUNE = 0b00000000;

    TRISA = 0;
    TRISB = 0;
    TRISC = 0;
    //ANSELB = 0 ;
    //START = 0 ;

    USARTinit();Delay(10);
    I2Cinit();Delay(10);
    SPIinit();Delay(10);
    adcinit();
    Delay(100);
    
    SLAVESEL = 0 ;
    //spi(0xff);
    spi(0x14);
    SLAVESEL = 1 ;
    //START = 1;
    accum = 0 ;
    n = 0 ;
    while(1)
    {
        if( !DATARDY )
        {
            //spi(0x13);
            SLAVESEL = 0 ;
            data = 0 ;
            data  = spi(NOOP); data = data << 8;
            data |= spi(NOOP); data = data << 8;
            data |= spi(NOOP);
            SLAVESEL = 1 ;
            accum += data >> 8 ;
            n++ ;
            //data = 12345678 ;
            if ( n == 16 )
            {
                accum >>= 3 ;
                USARTWriteInt( accum );
                USARTWriteChar('\n');
                USARTWriteChar('\r');

/*                i2c_tx_buff[ 0 ] = (accum & 0xff);
                i2c_tx_buff[ 1 ] = (accum & 0xff00)>>8;
                i2c_tx_buff[ 2 ] = (accum & 0xff0000)>>16;
*/
                i2c_tx_buff[ 0 ] = (accum & 0xff);
                i2c_tx_buff[ 1 ] = (accum & 0xff00)>>8;
                i2c_tx_buff[ 2 ] = (accum & 0xff0000)>>16;
                i2c_tx_buff[ 3 ] = (accum & 0xff);
                i2c_tx_buff[ 4 ] = (accum & 0xff00)>>8;
                i2c_tx_buff[ 5 ] = (accum & 0xff);
                i2c_tx_buff[ 6 ] = (accum & 0xff00)>>8;
                i2c_tx_buff[ 7 ] = (accum & 0xff);
                i2c_tx_buff[ 8 ] = (accum & 0xff00)>>8;
                //i2c_tx_buff[ 8 ] = 0xff;
                
                ADCSTATPIN = 0 ;
                accum = 0 ;
                n = 0 ;
                /*if( PIR1bits.SSP1IF )
                {
                    Send_I2C( databyte1 );
                    Send_I2C( databyte2 );
                    Send_I2C( databyte3 );
                }*/
            }
        }
    }

        // Make RC5 a digital output
    //TRISCbits.TRISC5 = 0;
    /*while(1)
    {
        LATCbits.LATC5 = 1; 
        Delay(100);
        LATCbits.LATC5 = 0; 
        Delay(100);
    }*/
    /* for testing usart*/
    /*
    while(1)
    {
        //USARTWriteChar('U');
        //USARTWriteInt('U',5);
        USARTWriteString(" hello");
        USARTWriteChar('\n');
        USARTWriteChar('\r');
    }*/
    return 0;
}

/*while (SSPSTATbits.S == 1)
{
donkey = ReadI2C();
PIR1bits.SSPIF = 0;
SSPSTATbits.S =0;
} */
void interrupt i2cddata(void)
{   int8u sspstat;
    int8u buff_idx;
    if (SSP1IF && SSP1IE)
    {
            SSP1IF = 0;
            sspstat = SSPSTAT & 0b00100101;
            if (sspstat == 0b00000001) //Last Byte was an Address + write
            {
                buff_idx = 0;
                SSP1CON1bits.CKP = 1;
            }
            else if ( sspstat == 0b00100001 ) // Last Byte was Data
            {
                i2c_rx_buff[buff_idx] = SSPBUF;
                SSP1CON1bits.CKP = 1;
                if (++buff_idx == sizeof(i2c_rx_buff))
                {
                    buff_idx--;
                }
            }
            else if ( sspstat == 0b00000101 ) //  Last Byte was an Address + read
            {
                //SSPBUF;
                ADCSTATPIN = 1 ;
                SSPBUF = i2c_tx_buff[0];
                SSP1CON1bits.CKP=1;
                buff_idx = 1;
            }
            else if (( sspstat == 0b00100100 )&&(!SSP1CON2bits.ACKSTAT)) // Last Byte was Data
            {
                SSPBUF = i2c_tx_buff[buff_idx];
                SSP1CON1bits.CKP = 1;
                if (++buff_idx == sizeof(i2c_tx_buff))
                {
                    buff_idx = 0;
                }
            }
            else
            {
                SSP1CON1bits.CKP=1;
            }
    }
}
