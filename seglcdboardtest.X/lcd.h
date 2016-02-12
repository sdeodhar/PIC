
#ifndef __LCD_H
#define __LCD_H
#include "seven_seg.h"
#include "GenericTypeDefs.h"

typedef union
{
    UINT16    val;
    struct
    {
        unsigned    digit0 : 4;
        unsigned    digit1 : 4;
        unsigned    digit2 : 4;
        unsigned    digit3 : 4;
    };
} BCD_TYPE;

void    lcd_init(void);
BOOL    lcd_display_digits(BCD_TYPE);
void    lcd_display_on(void);
void    lcd_display_off(void);

// Glass pin 5
    #define RC      SEG0COM0
    #define BATT    SEG0COM1
    #define MINUS   SEG0COM2
    #define AC      SEG0COM3

// Glass pin 6
    #define DH  SEG1COM0
    #define RH  SEG1COM1
    #define BC4 SEG1COM2
    #define DP4 SEG1COM3

// Glass pin 7
    #define A3  SEG2COM0
    #define F3  SEG2COM1
    #define E3  SEG2COM2
    #define D3  SEG2COM3

// Glass pin 8
    #define B3  SEG4COM0
    #define G3  SEG4COM1
    #define C3  SEG4COM2
    #define DP3 SEG4COM3

// Glass pin 9
    #define A2  SEG5COM0
    #define F2  SEG5COM1
    #define E2  SEG5COM2
    #define D2  SEG5COM3

// Glass pin 10
    #define B2  SEG10COM0
    #define G2  SEG10COM1
    #define C2  SEG10COM2
    #define DP2 SEG10COM3

// Glass pin 11
    #define A1  SEG12COM0
    #define F1  SEG12COM1
    #define E1  SEG12COM2
    #define D1  SEG12COM3

// Glass pin 12
    #define B1  SEG16COM0
    #define G1  SEG16COM1
    #define C1  SEG16COM2

//#define    SEG16COM3
// Glass pin 13
    #define S1      SEG17COM0
    #define S2      SEG17COM1
    #define MILLI   SEG17COM2
    #define MEGA    SEG17COM3

// Glass pin 14
    #define AMPS    SEG20COM0
    #define VOLT    SEG20COM1
    #define KILO    SEG20COM2
    #define OHMS    SEG20COM3
#endif // __LCD_H
