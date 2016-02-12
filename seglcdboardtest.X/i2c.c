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
#include <pic.h>
#include "GenericTypeDefs.h"
#include "mchp_support.h"

#define FOSC    0   // this will prevent the warning in the i2c.h header.

// the warning is only needed for the user functions and has no effect in this driver.
#include "i2c.h"

typedef enum
{
    S_IDLE,
    S_START,
    S_RESTART,
    S_SEND_ADDR,
    S_SEND_DATA,
    S_SEND_STOP,
    S_ACK_ADDR,
    S_RCV_DATA,
    S_RCV_STOP,
    S_ACK_RCV_DATA,
    S_NOACK_STOP,
    S_SEND_ADDR_10BIT_LSB,
    S_10BIT_RESTART
} i2c_states_t;

// these variables are configured for each driver control block
UINT8 i2c_errors;
i2c_states_t i2c_state;
UINT16 i2c_address;
UINT8 *i2c_buf_ptr;
UINT8 i2c_bytes_left;
I2C_RESULTS *i2c_complete;
I2C_TRANSACTION_REQUEST_BLOCK *i2c_trb_current;
UINT8 i2c_current_count;
UINT8 i2c_count;
UINT8 i2c_done;

/*************************************************************************/

/* Do not forget appropriate PLIB or other CPU independant stuff here    */

/*************************************************************************/
#define SCL_DATA    RC3
#define SDA_DATA    RC4
#define SCL_LAT     LATC3
#define SDA_LAT     LATC4
#define SCL_TRIS    TRISC3
#define SDA_TRIS    TRISC4

/*************************************************************************/
// Porting issues
//
//
/*************************************************************************/
typedef struct
{
    UINT8                           count;  // a count of trb's in the trb_list
    I2C_TRANSACTION_REQUEST_BLOCK   *trb_list;
    I2C_RESULTS                     *completion;    // set with the error of the last trb sent.
    // if all trb's are sent successfully, then this is I2C_REQUEST_COMPLETE
} i2c_queue_t;

#define I2C_QUEUE_SIZE  8
#define I2C_QUEUE_MASK  0x07

i2c_queue_t i2c_queue[I2C_QUEUE_SIZE];
UINT8       i2c_queue_head;
UINT8       i2c_queue_tail;

void        i2c_stop(I2C_RESULTS completion_code);
void        i2c_function_complete(void);

/****************************************************************************
  Function:
    void i2c_init(void)

  Summary:
    Initializes the i2c library and MSSP hardware.

  Description:
    This function initializes the i2c queue's and global variables.
    Then it intializes the MCU I/O pins associated with I2C.
    The SDA line is checked to make sure it is high.
    If the SDA line is high, the I2C bus must be jammed.  The bus is unjammed by
    toggling SCL until SDA goes high, then toggling SDA to create a STOP condition.
    Finally, the MSSP is initialized in I2C master mode.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    If multiple I2C buses are present, extensive rework is required.
    Please contact Microchip for a assistance.
  ***************************************************************************/
void i2c_init(void)
{
    i2c_errors = 0;
    i2c_state = S_IDLE;
    i2c_queue_head = 0;
    i2c_queue_tail = 0;
    i2c_queue_head = 0;
    i2c_queue_tail = 0;
    i2c_current_count = 0;

    // I/O pins
    SCL_LAT = 0;
    SDA_LAT = 0;
    SCL_TRIS = 1;
    SDA_TRIS = 1;

    // It is possible that during debugging we pressed halt while an I2C read was occuring.
    // This could hang the bus.  To resolve this we will toggle SCL until SDC goes high.
    // Then we will toggle SDA Low and Back high.
    // This sequence will clear out the MCP9800 and recover the bus.
    while(SDA_DATA == 0)
    {
        SCL_TRIS = 0;   // low
        NOP();
        NOP();
        NOP();
        SCL_TRIS = 1;   // high
        NOP();
        NOP();
        NOP();
    }

    SDA_TRIS = 0;
    NOP();
    NOP();
    NOP();
    SDA_TRIS = 1;

    // initialize the hardware
    SSPCON = 0x00;
    SSPSTAT = 0x00;
    SSPCON2 = 0x00;
    SSPCON = 0x38;

    SSPIF = 0;
}

/****************************************************************************
  Function:
    UINT8 i2c_get_errors(void)

  Summary:
    Return the total number of i2c errors that have been counted.

  Description:
    The I2C driver increments a counter when an error condition is detected.
    This function allows a calling function to learn the current error count.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    An 8-bit unsigned integer is returned representing the total i2c_errors accumulated by the driver.

  Remarks:
    The define POLLING_DRIVER is checked to create a version that is interrupt safe.
  ***************************************************************************/
UINT8 i2c_get_errors(void)
{
    UINT8 ret;
    #ifndef POLLING_DRIVER
    UINT8 gie_val;
    gie_val = ATOMICSTATE();
    nATOMIC();
    #endif
    ret = i2c_errors;
    #ifndef POLLING_DRIVER
    SETATOMIC(gie_val);
    #endif
    return ret;
}

/****************************************************************************
  Function:
    void i2c_insert(UINT8 count, I2C_TRANSACTION_REQUEST_BLOCK *trb_list, I2C_RESULTS *flag)

  Summary:
    Safely inserts a list of I2C transaction requests into the I2C transaction queue.

  Description:
    The I2C processes lists of transaction requests.  Each transaction list is handled as a string
    of I2C restarts.  When the list of transactions is complete, an I2C stop is produced, the flag
    is set with the correct condition code and the next list is processed from the queue.  
    This function inserts lists of requests prepared by the user application into the queue along
    with a pointer to the completion flag.
 
  Precondition:
    None

  Parameters:
    UINT8 count : The numer of transaction requests in the trb_list.
    I2C_TRANSACTION_REQUEST_BLOCK *trb_list : A pointer to an array of transaction requests.
    I2C_RESULTS *flag : A pointer to a completion flag.

  Returns:
    None

  Remarks:
    This function blocks until there is room in the I2C transaction queue to hold the new trb list.
    This function is thread/isr safe.  i.e. it disables interrupts around critical sections.
  ***************************************************************************/
void i2c_insert(UINT8 count, I2C_TRANSACTION_REQUEST_BLOCK *trb_list, I2C_RESULTS *flag)
{
    UINT8   idx;
    UINT8   new_head;

    #ifndef POLLING_DRIVER
    char    gie_old;
    gie_old = ATOMICSTATE();
    #endif

    // pre compute the new head pointer
    new_head = i2c_queue_head + 1;
    new_head &= I2C_QUEUE_MASK;

    // wait for room in the queue
    do
    {
        #ifndef POLLING_DRIVER
        nATOMIC();
        #else
        i2c_handler(); // we must repeatedly call the i2c_handler if we are in polling mode.
        #endif
        idx = i2c_queue_tail;
        #ifndef POLLING_DRIVER
        SETATOMIC(gie_old);
        #endif
    } while(new_head == idx);

    // now there is room, write at the head
    #ifndef POLLING_DRIVER
    nATOMIC();
    #endif
    *flag = I2C_REQUEST_PENDING;
    i2c_queue[i2c_queue_head].count = count;
    i2c_queue[i2c_queue_head].trb_list = trb_list;
    i2c_queue[i2c_queue_head].completion = flag;

    // update the head pointer
    i2c_queue_head = new_head;

    #ifdef POLLING_DRIVER
    SSPIF = 1;
    #else
    if(!SSPIE)
    {
        SSPIE = 1;
        SSPIF = 1;  // force an interrupt
    }

    SETATOMIC(gie_old);
    #endif
}

/****************************************************************************
  Function:
    void i2c_write_trb(I2C_TRANSACTION_REQUEST_BLOCK *trb, UINT16 address, UINT8 length, UINT8 *data, UINT8 baud)

  Summary:
    populates a trb supplied by the calling function with the parameters supplied by the calling function.

  Description:
    All I2C requests are in the form of TRB's.  This helper function takes standard parameters and correctly formats the TRB.
    The R/W bit is cleared to ensure that the resulting TRB describes an I2C write operation.
 
  Precondition:
    None

  Parameters:
    I2C_TRANSACTION_REQUEST_BLOCK *trb : A pointer to a caller supllied TRB.
    UINT8 address : The address of the I2C peripheral to be accessed
    UINT8 length : The length of the data block to be sent
    UINT8 *data : A pointer to the block of data to be sent
    UINT8 baud : The SPADD value to use for this message.

  Returns:
    None

  Remarks:
    This is a simple helper function.  It is thread/isr safe.
    It is recomended that the value for the baud parameter come from the macros in the header file.
  ***************************************************************************/
void i2c_build_write_trb(I2C_TRANSACTION_REQUEST_BLOCK *trb, UINT16 address, UINT8 length, UINT8 *data, UINT8 baud)
{
    trb->address = address << 1;
    trb->buffer_length = length;
    trb->baud = baud;
    trb->buffer = data;
}

/****************************************************************************
  Function:
    void i2c_read_trb(I2C_TRANSACTION_REQUEST_BLOCK *trb, UINT16 address, UINT8 length, UINT8 *data, UINT8 baud)

  Summary:
    populates a trb supplied by the calling function with the parameters supplied by the calling function.

  Description:
    All I2C requests are in the form of TRB's.  This helper function takes standard parameters and correctly formats the TRB.
    The R/W bit is set to ensure that the resulting TRB describes an I2C read operation.
 
  Precondition:
    None

  Parameters:
    I2C_TRANSACTION_REQUEST_BLOCK *trb : A pointer to a caller supllied TRB.
    UINT8 address : The address of the I2C peripheral to be accessed
    UINT8 length : The length of the data block to be received
    UINT8 *data : A pointer to the block of data to be received
    UINT8 baud : The SPADD value to use for this message.

  Returns:
    None

  Remarks:
    This is a simple helper function.  It is thread/isr safe.
    It is recomended that the value for the baud parameter come from the macros in the header file.
  ***************************************************************************/
void i2c_build_read_trb(I2C_TRANSACTION_REQUEST_BLOCK *trb, UINT16 address, UINT8 length, UINT8 *data, UINT8 baud)
{
    trb->address = address << 1;
    trb->address |= 0x01;   // make this a read
    trb->buffer_length = length;
    trb->baud = baud;
    trb->buffer = data;
}

/****************************************************************************
  Function:
    I2C_RESULTS i2c_write_block(UINT16 address, UINT8 length, UINT8 *data, UINT8 baud)

  Summary:
    Handles one entire I2C write transaction with the supplied parameters.

  Description:
    This function prepares a TRB, then inserts it on the I2C queue.
    Finally, it waits for the transaction to complete and returns the result.
 
  Precondition:
    None

  Parameters:
    UINT8 address : The address of the I2C peripheral to be accessed
    UINT8 length : The length of the data block to be sent
    UINT8 *data : A pointer to the block of data to be sent
    UINT8 baud : The SPADD value to use for this message.

  Returns:
    I2C_RESULTS

  Remarks:
    This is a simple helper function.  It is thread/isr safe.
    It is recomended that the value for the baud parameter come from the macros in the header file.
    This function blocks until done.
  ***************************************************************************/
I2C_RESULTS i2c_write_block(UINT16 address, UINT8 length, UINT8 *data, UINT8 baud)
{
    I2C_TRANSACTION_REQUEST_BLOCK   trb;
    I2C_RESULTS                     flag;
    trb.address = address << 1;
    trb.buffer_length = length;
    trb.baud = baud;
    trb.buffer = data;
    i2c_insert(1, &trb, &flag);
    while(flag == I2C_REQUEST_PENDING)
    {
        #ifdef POLLING_DRIVER
        CLRWDT();
        i2c_handler();
        #endif
    }   // block until request is complete

    return (flag);
}

/****************************************************************************
  Function:
    I2C_RESULTS i2c_read_block(UINT16 address, UINT8 length, UINT8 *data, UINT8 baud)

  Summary:
    Handles one entire I2C Read transaction with the supplied parameters.

  Description:
    This function prepares a TRB, then inserts it on the I2C queue.
    Finally, it waits for the transaction to complete and returns the result.
 
  Precondition:
    None

  Parameters:
    UINT8 address : The address of the I2C peripheral to be accessed
    UINT8 length : The length of the data block to be received
    UINT8 *data : A pointer to the block of data to be received
    UINT8 baud : The SPADD value to use for this message.

  Returns:
    I2C_RESULTS

  Remarks:
    This is a simple helper function.  It is thread/isr safe.
    It is recomended that the value for the baud parameter come from the macros in the header file.
    This function blocks until done.
  ***************************************************************************/
I2C_RESULTS i2c_read_block(UINT16 address, UINT8 length, UINT8 *data, UINT8 baud)
{
    I2C_TRANSACTION_REQUEST_BLOCK   trb;
    I2C_RESULTS                     flag;
    trb.address = address << 1;
    trb.address |= 0x01;    // make this a read
    trb.buffer_length = length;
    trb.baud = baud;
    trb.buffer = data;
    i2c_insert(1, &trb, &flag);
    while(flag == I2C_REQUEST_PENDING)
    {
        #ifdef POLLING_DRIVER
        CLRWDT();
        i2c_handler();
        #endif
    }                       // block until request is complete

    return (flag);
}

/****************************************************************************
  Function:
    void i2c_handler(void)

  Summary:
    Manages the MSSP hardware to be an I2C master.

  Description:
    This function processes transaction request block lists.  This function can be called
    via an ISR or polled.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    This is the core of the I2C driver.  Call it appropriately to keep the bus running.
  ***************************************************************************/
void i2c_handler(void)
{
    UINT8                           val;

    val = 0;                /* Dummy */

    if(SSPIF)
    {
        SSPIF = 0;          // clear the interrupt

        /* If we have a Write Collision, reset and go to our lost state */
        if(WCOL)
        {
            WCOL = 0;       /* clear the Write colision */
            val = SSPBUF;   /* Read the SSP buf to clear */
            i2c_state = S_IDLE;
            return;
        }

        /* Handle the correct I2C state */
        switch(i2c_state)
        {
            case S_IDLE:    /* In reset state, waiting for data to send */
            case S_START:
                if(i2c_queue_head != i2c_queue_tail)
                {

                    /* unload the i2c request information from the queue */

                    // update the number of messages in this buffer
                    // get a pointer to the next message
                    i2c_trb_current = i2c_queue[i2c_queue_tail].trb_list;
                    i2c_count = i2c_queue[i2c_queue_tail].count;
                    i2c_complete = i2c_queue[i2c_queue_tail].completion;

                    // adjust the baud rate
                    SSPADD = i2c_trb_current->baud;

                    // send the start condition
                    SEN = 1;
                    i2c_state = S_SEND_ADDR;                /* start the i2c request */
                }
                else
                {
                    #ifndef POLLING_DRIVER
                    SSPIE = 0;
                    #endif
                }

                break;

            case S_RESTART:

                /* check for pending I2C Request */

                // adjust the baud rate
                SSPADD = i2c_trb_current->baud;

                RSEN = 1;                                   // ... trigger a REPEATED START
                i2c_state = S_SEND_ADDR;                    /* start the i2c request */
                break;

            case S_SEND_ADDR_10BIT_LSB:
                if(ACKSTAT)
                {
                    i2c_errors++;
                    i2c_stop(I2C_REQUEST_ADDRESS_NO_ACK);
                }
                else
                {
                    SSPBUF = (i2c_address >> 1) & 0x00FF;   // Remove bit 0 as R/W is never sent here

                    // determine the next state
                    if(i2c_address & 0x01)                  // check R/W
                    {

                        // if this is a read we must repeat start the bus to perform a read
                        i2c_state = S_10BIT_RESTART;
                    }
                    else
                    {

                        // this is a write continue writing data
                        i2c_state = S_SEND_DATA;
                    }
                }

                break;

            case S_10BIT_RESTART:
                if(ACKSTAT)
                {
                    i2c_errors++;
                    i2c_stop(I2C_REQUEST_ADDRESS_NO_ACK);
                }
                else                        // ACK Status is good
                {

                    // restart the bus
                    RSEN = 1;

                    // fudge the address so S_SEND_ADDR works correctly
                    i2c_address = i2c_address >> 6 & 0x0006;

                    // set the R/W flag
                    i2c_address |= 0x0001;

                    // Resend the address as a read
                    i2c_state = S_SEND_ADDR;
                }

                break;

            case S_SEND_ADDR:

                /* Start has been sent, send the address byte */

                // extract the critical data for this message
                i2c_address = i2c_trb_current->address;
                i2c_buf_ptr = i2c_trb_current->buffer;
                i2c_bytes_left = i2c_trb_current->buffer_length;

                if(i2c_address > 0x00FF)    // we have a 10 bit address
                {

                    // send bits<9:8>
                    SSPBUF = 0xF0 | ((i2c_address >> 6) & 0x0006);  // mask bit 0 as this is always a write
                    i2c_state = S_SEND_ADDR_10BIT_LSB;
                }
                else
                {
                    SSPBUF = i2c_address;                           /* Transmit the address */
                    if(i2c_address & 0x01)
                    {
                        i2c_state = S_ACK_ADDR;                     /* Next state is to wait for address to be acked */
                    }
                    else
                    {
                        i2c_state = S_SEND_DATA;                    /* Next state is transmit */
                    }
                }

                break;

            case S_SEND_DATA:

                /* Make sure the previous byte was acknowledged */
                if(ACKSTAT)
                {

                    /* Transmission was not acknowledged */
                    i2c_errors++;

                    /* Reset the Ack flag */
                    ACKSTAT = 0;

                    /* Send a stop flag and go back to idle */
                    i2c_stop(I2C_REQUEST_DATA_NO_ACK);
                }
                else
                {

                    /* Grab the next data to transmit */
                    SSPBUF = *i2c_buf_ptr++;
                    if(--i2c_bytes_left == 0U)
                    {                       /* Did we send them all ? */
                        i2c_function_complete();
                    }
                }

                break;

            case S_ACK_ADDR:

                /* Make sure the previous byte was acknowledged */
                if(ACKSTAT)
                {

                    /* Transmission was not acknowledged */
                    i2c_errors++;

                    /* Reset the Ack flag */
                    ACKSTAT = 0;

                    /* Send a stop flag and go back to idle */
                    i2c_stop(I2C_REQUEST_ADDRESS_NO_ACK);
                    break;
                }

                RCEN = 1;
                i2c_state = S_ACK_RCV_DATA;
                break;

            case S_RCV_DATA:

                /* Acknowledge is completed.  Time for more data */

                /* Set up to receive a byte of data */
                RCEN = 1;
                i2c_state = S_ACK_RCV_DATA; /* Next thing is to ack the data */
                break;

            case S_ACK_RCV_DATA:

                /* Grab the byte of data received and acknowledge it */
                val = SSPBUF;
                *i2c_buf_ptr++ = val;       /* Save the data */

                if(--i2c_bytes_left)        /* Did we send them all ? */
                {

                    /* No, there's more to receive */

                    /* No, bit 7 is clear.  Data is ok */
                    ACKDT = 0;              /* Flag that we will acknowledge the data */
                    i2c_state = S_RCV_DATA; /* Wait for the acknowledge to complete, then get more */
                }
                else
                {

                    /* Yes, it's the last byte.  Don't ack it */
                    ACKDT = 1;              /* Flag that we will NAK the data */
                    i2c_function_complete();
                }

                ACKEN = 1;                  /* Initiate the acknowledge */
                break;

            case S_RCV_STOP:                /* Send the stop flag */
            case S_SEND_STOP:
                i2c_stop(I2C_REQUEST_COMPLETE);
                break;

            default:
                while(1);

                // sit here forever
                i2c_errors++;
                i2c_stop(I2C_REQUEST_LOST_STATE);
                break;
        }
    }
}

/****************************************************************************
  Function:
    void i2c_function_complete(void)

  Summary:
    Gets the next trb from the list or the queue.

  Description:
    This function determines if there are additional TRB's to process.
    First it looks in the current TRB List.  If the list has another TRB, it issues a restart.
    If the TRB list is empty, it issues a stop and updates the queue pointer.
    The i2c_handler S_IDLE state will check the queue and start as required.
 
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    
  ***************************************************************************/
void i2c_function_complete(void)
{

    // data pending?
    // update the current count for the next item in the list
    i2c_current_count++;

    // update the trb pointer
    i2c_trb_current++;

    // are we done with this string of requests?
    if(i2c_current_count == i2c_count)
    {

        /* if so, update the tail index so we get the next string */
        i2c_queue_tail++;
        i2c_queue_tail &= I2C_QUEUE_MASK;

        /* update the current count to 0 to start the next string */
        i2c_current_count = 0;
        i2c_done = 1;
        i2c_state = S_SEND_STOP;
    }
    else
    {
        i2c_state = S_RESTART;
    }
}

/****************************************************************************
  Function:
    void i2c_stop(I2C_RESULTS completion_code)

  Summary:
    Sends the stop bit if this is the last message in a TRB string.
    Update the completion flag with the passed code.

  Description:
    This function terminates the current I2C message and resets the state machine to pickup the next message.
    If the current message is the last message in the TRB string, it sends the STOP state and updates the completion flag.
    If the completion flag is NULL, it is not written to.
 
  Precondition:
    None

  Parameters:
    I2C_RESULTS completion_code : this code is placed into the i2c_complete flag if this message is the last message in the string.

  Returns:
    None

  Remarks:
    This function is called from the i2c_handler state machine.  Nothing else should use it.
  ***************************************************************************/
void i2c_stop(I2C_RESULTS completion_code)
{

    // is this stop the last message of a string?
    //if(i2c_done)
    //{            
        PEN = 1;            // then send a stop
        i2c_done = 0;       // clear the i2c_done flag

        if(i2c_complete != 0) // make sure i2c_complete is not NULL
        {                   
            *i2c_complete = completion_code; // and update the flag with the completion code
        }
    
    //}
    // if not, then no stop... there will be a restart shortly.
    i2c_state = S_IDLE; /* Done, back to idle */
}
