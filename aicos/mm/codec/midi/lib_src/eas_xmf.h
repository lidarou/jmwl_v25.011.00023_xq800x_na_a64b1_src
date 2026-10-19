/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_xmf.h
 *
 * Contents and purpose:
 * XMF Type 0 and 1 File Parser
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 82 $
 *   $Date: 2006-07-10 14:45:19 -0400 (Mon, 10 Jul 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_XMF_H
#define _EAS_XMF_H

#ifndef MAX_XMF_STREAMS
#define MAX_XMF_STREAMS 			16
#endif

/* offsets in to the XMF file */
#define XMF_OFS_HEADER_SIZE			4
#define XMF_OFS_FILE_TYPE			8
#define XMF_OFS_NUM_TRACKS			10

/* size of chunk info (chunk ID + chunk size) */
#define XMF_CHUNK_INFO_SIZE			8

/* 'MTrk' track chunk ID */
#define XMF_CHUNK_TYPE_TRACK		0x4d54726bL

/* some useful meta-events */
#define XMF_META_END_OF_TRACK		0x2f
#define XMF_META_TEMPO				0x51

/* default timebase (120BPM) */
#define XMF_DEFAULT_TIMEBASE		500000L

/* value for pXMFStream->ticks to signify end of track */
#define XMF_END_OF_TRACK			0xffffffff

#endif /* end _EAS_XMF_H */


