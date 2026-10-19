/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_effects.h
 *
 * Contents and purpose:
 * Defines a generic effects interface.
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 82 $
 *   $Date: 2006-07-10 14:45:19 -0400 (Mon, 10 Jul 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_EFFECTS_H
#define _EAS_EFFECTS_H

#include "eas_types.h"

typedef struct
{
	EAS_RESULT 	(*pfInit)(EAS_DATA_HANDLE pEASData, EAS_VOID_PTR *pInstData);
	void 		(*pfProcess)(EAS_VOID_PTR pInstData, EAS_PCM *in, EAS_PCM *out, EAS_I32 numSamples);
	EAS_RESULT 	(*pfShutdown)(EAS_DATA_HANDLE pEASData, EAS_VOID_PTR pInstData);
	EAS_RESULT 	(*pFGetParam)(EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 *pValue);
	EAS_RESULT 	(*pFSetParam)(EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 value);
} S_EFFECTS_INTERFACE;

typedef struct
{
	EAS_RESULT 	(*pfInit)(EAS_DATA_HANDLE pEASData, EAS_VOID_PTR *pInstData);
	void 		(*pfProcess)(EAS_VOID_PTR pInstData, EAS_I32 *in, EAS_I32 *out, EAS_I32 numSamples);
	EAS_RESULT 	(*pfShutdown)(EAS_DATA_HANDLE pEASData, EAS_VOID_PTR pInstData);
	EAS_RESULT 	(*pFGetParam)(EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 *pValue);
	EAS_RESULT 	(*pFSetParam)(EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 value);
} S_EFFECTS32_INTERFACE;

/* mixer instance data */
typedef struct
{
	S_EFFECTS_INTERFACE	*effect;
	EAS_VOID_PTR		effectData;
} S_EFFECTS_MODULE;

#endif /* end _EAS_EFFECTS_H */

