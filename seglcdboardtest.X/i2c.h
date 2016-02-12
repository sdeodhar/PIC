// i2c driver

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
#ifndef __I2C_H
    #define __I2C_H

// DEFINE POLLING_DRIVER if you are NOT using the I2C interrupt.
//    #define POLLING_DRIVER
    #ifdef POLLING_DRIVER
#warning THE POLLING DRIVER IS ACTIVE.#warning REMOVE LINE 28 IN I2C.H TO DISABLE THE POLLING DRIVER
    #else
#warning THE INTERRUPT DRIVER IS ACTIVE #warning REMEMBER i2c_handler() IN YOUR ISR
    #endif
    #include "GenericTypeDefs.h"

typedef enum
{
    I2C_REQUEST_PENDING,
    I2C_REQUEST_COMPLETE,
    I2C_REQUEST_STUCK_START,
    I2C_REQUEST_ADDRESS_NO_ACK,
    I2C_REQUEST_DATA_NO_ACK,
    I2C_REQUEST_LOST_STATE
} I2C_RESULTS;

// Define FOSC before using these defines!!!
// or you will get the FOSC set for 8MHZ
    #ifndef FOSC
        #define FOSC    8000000UL
#warning FOSC assumed to be 8 MHZ
    #endif
    
    #define I2C_400K    (FOSC / (4 * 400000UL) + 1)
    #define I2C_200K    (FOSC / (4 * 200000UL) + 1)
    #define I2C_100K    (FOSC / (4 * 100000UL) + 1)
    #define I2C_50K     (FOSC / (4 * 50000UL) + 1)
    #define I2C_25K     (FOSC / (4 * 25000UL) + 1)

// i2c transaction request block (TRB)
// these have to be built and sent to the driver to handle
// each i2c request.
typedef struct
{
    UINT16  address;        // Bits <10:1> are the 10 bit address.

    // Bits <8:1> are the 7 bit address
    // Bit 0 is R/W (1 for read)
    UINT8   baud;           // set the baud rate for this message.
    UINT8   buffer_length;  // the # of bytes in the buffer
    UINT8   *buffer;        // a pointer to a buffer of buffer_length bytes
} I2C_TRANSACTION_REQUEST_BLOCK;

// call i2c_init to initialize the MSSP & prepare for traffic
void        i2c_init(void);

// i2c_insert is the generic trb queuing function
// it blocks until the TRB list is added to the I2C queue
// count is the number of trb's in the trb_list
// trb_list is an array of trb's.
void        i2c_insert(UINT8 count, I2C_TRANSACTION_REQUEST_BLOCK *trb_list, I2C_RESULTS *flag);

// These two helper functions assemble the trb's
void        i2c_build_write_trb
            (
                I2C_TRANSACTION_REQUEST_BLOCK   *trb,
                UINT16                          address,
                UINT8                           length,
                UINT8                           *data,
                UINT8                           baud
            );
void        i2c_build_read_trb
            (
                I2C_TRANSACTION_REQUEST_BLOCK   *trb,
                UINT16                          address,
                UINT8                           length,
                UINT8                           *data,
                UINT8                           baud
            );

// simple blocking interface
// these function assemble 1 TRB, queue the request and return when the TRB has been processed
I2C_RESULTS i2c_write_block(UINT16 address, UINT8 length, UINT8 *data, UINT8 baud);
I2C_RESULTS i2c_read_block(UINT16 address, UINT8 length, UINT8 *data, UINT8 baud);

// poll this function OR place it in the ISR
// DO NOT DO BOTH
void        i2c_handler(void);

#endif // __I2C_H
