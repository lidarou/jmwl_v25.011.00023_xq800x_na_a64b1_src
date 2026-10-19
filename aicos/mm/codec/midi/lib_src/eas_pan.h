/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_pan.h
 *
 * Contents and purpose:
 * Calculates left and right gain multipliers based on a pan value from -63 to +63
 *
 * NOTES:
 * The _CMX_PARSER and _MFI_PARSER preprocessor symbols determine
 * whether the parser works for those particular file formats. 
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 82 $
 *   $Date: 2006-07-10 14:45:19 -0400 (Mon, 10 Jul 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef EAS_PAN_H
#define _EAS_PAN_H

#include "eas_types.h"

/*----------------------------------------------------------------------------
 * EAS_CalcPanControl()
 *----------------------------------------------------------------------------
 * Purpose: 
 * Assign the left and right gain values corresponding to the given pan value.
 *
 * This routine uses sin/cos approximations for an equal power curve:
 * 
 * sin(x) = (2-4*c)*x^2 + c + x
 * cos(x) = (2-4*c)*x^2 + c - x
 *
 * where  c = 1/sqrt(2)
 * using the a0 + x*(a1 + x*a2) approach
 *
 * Inputs: 
 * pan			- pan value (-63 to + 63)
 * 
 * Outputs:
 * pGainLeft	linear gain multiplier for left channel (15-bit fraction)
 * pGainRight	linear gain multiplier for left channel (15-bit fraction)
 *
 * Side Effects:
 *----------------------------------------------------------------------------
*/
void EAS_CalcPanControl (EAS_INT pan, EAS_I16 *pGainLeft, EAS_I16 *pGainRight);

#endif

