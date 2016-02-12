// MCP9800.c

// Correction history
// Author      Date       Rev   Comments
// --------------------------------------
// j.julicher  unknown    0.0   original
// w.brown     2011.3.10  0.1   Corrected write_one_byte()
// w.brown     2011.3.11  0.2   Changed general operation to shutdown with one-shot reads

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
#define FOSC    16000000UL
#include "i2c.h"
#include "mcp9800.h"
#include <htc.h>

extern bit mc9800_ok;

// Register Addresses
#define TEMPERATURE     0x00
#define CONFIGURATION   0x01
#define THYST           0x02
#define TSET            0x03

#define SF_12_BIT       16
#define SF_11_BIT       8
#define SF_10_BIT       4
#define SF_9_BIT        2

#define MODE_12_BIT     3
#define MODE_11_BIT     2
#define MODE_10_BIT     1
#define MODE_9_BIT      0

#define SHIFT_12_BIT    4
#define SHIFT_11_BIT    5
#define SHIFT_10_BIT    6
#define SHIFT_9_BIT     7

static UINT8    read_one_byte(UINT8 reg);
static void     write_one_byte(UINT8 reg, UINT8 data);
static UINT16   read_two_bytes(UINT8 reg);

typedef union
{
    struct
    {
        unsigned    shutdown : 1;
        unsigned    comp_int : 1;   // comparator/interrupt mode
        unsigned    alert_polarity : 1;
        unsigned    fault_queue : 2;
        unsigned    resolution : 2;
        unsigned    one_shot : 1;
    } bits;
    UINT8    value;
} MCP9800_CONFIG;

/****************************************************************************
  Function:
    void mcp9800_init(void)

  Summary:
    Initialize the MCP9800 attached via I2C into 12-bit mode.

  Description:
    This function uses the I2C bus to initialize the MCP9800
 
  Precondition:
    I2C is running

  Parameters:
    None

  Returns:
    None

  Remarks:
***************************************************************************/
void mcp9800_init(void)
{
    MCP9800_CONFIG    c;
    // start out in shutdown mode then initiate first reading
    c.value = 0;
    c.bits.resolution = MODE_12_BIT;
    c.bits.shutdown = 1;
    write_one_byte(CONFIGURATION, c.value);
    c.bits.one_shot = 1;
    write_one_byte(CONFIGURATION, c.value);
    
}

/****************************************************************************
  Function:
    void mcp9800_shutdown(void)

  Summary:
    sets the shutdown bit in the mcp9800 to save power.

  Description:
    Reads and modifies the configuration register in the MCP9800 to set
    the shutdown bit.
 
  Precondition:
    I2C is running

  Parameters:
    None

  Returns:
    None

  Remarks:
  ***************************************************************************/
void mcp9800_shutdown(void)
{
    MCP9800_CONFIG    c;
    c.value = read_one_byte(CONFIGURATION);
    c.bits.shutdown = 1;
    write_one_byte(CONFIGURATION, c.value);
}

/****************************************************************************
  Function:
    void mcp9800_powered(void)

  Summary:
    Clears the shutdown bit in the mcp9800 to turn it on.

  Description:
    Reads and modifies the configuration register in the MCP9800 to clear
    the shutdown bit.
 
  Precondition:
    I2C is running

  Parameters:
    None

  Returns:
    None

  Remarks:
  ***************************************************************************/
void mcp9800_powered(void)
{
    MCP9800_CONFIG    config;
    config.value = read_one_byte(CONFIGURATION);
    config.bits.shutdown = 0;
    write_one_byte(CONFIGURATION, config.value);
}

/****************************************************************************
  Function:
    UINT16 mcp9800_get_temp_raw(void)

  Summary:
    returns the raw data from the MCP9800

  Description:
    Requests the 16-bit temperature register contents and returns it as is.
 
  Precondition:
    I2C is running

  Parameters:
    None

  Returns:
    UINT16 representing the raw value from the TEMPERATURE register

  Remarks:
    The MCP9800 stores the temperature left justified.  This driver does
    not right shift the data to justify the result.  RAW means RAW
    If you want the data scaled into C, use mcp9800_get_temp()
  ***************************************************************************/
UINT16 mcp9800_get_temp_raw(void)
{
    return (read_two_bytes(TEMPERATURE));
}

/****************************************************************************
  Function:
    INT16 mcp9800_get_temp(void)

  Summary:
    returns the temperature data from the MCP9800 in degrees C * 10

  Description:
    Gets the data from the temp sensor and returns a fixed point value
    representing C*10.  23.4C is represented as 234.
 
  Precondition:
    I2C is running

  Parameters:
    None

  Returns:
    INT16 representing the temperature in degrees C * 10

  Remarks:
  ***************************************************************************/
INT16 mcp9800_get_temp(void)
{
    INT16 temp;
    temp = read_two_bytes(TEMPERATURE) >> SHIFT_12_BIT;
    temp *= 10;         // give room for decimal fixed point
    temp /= SF_12_BIT;  // scale factor for 12-bit mode
    // generate one-shot reading for next get-temp
    write_one_byte(CONFIGURATION, 0xE1);
    return (temp);      // temperature in C*10
}

/****************************************************************************
 The functions below this line are supporting functions of the driver and are
 not called outside of the mcp9800 driver
****************************************************************************/

/****************************************************************************
  Function:
    UINT8 read_one_byte(UINT8 reg)

  Summary:
    returns the 8-bit value in the specified register

  Description:
    Performs an I2C read of the requested register
 
  Precondition:
    I2C is running

  Parameters:
    UINT8 reg : register value to read from

  Returns:
    UINT8 representing the value in the requested register

  Remarks:
  ***************************************************************************/
static UINT8 read_one_byte(UINT8 reg)
{
    UINT8    d1, d2;
    d1 = reg;
    d1 &= 0x03; // guard the request register
    i2c_write_block(MCP9800_ADDRESS, 1, &d1, I2C_200K); // queue the write
    i2c_read_block(MCP9800_ADDRESS, 1, &d2, I2C_200K);  // queue the read
    return (d2);
}

/****************************************************************************
  Function:
    void write_one_byte(UINT8 reg, UINT8 data)

  Summary:
    Writes and 8-bit data value in the supplied register

  Description:
    Performs an I2C write of the 8-bit data into the register provided.
 
  Precondition:
    I2C is running

  Parameters:
    UINT8 reg : register value to write into
    UINT8 data : data value to write

  Returns:
    None

  Remarks:
  ***************************************************************************/
static void write_one_byte(UINT8 reg, UINT8 data)
{
    UINT8   d[2];
    UINT8*  p;
    
    p = d;
    *p++ = reg & 0x03; // guard the request register
    *p = data;
    i2c_write_block(MCP9800_ADDRESS, 2, d, I2C_200K); // write the register and corresponding data
}

/****************************************************************************
  Function:
    UINT16 read_two_byte(UINT8 reg)

  Summary:
    Reads a 16-bit value from the requested register

  Description:
    Performs an I2C read of two bytes from the passed register address
 
  Precondition:
    I2C is running

  Parameters:
    UINT8 reg : register value to read from

  Returns:
    UINT16 is returned containing the register contents

  Remarks:
  ***************************************************************************/
static UINT16 read_two_bytes(UINT8 reg)
{
    // result is little endian
    UINT8     d1;
    UINT16    d2;

    d1 = reg;
    d1 &= 0x03; // guard the request register
    i2c_write_block(MCP9800_ADDRESS, 1, &d1, I2C_200K);         // prepare the write
    i2c_read_block(MCP9800_ADDRESS, 2, (char *) &d2, I2C_200K); // prepare the read
    return (d2 << 8 | d2 >> 8);
}
