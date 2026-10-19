/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_smfdata.h
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
 *   $Revision: 686 $
 *   $Date: 2007-05-03 17:10:54 -0400 (Thu, 03 May 2007) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_SMF_DATA_H
#define _EAS_SMF_DATA_H

#ifndef MAX_SMF_STREAMS
#define MAX_SMF_STREAMS 			64
#endif

/* offsets in to the SMF file */
#define SMF_OFS_HEADER_SIZE			4
#define SMF_OFS_FILE_TYPE			8
#define SMF_OFS_NUM_TRACKS			10

/* size of chunk info (chunk ID + chunk size) */
#define SMF_CHUNK_INFO_SIZE			8

/* 'MTrk' track chunk ID */
#define SMF_CHUNK_TYPE_TRACK		0x4d54726bL

/* some useful meta-events */
#define SMF_META_TEXT				0x01
#define SMF_META_COPYRIGHT			0x02
#define SMF_META_SEQTRK_NAME		0x03
#define SMF_META_LYRIC				0x05
#define SMF_META_END_OF_TRACK		0x2f
#define SMF_META_TEMPO				0x51
#define SMF_META_TIME_SIGNATURE		0x58

/* default timebase (120BPM) */
#define SMF_DEFAULT_TIMEBASE		500000L

/* value for pSMFStream->ticks to signify end of track */
#define SMF_END_OF_TRACK			0xffffffff

#endif

