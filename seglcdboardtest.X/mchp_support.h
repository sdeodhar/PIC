// mchp_support.h
// device specific utility functions needed by all programs

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
#ifndef __MCHP_SUPPORT_H
    #define __MCHP_SUPPORT_H

    #ifdef __PICC__
        #define SETATOMIC(n) \
    {                        \
        GIE = n;             \
    }

#define ATOMICSTATE()   GIE

// TURN OFF ALL INTERRUPTS
#define ATOMIC() \
    {                    \
        GIE = 0;         \
    }

// TURN ON ALL INTERRUPTS
        #define nATOMIC() \
    {                     \
        GIE = 1;          \
    }

    #endif
    #ifdef __18CXX
        #define SETATOMIC(n) \
    {                        \
        INTCONbits.GIEH = n; \
    }

        #define ATOMICSTATE()   INTCONbits.GIEH
        #define ATOMIC()     \
    {                        \
        INTCONbits.GIEH = 1; \
    }

        #define nATOMIC()    \
    {                        \
        INTCONbits.GIEH = 0; \
    }

    #endif
#endif // __MCHP_SUPPORT_H
