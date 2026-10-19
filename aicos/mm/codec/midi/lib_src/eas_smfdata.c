/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_smfdata.c
 *
 * Contents and purpose:
 * SMF File Parser
 *
 * This file contains data definitions for the SMF parser.  
 *			
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 778 $
 *   $Date: 2007-07-23 19:45:17 -0400 (Mon, 23 Jul 2007) $
 *----------------------------------------------------------------------------
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00b788
#define LOG_MODULE    AMOD_AUDIO

#include "eas_miditypes.h"
#include "eas_smfdata.h"

/*----------------------------------------------------------------------------
 *
 * S_SMF_STREAM
 *
 * Static memory allocation for SMF parser
 *----------------------------------------------------------------------------
*/
static S_SMF_STREAM eas_SMFStreams[MAX_SMF_STREAMS];

/*----------------------------------------------------------------------------
 *
 * eas_SMFData
 *
 * Static memory allocation for SMF parser
 *----------------------------------------------------------------------------
*/
S_SMF_DATA eas_SMFData =
{
	eas_SMFStreams,		/* pointer to individual streams in file */
	0,					/* pointer to next stream with event */
	0,					/* pointer to synth */
	0,					/* file handle */
	{ 0, 0, 0, 0},		/* metadata callback */
	0,					/* file offset */
	0,					/* current time in milliseconds/256 */
	0,					/* actual number of streams */
	0,					/* current MIDI tick to msec conversion */
	0,					/* ticks per quarter note */
	0,					/* current state EAS_STATE_XXXX */
	0					/* flags */
};

