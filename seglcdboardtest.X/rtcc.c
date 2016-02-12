// rtcc.c

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
#include <time.h>

#ifdef USE_LCDIF
    #warning USING LCD MODULE FOR TIMEBASE
    #define CLOCKS_PER_SEC  128
#else
    #warning USING TIMER1 FOR TIMEBASE
    #define CLOCKS_PER_SEC  1
#endif
#include <pic.h>
#include "mchp_support.h"
#include "GenericTypeDefs.h"

volatile time_t device_time;
volatile UINT16 seconds_counter;

/****************************************************************************
  Function:
    void rtcc_init(void)

  Summary:
    Initialize the clock calendar driver.

  Description:
    This function configured the basics of a software driven RTCC peripheral.
    It relies upon a periodic event to provide time keeping.  Two event choices
    are available:
    1) Timer 1 Oscillator + Timer 1, and
    2) Timer 1 Oscillator + LCD Peripheral
    In either case, the Timer 1 Oscillator provides a steady 32.768kHz time base.
    If Timer 1 is used, then 0x8000 counts will provide a 1HZ tick.
    CLOCKS_PER_SEC is configured for 1 and all is well.
    If LCDIF is used, then CLOCKS_PER_SEC is 128 so device_time updates every 128
    LCDIF events.  The variable seconds_counter provides this countdown.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    The C stdlib functions understand UNIX time but they need a UNIX time counter.
    UNIX time is the number of seconds elapsed since Midnight Jan 1, 1970
  ***************************************************************************/
void rtcc_init(void)
{
    #ifdef USE_LCDIF
    #else
    T1CONbits.TMR1CS = 2;
    T1OSCEN = 1;
    nT1SYNC = 0;
    TMR1ON = 1;
    TMR1IF = 0;
    TMR1IE = 1;
    #endif
    
    device_time = 1293861600; // Jan 1 2011
    seconds_counter = CLOCKS_PER_SEC;
}

#ifdef USE_LCDIF

/****************************************************************************
  Function:
    void rtcc_handler(void) (LCDIF version)

  Summary:
    maintain device_time (seconds) using the LCDIF flag/interrupt.

  Description:
    This function decrements seconds_counter until 0 and then increments device_time.
    seconds_counter reloads with CLOCK_PER_SEC.
    This version of the function uses LCDIF as the time base.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    
  ***************************************************************************/

void rtcc_handler(void)
{

    // check something for the timebase
    if(LCDIF)
    {
        LCDIF = 0;

        // the LCD interrupt (clocked from T1OSC) has tripped
        // 128 LCDIF's per second
        if(!(--seconds_counter))
        {
            device_time++;
            seconds_counter = CLOCKS_PER_SEC;
        }
    }
}

#else

/****************************************************************************
  Function:
    void rtcc_handler(void) (TMR1 version)

  Summary:
    maintain device_time (seconds) using the LCDIF flag/interrupt.

  Description:
    This function decrements seconds_counter until 0 and then increments device_time.
    seconds_counter reloads with CLOCK_PER_SEC.
    This version of the function uses Timer 1 as the time base.
    Timer 1 is reloaded with 0x8000 to cause TMR1IF to overflow ever second.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    TMR1IF is not looked at or modified because there could be other tasks running
    on this interrupt.
  ***************************************************************************/

void rtcc_handler(void)
{
    TMR1ON = 0;
    TMR1H = 0x80;
    TMR1ON = 1;
    seconds_counter--;
    if(seconds_counter == 0)
    {
        device_time++;
        seconds_counter = CLOCKS_PER_SEC;
    }
}

#endif


/****************************************************************************
  Function:
    void rtcc_set(time_t time)

  Summary:
    set the device time to the passed unix time value.

  Description:
    Update the device time with the passed unix time.
    Interrupts are disabled for the copy.
    Interrupt state is restored to the original state on exit.
    
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    
  ***************************************************************************/

void rtcc_set(time_t *t)
{
    BOOL gie_val;
    gie_val = ATOMICSTATE();
    nATOMIC();
    device_time = *t;
    SETATOMIC(gie_val);
}

/****************************************************************************
  Function:
    time_t time(time_t *t)

  Summary:
    return the current device time.

  Description:
    This function retrieves the device time as either a return value or 
    filling in a variable passed by reference.  Interrupts are disabled
    during the copy and restored on exit.
 
  Precondition:
    None

  Parameters:
    time_t *t : A time_t pointer for the current time.

  Returns:
    time_t value of the current time.

  Remarks:
    This function is a prerequisit to supporting the standard C time libraries.
  ***************************************************************************/

/* implement the time(&t) function for the standard libraries */

/* time.h does not implment time as it is application dependent */
time_t time(time_t *t)
{
    BOOL   gie_val;
    time_t  the_time;
    
    gie_val = ATOMICSTATE();
    nATOMIC();
    the_time = device_time;
    SETATOMIC(gie_val);

    if(t)
    {
        *t = the_time;
    }

    return (the_time);
}
