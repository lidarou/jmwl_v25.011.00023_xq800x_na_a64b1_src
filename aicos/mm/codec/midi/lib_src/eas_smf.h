/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_smf.h
 *
 * Contents and purpose:
 * SMF Type 0 and 1 File Parser
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 82 $
 *   $Date: 2006-07-10 14:45:19 -0400 (Mon, 10 Jul 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_SMF_H
#define _EAS_SMF_H

/* prototypes for private interface to SMF parser */
EAS_RESULT SMF_CheckFileType (S_EAS_DATA *pEASData, EAS_FILE_HANDLE fileHandle, EAS_VOID_PTR *ppHandle, EAS_I32 offset);
EAS_RESULT SMF_Prepare (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData);
EAS_RESULT SMF_Time (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData, EAS_U32 *pTime);
EAS_RESULT SMF_Event (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData, EAS_INT parserMode);
EAS_RESULT SMF_State (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData, EAS_STATE *pState);
EAS_RESULT SMF_Close (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData);
EAS_RESULT SMF_Reset (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData);
EAS_RESULT SMF_Pause (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData);
EAS_RESULT SMF_Resume (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData);
EAS_RESULT SMF_SetData (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 value);
EAS_RESULT SMF_GetData (S_EAS_DATA *pEASData, EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 *pValue);
EAS_RESULT SMF_ParseHeader (EAS_HW_DATA_HANDLE hwInstData, S_SMF_DATA *pSMFData);

#endif /* end _EAS_SMF_H */


