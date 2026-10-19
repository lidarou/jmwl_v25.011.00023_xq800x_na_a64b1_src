/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_io.h
*
* @addtogroup common_io_interfacing_apis Register IO interfacing APIs
*
* The xil_io.h file contains the interface for the general I/O component, which
* encapsulates the Input/Output functions for the processors that do not
* require any special I/O handling.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 5.00 	pkp  	 05/29/14 First release
* 6.00  mus      08/19/16 Remove checking of __LITTLE_ENDIAN__ flag for
*                         ARM processors
* 7.20  har      01/03/20 Added Xil_SecureOut32 for avoiding blindwrite for
*                         CR-1049218
* </pre>
******************************************************************************/

#ifndef XIL_IO_H           /* prevent circular inclusions */
#define XIL_IO_H           /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <core_op.h>
#include "xil_types.h"

/*****************************************************************************/
/**
*
* @brief    Performs an input operation for a memory location by
*           reading from the specified address and returning the 32 bit Value
*           read  from that address.
*
* @param	Addr: contains the address to perform the input operation
*
* @return	The 32 bit Value read from the specified input address.
*
******************************************************************************/
__STATIC_INLINE u32 Xil_In32(UINTPTR Addr)
{
	return *(volatile u32 *) Addr;
}

/*****************************************************************************/
/**
*
* @brief    Performs an output operation for a memory location by writing the
*           32 bit Value to the the specified address.
*
* @param	Addr contains the address to perform the output operation
* @param	Value contains the 32 bit Value to be written at the specified
*           address.
*
* @return	None.
*
******************************************************************************/
__STATIC_INLINE void Xil_Out32(UINTPTR Addr, u32 Value)
{
	volatile u32 *LocalAddr = (volatile u32 *)Addr;
	*LocalAddr = Value;
}

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/**
* @} End of "addtogroup common_io_interfacing_apis".
*/
