/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_tcdata.h
 *
 * Contents and purpose:
 * SMF File Parser
 *
 * This file contains data declarations for the ToneControl parser.  
 *			
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 82 $
 *   $Date: 2006-07-10 14:45:19 -0400 (Mon, 10 Jul 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef EAS_TFDATA_H
#define EAS_TCDATA_H

#include "eas_data.h"

/*----------------------------------------------------------------------------
 *
 * S_TC_DATA
 *
 * This structure contains the state data for the ToneControl parser
 *----------------------------------------------------------------------------
*/
typedef struct
{
	EAS_FILE_HANDLE	fileHandle;			/* file handle */
	S_SYNTH 		*pSynth;			/* synthesizer handle */
	EAS_I32			fileOffset;			/* offset to start of data */
	EAS_I32			time;				/* current time in 256ths of a msec */
	EAS_I32			tick;				/* tick based on current tempo and resolution */
	EAS_I32			length;				/* length of current note */
	EAS_I32			restorePos;			/* return to here after block */
	EAS_U8			state;				/* current state EAS_STATE_XXXX */
	EAS_U8			volume;				/* volume */
	EAS_I8			note;				/* current note */
	EAS_I8			repeatCount;		/* note repeat counter */
	EAS_I8			tempo;				/* tempo from file (bpm = tempo * 4) */
	EAS_I8			resolution;			/* resolution from file */
	EAS_I8			dataByte;			/* storage for characters that are "put back" */
	EAS_BOOL8		byteAvail;			/* char in "put back" buffer */
} S_TC_DATA;

#endif


