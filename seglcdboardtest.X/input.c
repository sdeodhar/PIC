// input.c

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
#include "pic.h"
#include "input.h"

/****************************************************************************
  Function:
    void input_init(void)

  Summary:
    Initializes the inputs on the F1 Evaluation platform

  Description:
    This function configures the button & pot of the F1 Evaluation platform.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    
  ***************************************************************************/
void input_init(void)
{

    // RD2 is the button input
    TRISD2 = 1;
    ANSD2 = 0;

    // RB2 is the pot input
    TRISB2 = 1;
    ANSB2 = 1;
}

/****************************************************************************
  Function:
    INT16 get_adc(UINT8 channel)

  Summary:
    Initializes the inputs on the F1 Evaluation platform

  Description:
    This function performs a single ADC conversion blocking until it is finished.
    The result is returned as a signed 16-bit integer.
    This function blocks until the ADC is finished.
 
  Precondition:
    None

  Parameters:
    UINT8 channel : the channel number to convert

  Returns:
    INT16 of the current ADC value

  Remarks:
    The ADC is turned on and off in this function.
    The channel is left at the last converted channel.
    This function waits 4 instruction cycles (16 Tosc) for acquisition.
  ***************************************************************************/
INT16 get_adc(UINT8 channel)
{
	char x;
    ADON = 1;                       // ADC is ON
    ADNREF = 0;                     // Negative Reference = VSS
    ADPREF0 = 0;                    // Positive Reference = Vdd
    ADPREF1 = 0;
    ADFM = 1;                       // right justify
    ADCON0bits.CHS = channel;       // select ADC channel
    NOP();                          // wait a little for acquisition.
    NOP();
    NOP();
    NOP();
    GO_nDONE = 1;                   // Start the conversion
    // wait for conversion
    for(x=200;x;x--)                // avoid getting stuck
      if(GO_nDONE == 0) break;      // exit as soon as results are ready

    ADON = 0;                       // turn ADC off to save a little power
    return (ADRESH << 8 | ADRESL);  // return the result
}

/****************************************************************************
  Function:
    INT16 input_pot(void)

  Summary:
    Measures the potentiometer and returns a filtered result.

  Description:
    This function uses get_adc(8) to measure channel 8.  Then it performs a
    rolling average filter and returns the result.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    INT16 filtered measurement of the potentiometer.

  Remarks:
    The rolling average filter is adequate for simple DC measurments.
  ***************************************************************************/

INT16 input_pot(void)

// 32 point rolling average filter with simulated infinite sum
{
    static INT16  integrator = 0;
    INT16         average;
    
    integrator += get_adc(8);
    average = integrator / 32;
    integrator -= average;
    return (average+1);
}

/****************************************************************************
  Function:
    BOOL input_button(void)

  Summary:
    Returns the debounced value of the push button on RD2

  Description:
    This function requires 10 consequitive samples with RD2 low (pushed) to
    qualify the button as bounce free.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    Debounced value of RD2.
    FALSE = button not pressed (RD2 is high)
    TRUE = button pressed (RD2 is low for 10 samples)

  Remarks:
    
  ***************************************************************************/

BOOL input_button(void)
{
    static INT8 cntr = 0;

    if(RD2) // Button is on PORTD pin 2
    {
        cntr = 0;
    }
    else
    {
        if(cntr < 10)
        {
            cntr++;
        }
        else
        {
            cntr = 10;
        }
    }
    return (cntr == 10);
}

/****************************************************************************
  Function:
    event_t input_event(void)

  Summary:
    Processes the single button into the states UP, DOWN, PRESSED & RELEASED.

  Description:
    This function helps write user interface state machines by determining when
    the button was pressed, released
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    event_t value of the current button events.
    Valid responses are BUTTON_UP, BUTTON_DOWN, BUTTON_PRESSED, BUTTON_RELEASED

  Remarks:
    
  ***************************************************************************/

event_t input_event(void)
{
    event_t     ret;
    static BOOL btn_pv;
    BOOL        btn;

    btn = input_button();
    if(!btn &!btn_pv)
    {
	    // button is not pressed now nor was it pressed previously
        ret = BUTTON_UP;
    }
    else if(btn &!btn_pv)
    {
	    // button is pressed now but it wasn't previously
        ret = BUTTON_PRESSED;
    }
    else if(btn & btn_pv)
    {
	    // button was pressed previously and is still pressed
        ret = BUTTON_DOWN;
    }
    else    // if(!btn & btn_pv)
    {
	    // button is not pressed now but it was previously
        ret = BUTTON_RELEASED;
    }

    btn_pv = btn;
    return (ret);
}
