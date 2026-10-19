/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_enhdata.h
 *
 * Contents and purpose:
 * Contains the prototypes for the enhancer effect.
 *
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 82 $
 *   $Date: 2006-07-10 14:45:19 -0400 (Mon, 10 Jul 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_ENHDATA_H
#define _EAS_ENHDATA_H

#include "eas_types.h"

/* parameters for each enhancer */
typedef struct
{
	/* user adjustable parameters for the Enhancer */
	EAS_I8		m_nEnhancerFeedForward1;
	EAS_I8		m_nEnhancerFeedback1;
	EAS_I8		m_nDriveCoef;
	EAS_I8		m_nEnhancerFeedback2;
	EAS_I8		m_nWet;
	EAS_I8		m_nDry;

	/* state variables (filter delays) */
	EAS_I16		m_zF0L;		/* filter 1 zero state var, left */
	EAS_I16		m_zF1L;		/* filter 1 pole state var, left */
	EAS_I16		m_zF2L;		/* filter 2 zero state var, left */
	EAS_I16		m_zF0R;		/* filter 1 zero state var, right */
	EAS_I16		m_zF1R;		/* filter 1 pole state var, right */
	EAS_I16		m_zF2R;		/* filter 2 zero state var, right */
	EAS_BOOL	bypass;
} S_ENHANCER_OBJECT;

#endif /* #ifndef _EAS_ENHDATA_H */


