/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_xmfdata.h
 *
 * Contents and purpose:
 * Contains declarations for the XMF file parser.
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

#ifndef _EAS_XMFDATA_H
#define _EAS_XMFDATA_H

#include "eas_data.h"

/*----------------------------------------------------------------------------
 *
 * S_XMF_DATA
 *
 * This structure contains the instance data required to parse an XMF file.
 *
 *----------------------------------------------------------------------------
*/

typedef struct
{
	EAS_FILE_HANDLE		fileHandle;
	EAS_I32				fileOffset;
	EAS_VOID_PTR		pSMFData;
	EAS_I32				midiOffset;
	EAS_I32				dlsOffset;
	S_DLS				*pDLS;
} S_XMF_DATA;

#endif
