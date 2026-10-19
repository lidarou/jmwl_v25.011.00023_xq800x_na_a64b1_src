/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_synth_protos.h
 *
 * Contents and purpose:
 * Declarations, interfaces, and prototypes for synth.
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 82 $
 *   $Date: 2006-07-10 14:45:19 -0400 (Mon, 10 Jul 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_SYNTH_PROTOS_H
#define _EAS_SYNTH_PROTOS_H

/* includes */
#include "eas_data.h"
#include "eas_sndlib.h"

#ifdef _SPLIT_ARCHITECTURE
typedef struct s_frame_interface_tag
{
	EAS_BOOL (* EAS_CONST pfStartFrame)(EAS_FRAME_BUFFER_HANDLE pFrameBuffer);
	EAS_BOOL (* EAS_CONST pfEndFrame)(EAS_FRAME_BUFFER_HANDLE pFrameBuffer, EAS_I32 *pMixBuffer, EAS_I16 masterGain);
} S_FRAME_INTERFACE;
#endif

/* generic synthesizer interface */
typedef struct
{
	EAS_RESULT (* EAS_CONST pfInitialize)(S_VOICE_MGR *pVoiceMgr);
	EAS_RESULT (* EAS_CONST pfStartVoice)(S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, EAS_I32 voiceNum, EAS_U16 regionIndex);
	EAS_BOOL (* EAS_CONST pfUpdateVoice)(S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, EAS_I32 voiceNum, EAS_I32 *pMixBuffer, EAS_I32 numSamples);
	void (* EAS_CONST pfReleaseVoice)(S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, EAS_I32 voiceNum);
	void (* EAS_CONST pfMuteVoice)(S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, EAS_I32 voiceNum);
	void (* EAS_CONST pfSustainPedal)(S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, S_SYNTH_CHANNEL *pChannel, EAS_I32 voiceNum);
	void (* EAS_CONST pfUpdateChannel)(S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, EAS_U8 channel);
} S_SYNTH_INTERFACE;

#endif



