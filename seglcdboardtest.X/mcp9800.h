// MCP9800.h

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

// uncomment one address
//#define MCP9800_ADDRESS 0x40
//#define MCP9800_ADDRESS 0x41
//#define MCP9800_ADDRESS 0x42
//#define MCP9800_ADDRESS 0x43
//#define MCP9800_ADDRESS 0x44
//#define MCP9800_ADDRESS 0x45
//#define MCP9800_ADDRESS 0x46
//#define MCP9800_ADDRESS 0x47
#define MCP9800_ADDRESS 0x48
//#define MCP9800_ADDRESS 0x4D

void      mcp9800_init(void);
INT16     mcp9800_get_temp(void);
UINT16    mcp9800_get_temp_raw(void);
