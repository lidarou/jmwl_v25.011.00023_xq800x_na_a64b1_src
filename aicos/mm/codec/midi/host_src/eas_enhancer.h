/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_enhancer.h
 *
 * Contents and purpose:
 * Contains parameter enumerations for the enhancer effect
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

#ifndef _EAS_ENHANCHER_H
#define _EAS_ENHANCHER_H

/* enumerated parameter settings for enhancer effect */
typedef enum
{
	EAS_PARAM_ENHANCER_BYPASS,
	EAS_PARAM_ENHANCER_PRESET,
	EAS_PARAM_ENHANCER_FEEDBACK1,
	EAS_PARAM_ENHANCER_FEEDFORWARD1,
	EAS_PARAM_ENHANCER_FEEDBACK2,
	EAS_PARAM_ENHANCER_DRIVE,
	EAS_PARAM_ENHANCER_WET,
	EAS_PARAM_ENHANCER_DRY
} E_ENHANCER_PARAMS;

#endif /* #ifndef _EAS_ENHANCER_H */


