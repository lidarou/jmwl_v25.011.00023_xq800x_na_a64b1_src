/*----------------------------------------------------------------------------
 *
 * File:
 * eas_dlssynth.h
 *
 * Contents and purpose:
 * Implements the Mobile DLS synthesizer.
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 143 $
 *   $Date: 2006-07-17 17:09:35 -0400 (Mon, 17 Jul 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_DLSSYNTH_H
#define _EAS_DLSSYNTH_H

/* prototypes */
void DLS_MuteVoice (S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, EAS_I32 voiceNum);
void DLS_ReleaseVoice (S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, EAS_I32 voiceNum);
void DLS_SustainPedal (S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, S_SYNTH_CHANNEL *pChannel, EAS_I32 voiceNum);
EAS_RESULT DLS_StartVoice (S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, EAS_I32 voiceNum, EAS_U16 regionIndex);
EAS_BOOL DLS_UpdateVoice (S_VOICE_MGR *pVoiceMgr, S_SYNTH *pSynth, S_SYNTH_VOICE *pVoice, EAS_I32 voiceNum, EAS_I32 *pMixBuffer, EAS_I32  numSamples);

#endif

