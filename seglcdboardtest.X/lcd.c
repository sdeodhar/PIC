/*********************************************************************
* Software License Agreement:
*
* The software supplied herewith by Microchip Technology Incorporated
* (the "Company") for its PICmicro® Microcontroller is intended and
* supplied to you, the Company's customer, for use solely and
* exclusively on Microchip PICmicro Microcontroller products. The
* software is owned by the Company and/or its supplier, and is
* protected under applicable copyright laws. All rights are reserved.
* Any use in violation of the foregoing restrictions may subject the
* user to criminal sanctions under applicable laws, as well as to
* civil liability for the breach of the terms and conditions of this
* license.
*
* THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*********************************************************************/
#include <pic.h>
#include "lcd.h"
#include "seven_seg.h"

const SEVEN_SEGMENT_TYPE   segment_data[] = {DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9, DIGIT_A, DIGIT_B, DIGIT_C, DIGIT_D, DIGIT_E, DIGIT_F};

/****************************************************************************
  Function:
    void lcd_init(void)

  Summary:
    Initializes the LCD peripheral for the F1 Evaluation Platform

  Description:
    This function configures the LCD peripheral for:
    Type B Waveform,
    Timer 1 Oscillator, and 
    1:1 prescaler
    All segment data is cleared.  The segments enables are configured for
    the F1 Evaluation Platform hardware.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    Type B waverorm was used to allow the LCDIF flag to be the RTCC source.
    This feature was needed when operating the BLDC motors because Timer 1
    is needed for accurate commutation time and cannot be used for RTCC.
  ***************************************************************************/
void lcd_init(void)
{

    // Configure LCDPS
    // Wave form type A or B
    // Bias Mode
    // Prescaler
    // 1:16 - 1:1
    LCDPS = 0;
    WFT = 1;        // B type
    LCDPSbits.LP = 0;

    T1OSCEN = 1;    // activate the 32khz oscillator for our clock source

    /*******************************************************/

    /* User is responsible to enable the needed segments   */

    /* Replace the code in the next 3 lines for your glass */

    /*******************************************************/
    LCDSE0 = 0x37;
    LCDSE1 = 0x14;
    LCDSE2 = 0x13;

    /*******************************************************/
    // Configure LCDCON
    // LCD enbled
    // LCD enabled in sleep
    // LCD Clock Select - T1OSC
    // LCD Commons - 1/4
    LCDCON = 0;
    SLPEN = 0;
    WERR = 0;
    LCDCONbits.CS = 1;
    LCDCONbits.LMUX = 3;
    // clear ALL SEGMENT DATA
    LCDDATA0 = 0;
    LCDDATA1 = 0;
    LCDDATA2 = 0;
    LCDDATA3 = 0;
    LCDDATA4 = 0;
    LCDDATA5 = 0;
    LCDDATA6 = 0;
    LCDDATA7 = 0;
    LCDDATA8 = 0;
    LCDDATA9 = 0;
    LCDDATA10 = 0;
    LCDDATA11 = 0;

    LCDIF = 0;

    // Reference ladder control
    LCDRL = 0x30;

    // Configure LCDREF
    // Internal reference enabled
    // Internal Reference Source
    // Internal Reference always on
    LCDREF = 0;
    LCDIRE = 1;
    LCDIRS = 0;
    LCDIRI = 0;

    LCDCST = 0;     // maximum contrast
    LCDEN = 1;
}

/****************************************************************************
  Function:
    void lcd_display_off(void)

  Summary:
    Turn off the LCD peripheral.  Leave the display configured.

  Description:
    This function is needed to generate a blinking display without modifying
    all the segment data.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    
  ***************************************************************************/
void lcd_display_off(void)
{
    LCDEN = 0;
}

/****************************************************************************
  Function:
    void lcd_display_on(void)

  Summary:
    Turn on the LCD peripheral.  Leave the display configured.

  Description:
    This function is needed to generate a blinking display without modifying
    all the segment data.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    
***************************************************************************/
void lcd_display_on(void)
{
    LCDEN = 1;
}

/****************************************************************************
  Function:
    BOOL lcd_display_digits(BCD_TYPE b)

  Summary:
    Render the 4 BCD digits 0-9,A-F passed in BCD_TYPE
  Description:
    This function checks the Write Allow bit to determine if it is ok to modify
    the LCD segments registers.  Write Allow could be false in Type B waveforms.
    If the Write Allow flag is true, this function accepts the numbers for
    display on the LCD. It renders the 7 segment data as quickly as possible by 
    a 2 level mapping of digits to segments and then segments to LCD segments.
    This function returns TRUE or FALSE based upon the state of WA.
 
  Precondition:
    None
  Parameters:
    BCD_TYPE b : digits to render

  Returns:
    BOOL TRUE if display was updated.
         FALSE if the display was not updated.
  Remarks:
    It is possible that this function will return false and thereby leave the
  in the old state. 
    If you are modifying other icons on the glass (decimal points, symbols)
    test the result of this function and only modify the segments if this returns true.
    As long as you are quick, the WA bit will still be true and allow writes.
  ***************************************************************************/

BOOL lcd_display_digits(BCD_TYPE b)
{

    // Map Digit 3
    UINT8   val;

// The long list of IF statements was the fastest code as of HITEC 9.65PL1

    if(WA) // make sure it is OK to write the data
    {
        val = segment_data[b.digit0].val;
        if(val & SEG_A)
        {
            A1 = 1;
        }

        if(!(val & SEG_A))
        {
            A1 = 0;
        }

        if(val & SEG_B)
        {
            B1 = 1;
        }

        if(!(val & SEG_B))
        {
            B1 = 0;
        }

        if(val & SEG_C)
        {
            C1 = 1;
        }

        if(!(val & SEG_C))
        {
            C1 = 0;
        }

        if(val & SEG_D)
        {
            D1 = 1;
        }

        if(!(val & SEG_D))
        {
            D1 = 0;
        }

        if(val & SEG_E)
        {
            E1 = 1;
        }

        if(!(val & SEG_E))
        {
            E1 = 0;
        }

        if(val & SEG_F)
        {
            F1 = 1;
        }

        if(!(val & SEG_F))
        {
            F1 = 0;
        }

        if(val & SEG_G)
        {
            G1 = 1;
        }

        if(!(val & SEG_G))
        {
            G1 = 0;
        }

        val = segment_data[b.digit1].val;
        if(val & SEG_A)
        {
            A2 = 1;
        }

        if(!(val & SEG_A))
        {
            A2 = 0;
        }

        if(val & SEG_B)
        {
            B2 = 1;
        }

        if(!(val & SEG_B))
        {
            B2 = 0;
        }

        if(val & SEG_C)
        {
            C2 = 1;
        }

        if(!(val & SEG_C))
        {
            C2 = 0;
        }

        if(val & SEG_D)
        {
            D2 = 1;
        }

        if(!(val & SEG_D))
        {
            D2 = 0;
        }

        if(val & SEG_E)
        {
            E2 = 1;
        }

        if(!(val & SEG_E))
        {
            E2 = 0;
        }

        if(val & SEG_F)
        {
            F2 = 1;
        }

        if(!(val & SEG_F))
        {
            F2 = 0;
        }

        if(val & SEG_G)
        {
            G2 = 1;
        }

        if(!(val & SEG_G))
        {
            G2 = 0;
        }

        val = segment_data[b.digit2].val;
        if(val & SEG_A)
        {
            A3 = 1;
        }

        if(!(val & SEG_A))
        {
            A3 = 0;
        }

        if(val & SEG_B)
        {
            B3 = 1;
        }

        if(!(val & SEG_B))
        {
            B3 = 0;
        }

        if(val & SEG_C)
        {
            C3 = 1;
        }

        if(!(val & SEG_C))
        {
            C3 = 0;
        }

        if(val & SEG_D)
        {
            D3 = 1;
        }

        if(!(val & SEG_D))
        {
            D3 = 0;
        }

        if(val & SEG_E)
        {
            E3 = 1;
        }

        if(!(val & SEG_E))
        {
            E3 = 0;
        }

        if(val & SEG_F)
        {
            F3 = 1;
        }

        if(!(val & SEG_F))
        {
            F3 = 0;
        }

        if(val & SEG_G)
        {
            G3 = 1;
        }

        if(!(val & SEG_G))
        {
            G3 = 0;
        }

        if(b.digit3 > 0)
        {
            BC4 = 1;
        }
        else
        {
            BC4 = 0;
        }

        return (1);
    }

    return (0);
}
