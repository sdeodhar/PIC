// seven_seg.h

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

/*

 AAAAAA
F      B
F      B
F      B
 GGGGGG
E      C
E      C
E      C
 DDDDDD  DP
 
*/
#define SEG_A   0x01
#define SEG_B   0x02
#define SEG_C   0x04
#define SEG_D   0x08
#define SEG_E   0x10
#define SEG_F   0x20
#define SEG_G   0x40
#define SEG_DP  0x80

#define DIGIT_0 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define DIGIT_1 (SEG_B | SEG_C)
#define DIGIT_2 (SEG_A | SEG_B | SEG_G | SEG_E | SEG_D)
#define DIGIT_3 (SEG_A | SEG_B | SEG_C | SEG_G | SEG_D)
#define DIGIT_4 (SEG_F | SEG_G | SEG_C | SEG_B)
#define DIGIT_5 (SEG_A | SEG_F | SEG_G | SEG_C | SEG_D)
#define DIGIT_6 (SEG_A | SEG_F | SEG_G | SEG_C | SEG_D | SEG_E)
#define DIGIT_7 (SEG_A | SEG_B | SEG_C)
#define DIGIT_8 (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define DIGIT_9 (SEG_A | SEG_B | SEG_C | SEG_F | SEG_G)
#define DIGIT_A (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define DIGIT_B (SEG_F | SEG_G | SEG_C | SEG_D | SEG_E)
#define DIGIT_C (SEG_A | SEG_F | SEG_E | SEG_D)
#define DIGIT_D (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)
#define DIGIT_E (SEG_A | SEG_F | SEG_G | SEG_E | SEG_D)
#define DIGIT_F (SEG_A | SEG_F | SEG_G | SEG_E)

typedef union
{
    unsigned char   val;
    struct
    {
        unsigned    seg_dp : 1;
        unsigned    seg_g : 1;
        unsigned    seg_f : 1;
        unsigned    seg_e : 1;
        unsigned    seg_d : 1;
        unsigned    seg_c : 1;
        unsigned    seg_b : 1;
        unsigned    seg_a : 1;
    };
} SEVEN_SEGMENT_TYPE;

