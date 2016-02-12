
/*
 * File:   Main.c
 * Author: Admin
 *
 * Created on April 1, 2015, 4:07 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <htc.h>
//#include<pic18f4620>
//#include <pic18f4620.h>
#include<string.h>



// PIC18F4620 Configuration Bit Settings

//#include <htc.h>
__CONFIG(1,IESODIS & FCMDIS & RCIO);
__CONFIG(2,BORDIS & PWRTDIS & WDTDIS);
__CONFIG(3,MCLRDIS & LPT1DIS & PBDIGITAL & CCP2RC1);
__CONFIG(4,XINSTDIS & DEBUGDIS & LVPEN & STVRDIS);
__CONFIG(5,UNPROTECT);
__CONFIG(6,WRTEN);
__CONFIG(7,TRPB);


#define INCkey RA6
#define DECkey RA7
#define SHIFTkey RC0
#define ENTERkey RC1
//#define EXITkey PORTCbits.RC2
#define EXITkey RD0

//#define RS LATAbits.LATA5
#define EN LATA5
//#define EN LATEbits.LATE0
#define RS LATD1
#define d0 LATA1
#define d1 LATA2
#define d2 LATA3
#define d3 LATA4

#define HEATER_OUT LATD3

#define s1m0 LATB1
#define s1m1 LATB0
#define s1m2 LATD7

#define s2m0 LATD6
#define s2m1 LATD5
#define s2m2 LATD4

#define CONFIG  6
#define INC     1
#define DEC     2
#define SHIFT   3
#define ENTER   4
#define EXIT    5

#define J   0
#define K   1

#define config  0b00010101
#define inc     0b00000001
#define dec     0b00000010
#define shift   0b00000100
#define enter   0b00001000
#define exit    0b00010000

#define Ts       1000
#define Kc 50
#define Ti 25
#define Td 0
#define maxtime  1000
#define MAXOUT   1000

typedef struct lookuptable
{
	 float tmp;
	 float vtg;
}tcouple;

tcouple k[31]=
{
	{0.0,0.000},
	{20.0,0.798},
	{40.0,1.611},
	{60.0,2.436},
	{80.0,3.628},
	{100.0,4.096},
	{120.0,5.328},
	{140.0,5.735},
	{160.0,6.941},
	{180.0,7.340},
	{200.0,8.539},
	{220.0,8.940},
	{240.0,10.153},
	{260.0,10.561},
	{280.0,11.382},
	{300.0,12.209},
	{320.0,13.040},
	{340.0,13.874},
	{360.0,14.713},
	{380.0,15.554},
	{400.0,16.397},
	{420.0,17.243},
	{440.0,18.091},
	{460.0,18.941},
	{480.0,19.792},
	{500.0,20.644},
	{520.0,21.497},
	{540.0,22.350},
	{560.0,23.203},
	{580.0,24.055}
};

tcouple j[31]={{0,   0.507},
	{20,  1.537},
        {40,  2.585},
	{60,  3.650},
	{80,  4.726},
	{100,  5.814},
	{120, 6.909},
	{140,  8.010},
	{160, 9.115},
	{180, 10.224},
	{200, 11.334},
	{220,  12.445},
	{240, 13.555},
	{260,  14.665},
	{280, 15.773},
	{300,  16.881},
	{320, 17.986},
	{340,  19.090},
	{360, 20.194},
	{380,  21.297},
	{400, 22.400},
	{420,  23.504},
	{440, 24.610},
	{460,  25.720},
	{480, 26.834},
	{500,  27.393},
	{520, 29.080},
	{540,  29.647},
	{560, 31.360},
        {580,31.939  }
};

int getkey(void);
int waitkey(void);
void lcd_init( void );
void lcd_command(unsigned char);
void lcd_data(unsigned char data);
int Navigate(int key,int statevariable);
void DelayMs(int);
int Display(const char *str,char pos);
int dispnum(int number,char pos);
void timer0_init(void);
//void interrupt isr(void);
void adc_init(void);
int read_adc(void);
void interrupt timer0_isr(void);
float actual(float actualvtg,tcouple*t);
float currenttemp(void);
float calc_err(void);
void cal_pid_parameters(void);
void pid(void);
void pid_error(void);
void choose_tcouple(void);
int getnum(int);
char numstr[16];


int /*error1, error_idx=0*/ count=0, flag,ADC_Count;
//error[9]={10,20,30,40,50,60,70,80,90};
float a;


float  e0, e1, e2, u , u1;
float  ki , kd ,k1 ,k2 ,k3;
char pid_cycle_no=0;
int outint, outint_1, sampleflag, cur_inc;
float accum, samplecount,accumcount;
char nodigit;
unsigned char DATA,PIDFlag, position;
unsigned int sw,thermocouple_type;
float cur_temperature,set_temperature;

const char string2[]="t_couple type   ";
const char string1[]="PID CONTROLLER  ";
const char string3[]="     J          ";
const char string4[]="     K          ";
const char string5[]="   SET TEMP     ";
const char string6[]=" PID T-SET T-CUR";


typedef struct
{
	char enter_state;
	char escape_state;
	int statemax_val;
	int statemin_val;
}allstates;

allstates states[5]={{0,0,0,0},{2,1,0,0},{3,1,1,0},{4,2,580,0},{0,0,0,0}};

void main( void )
{

	int ret,i;
	OSCTUNE=0;
	OSCCON|=0b01110011;

	TRISA=0b11000001;
	//TRISAbits.RA6=1;
	//TRISAbits.RA7=1;
	TRISC=0b0111;
	TRISE=0b000000000;
	TRISB7=0;
        TRISD3=0;
        TRISD2=0;
        TRISD1=0;
        TRISD0=1;
        TRISB0=0;
        TRISB1=0;
        TRISD4=0;
        TRISD5=0;
        TRISD6=0;
        TRISD7=0;

        thermocouple_type = J;
        set_temperature = 100;
        PIDFlag = 0;
        sampleflag=0;
        samplecount=0;
        count=0;
        accum=0;
        flag=0;
        outint=0;
        outint_1=0;
        for(sw=0;sw<16; sw++)
        {
           numstr[sw]=' ';
        }
        //cur_inc = 1;
        lcd_init();
        //Display(string1,1);

         //choose_tcouple();
         //HEATER_OUT = 1;
         //while(1);
        adc_init();
        timer0_init();
        cal_pid_parameters();
	sw=1;
        //HEATER_OUT=0;

        /*while(1)
	{
           // lcd_command(0x80);
	    lcd_data(0xFF);


		PORTBbits.RB7=0x01;
	}
       while(1)
        {
            ret=getkey();
            dispnum(ret,4,0);
        }*/
         while(1)
	{
            // HEATER_OUT=1;
                switch(sw)
		{
			case 1:cur_inc = 1;
                                Display(string1,0x80);
				sw=2;
                                DelayMs(50);
				break;

			case 2:
                            for(i=0;i<16; i++)
                            {
                                numstr[i]=' ';
                            }
                            Display(string2,0x80);
                            PIDFlag=0;
                             if(thermocouple_type==J)
			     Display(string3,0xc0);
                            else if(thermocouple_type==K)
                            Display(string4,0xc0);
				   ret = waitkey();
                            thermocouple_type = Navigate(ret,thermocouple_type);
				//LATDbits.LATD2=1;
                                break;

			case 3:
                            choose_tcouple();
                            Display(string5,0x80);
                            nodigit=4;
                         dispnum((int)set_temperature,0);
                         ret=waitkey();
                         set_temperature = Navigate(ret,(int)set_temperature);
	         	   //LATDbits.LATD2=0;
                            break;

			case 4:Display(string6,0x80);
                               ret = getkey();
                               if(sw == 2)
                                   PIDFlag = 0;
                               else PIDFlag = 1;

                               if(flag==1)
                               {
                                  accum = read_adc();

                                  //accum = accum/accumcount;
                                  //accumcount = 0;
                                  //dispnum( accum ,5,5);

                                  accumcount = 0;
                                  nodigit=4;
                                  dispnum((int)set_temperature,5);

                                  cur_temperature = currenttemp();
                                  dispnum((int)cur_temperature,10);
                                  pid();
                                  accum=0;
                                  dispnum((int)outint,0);
                                  flag=0;
                                  //LATDbits.LATD2=0;
                                  //LATDbits.LATD3=0;

                                  /* errorr=calc_err();
                                   dispnum(errorr,4,11);*/
                                   //read_adc(); dispnum(ADC_Count,5);
                                   //actual(realvtg,k);//convert to temp,calculate error,Pass to PID
                                   //dispnum(cur_temperature,5);
                               }
                              /* if(sampleflag==1)
                               {
                                  accum += read_adc();
                                  accumcount++;
                                  sampleflag = 0;
                               }*/

			       break;

		}
	}
}

  int waitkey( void)
{
	int detected=0;
	while(detected==0)
	{
	        if(INCkey==0)
 	        detected|=inc;
       		if(DECkey==0)
       		detected|=dec ;
       		if(SHIFTkey==0)
       		detected|=shift;
       		if(ENTERkey==0)
       		detected|= enter;
                if(EXITkey==0)
       		detected|= exit;

	}
	if(detected==config)
	{
            sw = 2;
            return(0);
	}
	else if(detected==inc)
	{
		return(INC);
	}
	else if(detected==dec)
	{
               return(DEC);
	}
	else if(detected==shift)
	{
		return(SHIFT);
	}
	else if(detected==enter)
	{
            	return(ENTER);
	}
        else if(detected==exit)
        {
        	return(EXIT);
        }

}
int getkey()
{
       int detected=0;
       if(INCkey==0)
       detected|=inc;
       if(DECkey==0)
       detected|=dec ;
       if(SHIFTkey==0)
       detected|=shift;
       if(ENTERkey==0)
       detected|= enter;
        if(EXITkey==0)
       detected|= exit;
       //return(detected);

        if(detected==config)
	{
            sw = 2;
            return(6);
	}
	else if(detected==inc)
	{
		return(INC);
	}
	else if(detected==dec)
	{
               return(DEC);
	}
	else if(detected==shift)
	{
		return(SHIFT);
	}
	else if(detected==enter)
	{
            	return(ENTER);
	}
        else if(detected==exit)
        {
        	return(EXIT);
        }
       return(7);

}

int Navigate(int key,int statevariable)
{
        int i, max,cur_dig;

        for(i=0; i<nodigit-1; i++)
            max*=10;

        if(statevariable > 9)
            cur_dig = (statevariable % (cur_inc * 10))/cur_inc;
        else
            cur_dig = statevariable;
        //if(max > states[sw].statemax_val)

	switch(key)
	{
            case INC:
                if(cur_dig < 9)
                if((statevariable + cur_inc )<=(states[sw].statemax_val))
                    statevariable+=cur_inc;
                //if(statevariable>(states[sw].statemax_val))
                  //  statevariable=statevariable;
                break;

            case DEC:
                if (cur_dig > 0)
                if((statevariable- cur_inc)>=(states[sw].statemin_val))
                    statevariable-=cur_inc;
                //if(statevariable<(states[sw].statemin_val))
                   // statevariable=statevariable;
                break;

            case SHIFT:
                position++;
                //lcd_command(position);
                cur_inc*=10;

                if(cur_inc>max)
                    cur_inc=1;
                    break;

            case ENTER:
                sw=states[sw].enter_state;
                //lcd_command(0Xc0);
                cur_inc=1;
                position=0xc0;
                    //return(statevariable);
                    break;

                case EXIT:
                    sw=states[sw].escape_state;
                    cur_inc=1;
                    position=0xc0;
                    //return(statevariable);
                    break;

                case CONFIG:
                    sw = 2;
                    //return(statevariable);
                    break;

	}
        return(statevariable);
}


void lcd_command(unsigned char cmd)
{
   RS = 0;
   DATA=((cmd>>4) & 0x0f);
   d0 =  DATA & 0x01;
   d1 = (DATA & 0x02) >> 1;
   d2 = (DATA & 0x04) >> 2;
   d3 = (DATA & 0x08) >> 3;
   EN=1;
   DelayMs(10);
   EN=0;
   DATA=(cmd & 0x0f);
   d0 = DATA & 0x01;
   d1 = (DATA & 0x02) >> 1;
   d2 = (DATA & 0x04) >> 2;
   d3 = (DATA & 0x08) >> 3;
   EN=1;
   DelayMs(10);
   EN=0;
   DelayMs(5);
}

void lcd_data(unsigned char data)
{
   RS=1;
   DATA=((data >>4) & 0x0f);
   d0 = DATA & 0x01;
   d1 = (DATA & 0x02) >> 1;
   d2 = (DATA & 0x04) >> 2;
   d3 = (DATA & 0x08) >> 3;
   EN=1;
   DelayMs(10);
   EN=0;
   DATA=(data & 0x0f);
   d0 = DATA & 0x01;
   d1 = (DATA & 0x02) >> 1;
   d2 = (DATA & 0x04) >> 2;
   d3 = (DATA & 0x08) >> 3;
   EN=1;
   DelayMs(10);
   EN=0;
   DelayMs(5);
}

/*LCD*/
int Display(const char *str,char pos)
{
	int i;
        //lcd_command(0x01) ;
       DelayMs(5) ;
       lcd_command(pos) ;
       DelayMs(5) ;
       for(i=0;i!=16;i++)
       {
          lcd_data(str[i]);
          DelayMs(2);
       }
       	return(0);

}

void lcd_init( void )
{
   DelayMs(100);
   lcd_command(0x03);
   DelayMs(250);
   lcd_command(0x03);
   DelayMs(250);
   lcd_command(0x03);
   DelayMs(250);
   lcd_command(0x02);
   DelayMs(250);
   lcd_command(0x28);
   DelayMs(250);
   lcd_command(0x28);
   DelayMs(250);
   lcd_command(0x28);
   DelayMs(250);
   lcd_command(0x0C);
   DelayMs(250);
   lcd_command(0x06);
   DelayMs(250);
   lcd_command(0x01);
   DelayMs(250);
}

void DelayMs(int del)
{
	int i,j;
	for(i=0;i<del;i++)
		for(j=0;j<50;j++);
}

int dispnum(int number,char pos)
{
	int i;
//	/*for(i=0;i<16;i++)
//        {
//            numstr[i]=' ';
//        }*/
       for(i=pos+nodigit;i>pos;i--)
       {
		numstr[i]=((number%10)+0x30);
		number=number/10;
       }
	//numstr[i]='\0';
	Display(numstr,0xc0);
}

void timer0_init(void)
{
	T0CON=0b00001000;
	TMR0H=0XF8;
	TMR0L=0X30;
	INTCONbits.GIE=1;
        INTCONbits.PEIE=1;
	INTCONbits.TMR0IE=1;
        T0CON|=0b10000000;
}

void adc_init(void)
{
	ADCON0=0;
	ADCON1=0b00001110;
	ADCON2=0b10101000;

        ADIF = 0;
        //PIE1bits.ADIE = 1;
        INTCONbits.GIE = 1;

        DelayMs(5);
        ADCON0 |= 0b010;
}

int read_adc(void)
{
    ADIF = 0;
    ADCON0bits.ADON = 1;           //Enable ADC module.

    //asm("NOP");asm("NOP");
    //asm("NOP");asm("NOP");
    DelayMs(10);           //Wait for 11.5 Tad (acquisition time).

    ADCON0 |= 0b010;		//Set GO/DONE bit.
    while(!ADIF){}
    ADIF = 0;
    ADCON0bits.ADON = 0;
    ADC_Count = ADRESH;
    ADC_Count <<= 8 ;
    ADC_Count += ADRESL;// Read result into ADC_Count.
    ADC_Count = ADC_Count & 0x03ff;
    ADIF = 0;
    return ADC_Count;

}


/*void low_isr(void)
{
    if (INTCONbits.TMR0IE && INTCONbits.TMR0IF)
    {
        LATBbits.LATB7 = 1;
        if(PIDFlag)
        {
            count++;
            //PORTDbits.RD2=1;
            if(count==100)
            {
		count=0;
		flag=1;
                //LATBbits.LATB7 = 0;
            }
            //if(count>=error1)
		//PORTDbits.RD2=0;
            TMR0H=0XF0;
            TMR0L=0X60;
            INTCONbits.TMR0IF=0;
        }
    }
}*/
//#pragma code timer0_isr=0X0008
//#pragma interrupt timer0_isr
void interrupt timer0_isr(void)
{
    if(TMR0IE&&TMR0IF)
    {
        TMR0IF=0;
        TMR0H=0XF8;
	TMR0L=0X30;
        count++;
        samplecount++;
        //HEATER_OUT = 0;

        if(count==1000  /*sampleflag == 0*/)
        {
            flag=1; sampleflag =0;
            count=0;samplecount =0;
        }

        /*if(samplecount==200 )
        {
            accum += read_adc();
            accumcount++;
            //sampleflag=1;
            samplecount=0;
        }*/
        if(outint)
        {
            outint--;
            //LATDbits.LATD2=0;//~LATDbits.LATD2;
            HEATER_OUT = 0;//LATDbits.LATD3=1;//~PORTDbits.RD3;
        }
        else
        {
            HEATER_OUT = 1;//LATDbits.LATD3=1;
        }
        //count++;
        //if(count==100)
        //   count=0;
        TMR0IF=0;
    }
}

//#pragma code



float actual(float actualvtg,tcouple*t)
{       float actualtmp;
	float tmp1,tmp2,vtg1,vtg2,slope;
        int i;
	for(i=0 ;i < 31; i++)
	{
		if(t[i].vtg==actualvtg)
		{
			actualtmp=(t[i].tmp);
			return(actualtmp);
		}

		if(t[i].vtg<actualvtg && t[i+1].vtg>actualvtg)
		{
			vtg1 = t[i].vtg;
			vtg2 = t[i+1].vtg;

			tmp1 = t[i].tmp;
			tmp2 = t[i+1].tmp;
                        //dispnum(tmp1,4,1);
                        //dispnum(tmp2,4,6);
			slope=(tmp2-tmp1)/(vtg2-vtg1);
                        //dispnum(slope,4,11);
                        actualtmp=tmp1+((actualvtg-vtg1)*slope);
			return(actualtmp);
		}
	}
}

float currenttemp(void)
{
    float temp;
    //int adcread;
    //adcread = read_adc() ;
    a = accum ;//adcread ;
                /*if(a==1023)
                {
                         strncpy( Lcd_LINE1 , Lcd_L50 , 16 ) ;
                         strncpy( Lcd_LINE2 , Lcd_L27 , 16 ) ;
                         //Display( ) ;
                }*/
    //a=(/*2500.0*a)/1023.0;
   // a=(a/81.0);
    a=(20.640*a)/526.0;

    //dispnum((int)a,5,1);
    //a=(a/81.0);
    if(thermocouple_type==J)
        temp =actual(a,j);
    else if(thermocouple_type==K)
        temp =actual(a,k);

    //dispnum((int)temp,5,1);
    return  temp;

    /*            ad=readADC(3);
                a=ad ;

                a=(2490.0*a)/1024.0;
                a=a/100;
                b= ((a*27.0)/19.0);
      */           //e= e*100;
  }

float calc_err(void)
{
    float error;
    /*if(cur_temperature<set_temperature)
		error=set_temperature-cur_temperature;

	else
		error=cur_temperature-set_temperature;
		return(error);*/

   error=set_temperature-cur_temperature;
   return(error);

}

void cal_pid_parameters(void)
{
	// Td , Ti to be Specified in terms of Ts
        k1 = Kc + Kc/Ti + Kc*Td ;
	k2 = -(Kc +(2.0*Kc*Td )) ;
	k3 = Kc*Td ;
        u1 = 500.0;
}

void pid(void)
{
        //e0 = calc_error();//setpoint-e ;
	pid_error();
	if(pid_cycle_no>1)
	{
	        u=(u1 + (k1*e0) + (k2*e1) + (k3*e2));

               if (u > 1000.0) u = 1000.0;
               if (u < 0.0) u = 0.0;

                u1 = u;

                outint = u * 1 ;
                if ( outint > MAXOUT )
                {
                      outint = MAXOUT ;
                }

                if ( outint < 0  )
                {
                      outint = 0 ;
                }

               // outint_1 = outint/10;
	}
}

void pid_error(void)
{
 	switch(pid_cycle_no)
	{
		case 0: e2=calc_err();
			pid_cycle_no++;
			break;

		case 1: e1=calc_err();
			pid_cycle_no++;
			break;

		case 2: e0=calc_err();
                        pid_cycle_no++;
                        break;

                case 3: e0=calc_err();
                        e2 = e1 ;
                        e1 = e0 ;
                        break;
	}
}

void choose_tcouple(void)
{
	if(thermocouple_type==J)
	{
		s1m0=1;
		s1m1=1;
		s1m2=0;
		s2m0=0;
		s2m1=0;
		s2m2=0;

	}
	if(thermocouple_type==K)
	{
		s1m0=0;
		s1m1=0;
		s1m2=0;

		s2m0=1;
		s2m1=0;
		s2m2=0;

	}
}

/*int getnum(int num)
{
    int r, cur_inc=1, max=1, i;
    r=waitkey();
    switch(r)
    {
        case INC:
            num+=cur_inc;
            break;

        case DEC:
            num-=cur_inc;
            break;

        case SHIFT:
            position++;
            lcd_command(position);
            cur_inc*=10;
            for(i=0; i<nodigit-1; i++)
            {
                max*=10;
            }
            if(cur_inc>max)
                cur_inc=1;
            break;
    }
    return(num);
}*/
